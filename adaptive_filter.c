/*
 * adaptive_filter - 白色信号とその応答から NLMS 適応フィルタでインパルス応答を算出
 *
 * 使い方:
 *   ./adaptive_filter white_noise.wav response.wav output.wav [filter_len]
 *   filter_len: フィルタ長（サンプル数）。デフォルト 48000（1秒分）
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
        fclose(fp);
        return -1;
    }

    if (memcmp(header.riff, "RIFF", 4) != 0 ||
        memcmp(header.wave, "WAVE", 4) != 0 ||
        memcmp(header.fmt, "fmt ", 4) != 0 ||
        memcmp(header.data, "data", 4) != 0) {
        fclose(fp);
        return -1;
    }

    *fs = (int)header.sampleRate;
    int num_samples = (int)(header.dataSize / 2);

    *samples = (int16_t *)malloc((size_t)num_samples * sizeof(int16_t));
    if (!*samples) {
        fclose(fp);
        return -1;
    }

    if (fread(*samples, sizeof(int16_t), (size_t)num_samples, fp) != (size_t)num_samples) {
        free(*samples);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return num_samples;
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
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    const char *white_file = (argc > 1) ? argv[1] : "white_noise_180s.wav";
    const char *response_file = (argc > 2) ? argv[2] : "white_noise_recorded.wav";
    const char *output_file = (argc > 3) ? argv[3] : "impulse_response_white.wav";
    int filter_len = (argc > 4) ? atoi(argv[4]) : 48000;

    printf("適応フィルタでインパルス応答を算出中...\n");
    printf("白色信号: %s\n", white_file);
    printf("応答: %s\n", response_file);
    printf("出力: %s\n", output_file);
    printf("フィルタ長: %d サンプル (%.2f 秒)\n", filter_len, (double)filter_len / 48000.0);

    int16_t *white_samples = NULL;
    int fs_white;
    int white_len = read_wav(white_file, &white_samples, &fs_white);
    if (white_len < 0) return 1;

    int16_t *response_samples = NULL;
    int fs_response;
    int response_len = read_wav(response_file, &response_samples, &fs_response);
    if (response_len < 0) {
        free(white_samples);
        return 1;
    }

    if (fs_white != fs_response) {
        fprintf(stderr, "エラー: サンプリング周波数が一致しません\n");
        free(white_samples);
        free(response_samples);
        return 1;
    }

    int len = (white_len < response_len) ? white_len : response_len;
    if (filter_len > len / 2) filter_len = len / 2;

    double *w = (double *)calloc((size_t)filter_len, sizeof(double));
    double *x = (double *)malloc((size_t)len * sizeof(double));
    if (!w || !x) {
        fprintf(stderr, "エラー: メモリ確保に失敗\n");
        free(white_samples);
        free(response_samples);
        if (w) free(w);
        if (x) free(x);
        return 1;
    }

    for (int i = 0; i < len; i++) {
        x[i] = (double)white_samples[i] / 32768.0;
    }

    /* NLMS 適応フィルタ */
    double mu = 0.1;
    double eps = 1e-6;

    for (int n = filter_len; n < len; n++) {
        double x_power = 0;
        for (int i = 0; i < filter_len; i++) {
            x_power += x[n - i] * x[n - i];
        }
        double step = mu / (x_power + eps);

        double y_hat = 0;
        for (int i = 0; i < filter_len; i++) {
            y_hat += w[i] * x[n - i];
        }

        double d = (double)response_samples[n] / 32768.0;
        double error = d - y_hat;

        for (int i = 0; i < filter_len; i++) {
            w[i] += step * error * x[n - i];
        }
    }

    /* フィルタ係数を WAV として出力 */
    double max_val = 0;
    for (int i = 0; i < filter_len; i++) {
        double v = fabs(w[i]);
        if (v > max_val) max_val = v;
    }
    if (max_val < 1e-10) max_val = 1e-10;

    int16_t *ir_samples = (int16_t *)malloc((size_t)filter_len * sizeof(int16_t));
    if (!ir_samples) {
        free(w);
        free(x);
        free(white_samples);
        free(response_samples);
        return 1;
    }

    double scale = 32767.0 * 0.9 / max_val;
    for (int i = 0; i < filter_len; i++) {
        double v = w[i] * scale;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        ir_samples[i] = (int16_t)v;
    }

    if (write_wav(output_file, ir_samples, filter_len, fs_white) < 0) {
        fprintf(stderr, "エラー: WAVファイルの書き込みに失敗\n");
    } else {
        printf("完了: %s を保存しました。\n", output_file);
    }

    free(white_samples);
    free(response_samples);
    free(w);
    free(x);
    free(ir_samples);

    return 0;
}
