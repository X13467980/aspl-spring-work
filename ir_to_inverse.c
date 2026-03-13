/*
 * ir_to_inverse - インパルス応答から逆フィルタ（逆特性）を算出
 *
 * 発展課題: 空間伝達特性に対する逆特性を計算
 * 周波数領域で H_inv(k) = conj(H(k)) / (|H(k)|^2 + eps) を計算し、
 * IFFT で時間領域の逆フィルタを出力
 *
 * 使い方:
 *   ./ir_to_inverse [インパルス応答.wav] [逆フィルタ出力.wav]
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

void simple_fft(double complex *x, int n) {
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            double complex t = x[i];
            x[i] = x[j];
            x[j] = t;
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * M_PI / len;
        double complex wlen = cos(ang) + I * sin(ang);
        for (int i = 0; i < n; i += len) {
            double complex w = 1.0;
            for (int j = 0; j < len / 2; j++) {
                double complex u = x[i + j];
                double complex v = x[i + j + len / 2] * w;
                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

void simple_ifft(double complex *x, int n) {
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            double complex t = x[i];
            x[i] = x[j];
            x[j] = t;
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / len;
        double complex wlen = cos(ang) + I * sin(ang);
        for (int i = 0; i < n; i += len) {
            double complex w = 1.0;
            for (int j = 0; j < len / 2; j++) {
                double complex u = x[i + j];
                double complex v = x[i + j + len / 2] * w;
                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    for (int i = 0; i < n; i++) x[i] /= n;
}

int write_wav(const char *filename, int16_t *samples, int num_samples, int fs) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "エラー: %s を開けません\n", filename);
        return -1;
    }

    WavHeader head = {
        .riff = {'R', 'I', 'F', 'F'},
        .fileSize = (uint32_t)(36 + num_samples * 2),
        .wave = {'W', 'A', 'V', 'E'},
        .fmt = {'f', 'm', 't', ' '},
        .fmtSize = 16,
        .audioFormat = 1,
        .numChannels = 1,
        .sampleRate = (uint32_t)fs,
        .byteRate = (uint32_t)(fs * 2),
        .blockAlign = 2,
        .bitsPerSample = 16,
        .data = {'d', 'a', 't', 'a'},
        .dataSize = (uint32_t)(num_samples * 2)
    };

    fwrite(&head, sizeof(WavHeader), 1, fp);
    if (fwrite(samples, sizeof(int16_t), (size_t)num_samples, fp) != (size_t)num_samples) {
        fprintf(stderr, "エラー: 書き込みに失敗\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    const char *ir_file = (argc > 1) ? argv[1] : "impulse_response.wav";
    const char *output_file = (argc > 2) ? argv[2] : "inverse_filter.wav";

    printf("インパルス応答から逆フィルタを算出中...\n");
    printf("入力: %s\n", ir_file);
    printf("出力: %s\n", output_file);

    int16_t *ir_samples = NULL;
    int fs;
    int ir_len = read_wav(ir_file, &ir_samples, &fs);
    if (ir_len < 0) {
        return 1;
    }

    printf("IR: %d サンプル, fs = %d Hz (%.3f 秒)\n", ir_len, fs, (double)ir_len / fs);

    int N = 1;
    while (N < ir_len) N <<= 1;
    printf("FFT長: %d\n", N);

    double complex *H = (double complex *)calloc((size_t)N, sizeof(double complex));
    if (!H) {
        fprintf(stderr, "エラー: メモリ確保に失敗\n");
        free(ir_samples);
        return 1;
    }

    for (int i = 0; i < ir_len; i++) {
        H[i] = (double)ir_samples[i] / 32768.0;
    }
    free(ir_samples);

    simple_fft(H, N);

    /* 正則化付き逆フィルタ: H_inv = conj(H) / (|H|^2 + eps) */
    double eps = 1e-6;
    double complex *H_inv = (double complex *)malloc((size_t)N * sizeof(double complex));
    if (!H_inv) {
        fprintf(stderr, "エラー: メモリ確保に失敗\n");
        free(H);
        return 1;
    }

    for (int k = 0; k < N; k++) {
        double mag2 = cabs(H[k]) * cabs(H[k]) + eps;
        H_inv[k] = conj(H[k]) / mag2;
    }
    free(H);

    simple_ifft(H_inv, N);

    /* 時間領域の逆フィルタを正規化して WAV 出力 */
    double max_val = 0;
    for (int i = 0; i < N; i++) {
        double v = fabs(creal(H_inv[i]));
        if (v > max_val) max_val = v;
    }
    if (max_val < 1e-10) max_val = 1e-10;

    int16_t *inv_samples = (int16_t *)malloc((size_t)N * sizeof(int16_t));
    if (!inv_samples) {
        fprintf(stderr, "エラー: メモリ確保に失敗\n");
        free(H_inv);
        return 1;
    }

    double scale = 32767.0 * 0.9 / max_val;
    for (int i = 0; i < N; i++) {
        double v = creal(H_inv[i]) * scale;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        inv_samples[i] = (int16_t)v;
    }
    free(H_inv);

    if (write_wav(output_file, inv_samples, N, fs) < 0) {
        fprintf(stderr, "エラー: WAVファイルの書き込みに失敗\n");
        free(inv_samples);
        return 1;
    }

    printf("完了: %s を保存しました。\n", output_file);
    printf("逆フィルタ長: %d サンプル (%.3f 秒)\n", N, (double)N / fs);

    free(inv_samples);
    return 0;
}
