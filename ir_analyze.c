/*
 * ir_analyze - インパルス応答から残響時間（T10, T20, RT60）を算出
 *
 * 手順:
 *   1. インパルス応答の2乗積分より残響曲線を算出（Schroeder積分）
 *   2. 残響曲線の -5 dB から -15 dB で T10 を算出
 *   3. 同様に、-5 dB から -25 dB で T20 を算出
 *   4. T10 と T20 それぞれから、60 dB減衰の時間を近似して算出
 *
 * 使い方:
 *   ./ir_analyze [インパルス応答.wav] [残響曲線出力.txt]
 *   第2引数省略時は残響曲線をファイル出力しない
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    char     riff[4];
    uint32_t fileSize;
    char     wave[4];
    char     fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char     data[4];
    uint32_t dataSize;
} WavHeader;
#pragma pack(pop)

int read_wav(const char *filename, int16_t **samples, int *fs) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "エラー: %s を開けません\n", filename);
        return -1;
    }

    WavHeader header;
    if (fread(&header, sizeof(WavHeader), 1, fp) != 1) {
        fprintf(stderr, "エラー: WAVヘッダの読み込みに失敗\n");
        fclose(fp);
        return -1;
    }

    if (memcmp(header.riff, "RIFF", 4) != 0 ||
        memcmp(header.wave, "WAVE", 4) != 0 ||
        memcmp(header.fmt, "fmt ", 4) != 0 ||
        memcmp(header.data, "data", 4) != 0) {
        fprintf(stderr, "エラー: 無効なWAVファイル\n");
        fclose(fp);
        return -1;
    }

    *fs = (int)header.sampleRate;
    int num_samples = (int)(header.dataSize / 2);

    *samples = (int16_t *)malloc((size_t)num_samples * sizeof(int16_t));
    if (!*samples) {
        fprintf(stderr, "エラー: メモリ確保に失敗\n");
        fclose(fp);
        return -1;
    }

    if (fread(*samples, sizeof(int16_t), (size_t)num_samples, fp) != (size_t)num_samples) {
        fprintf(stderr, "エラー: データの読み込みに失敗\n");
        free(*samples);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return num_samples;
}

/*
 * ノイズ対策: 有効なインパルス応答長を算出
 * ピークから指定dB以下に減衰した点で打ち切り、ノイズテールを除外する
 * T10算出を確実にするための工夫（ノイズフロアが残響曲線を平坦化するのを防ぐ）
 */
int get_effective_ir_length(int16_t *ir, int len, double cutoff_db) {
    if (len <= 0) return len;
    int16_t peak = 0;
    for (int i = 0; i < len; i++) {
        int16_t a = (ir[i] >= 0) ? ir[i] : -ir[i];
        if (a > peak) peak = a;
    }
    if (peak == 0) return len;
    double threshold = peak * pow(10.0, cutoff_db / 20.0);
    for (int i = len - 1; i >= 0; i--) {
        int16_t s = ir[i];
        double mag = (s == -32768) ? 32768.0 : (s >= 0 ? (double)s : -(double)s);
        if (mag > threshold) {
            return i + 1;
        }
    }
    return len;
}

/*
 * Schroeder積分で残響曲線を計算
 * E(t) = ∫[t to ∞] h^2(τ) dτ
 * インパルス応答の2乗の後方累積積分
 */
void schroeder_integral(int16_t *ir, int len, double *decay_curve) {
    double sum = 0.0;
    for (int i = len - 1; i >= 0; i--) {
        double sample = (double)ir[i] / 32768.0;
        sum += sample * sample;
        decay_curve[i] = sum;
    }

    double max_energy = decay_curve[0];
    if (max_energy > 0) {
        for (int i = 0; i < len; i++) {
            if (decay_curve[i] > 0) {
                decay_curve[i] = 10.0 * log10(decay_curve[i] / max_energy);
            } else {
                decay_curve[i] = -100.0;
            }
        }
    }
}

/*
 * 線形回帰で減衰時間を計算
 * start_db から end_db の区間で傾きを求め、T10/T20 を算出
 * 戻り値: 減衰時間（秒）、エラー時は -1
 */
double calculate_decay_time(double *decay_curve, int len, int fs,
                            double start_db, double end_db, int *start_idx, int *end_idx) {
    *start_idx = -1;
    *end_idx = -1;

    for (int i = 0; i < len; i++) {
        if (*start_idx < 0 && decay_curve[i] <= start_db && decay_curve[i] >= end_db) {
            *start_idx = i;
        }
        if (decay_curve[i] <= end_db) {
            *end_idx = i;
            break;
        }
    }

    if (*start_idx < 0 || *end_idx < 0 || *end_idx <= *start_idx) {
        return -1.0;
    }

    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    int n = *end_idx - *start_idx + 1;

    for (int i = *start_idx; i <= *end_idx; i++) {
        double x = (double)i / fs;
        double y = decay_curve[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    double denominator = n * sum_x2 - sum_x * sum_x;
    if (fabs(denominator) < 1e-10) {
        return -1.0;
    }

    double slope = (n * sum_xy - sum_x * sum_y) / denominator;

    if (slope >= 0) {
        return -1.0;
    }

    double db_range = fabs(start_db - end_db);
    return db_range / (-slope);
}

int main(int argc, char *argv[]) {
    const char *ir_file = (argc > 1) ? argv[1] : "impulse_response.wav";
    const char *curve_file = (argc > 2) ? argv[2] : NULL;

    printf("インパルス応答から残響時間を算出中...\n");
    printf("入力ファイル: %s\n", ir_file);

    int16_t *ir_samples = NULL;
    int fs;
    int num_samples = read_wav(ir_file, &ir_samples, &fs);
    if (num_samples < 0) {
        return 1;
    }

    printf("読み込み完了: %d サンプル, fs = %d Hz (%.3f 秒)\n",
           num_samples, fs, (double)num_samples / fs);

    int eff_len = get_effective_ir_length(ir_samples, num_samples, -40.0);
    if (eff_len < num_samples) {
        printf("ノイズ対策: IRを %.3f 秒まで打ち切り（全長 %.3f 秒）\n",
               (double)eff_len / fs, (double)num_samples / fs);
    }

    double *decay_curve = (double *)malloc((size_t)eff_len * sizeof(double));
    if (!decay_curve) {
        fprintf(stderr, "エラー: メモリ確保に失敗\n");
        free(ir_samples);
        return 1;
    }
    schroeder_integral(ir_samples, eff_len, decay_curve);

    int t10_start, t10_end;
    double t10 = calculate_decay_time(decay_curve, eff_len, fs, -5.0, -15.0, &t10_start, &t10_end);
    double rt60_t10 = (t10 > 0) ? t10 * 6.0 : -1.0;

    int t20_start, t20_end;
    double t20 = calculate_decay_time(decay_curve, eff_len, fs, -5.0, -25.0, &t20_start, &t20_end);
    double rt60_t20 = (t20 > 0) ? t20 * 3.0 : -1.0;

    printf("\n=== 残響時間解析結果 ===\n");

    if (t10 > 0) {
        printf("T10: %.3f 秒 (区間: %.3f - %.3f 秒)\n",
               t10, (double)t10_start / fs, (double)t10_end / fs);
        printf("RT60 (T10から): %.3f 秒\n", rt60_t10);
    } else {
        printf("T10: 計算できませんでした（ノイズレベルが高い可能性）\n");
    }

    if (t20 > 0) {
        printf("T20: %.3f 秒 (区間: %.3f - %.3f 秒)\n",
               t20, (double)t20_start / fs, (double)t20_end / fs);
        printf("RT60 (T20から): %.3f 秒\n", rt60_t20);
    } else {
        printf("T20: 計算できませんでした（ノイズレベルが高い可能性）\n");
    }

    if (curve_file) {
        FILE *fp = fopen(curve_file, "w");
        if (fp) {
            fprintf(fp, "# 時間(秒)\tエネルギー(dB)\n");
            for (int i = 0; i < eff_len; i++) {
                fprintf(fp, "%.6f\t%.2f\n", (double)i / fs, decay_curve[i]);
            }
            fclose(fp);
            printf("\n残響曲線を %s に保存しました\n", curve_file);
        }
    }

    free(ir_samples);
    free(decay_curve);

    return 0;
}
