/*
 * tsp_to_ir - TSP信号とその応答からインパルス応答を算出
 * 周波数領域で逆フィルタ（down-TSP）を適用
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
    const char *tsp_file = (argc > 1) ? argv[1] : "tsp_signal.wav";
    const char *response_file = (argc > 2) ? argv[2] : "tsp_response.wav";
    const char *output_file = (argc > 3) ? argv[3] : "impulse_response.wav";

    printf("TSP信号からインパルス応答を算出中...\n");
    printf("TSP信号: %s\n", tsp_file);
    printf("TSP応答: %s\n", response_file);
    printf("出力: %s\n", output_file);

    int16_t *tsp_samples = NULL;
    int fs_tsp;
    int tsp_len = read_wav(tsp_file, &tsp_samples, &fs_tsp);
    if (tsp_len < 0) return 1;

    int16_t *response_samples = NULL;
    int fs_response;
    int response_len = read_wav(response_file, &response_samples, &fs_response);
    if (response_len < 0) {
        free(tsp_samples);
        return 1;
    }

    if (fs_tsp != fs_response) {
        fprintf(stderr, "エラー: サンプリング周波数が一致しません\n");
        free(tsp_samples);
        free(response_samples);
        return 1;
    }

    int N = 1;
    int max_len = (tsp_len > response_len) ? tsp_len : response_len;
    while (N < max_len) N <<= 1;
    printf("FFT長: %d\n", N);

    double complex *TSP = (double complex *)calloc((size_t)N, sizeof(double complex));
    for (int i = 0; i < tsp_len; i++) {
        TSP[i] = (double)tsp_samples[i] / 32768.0;
    }
    simple_fft(TSP, N);

    int start_idx = (response_len >= tsp_len * 2) ? tsp_len : 0;
    int copy_len = (response_len - start_idx < N) ? response_len - start_idx : N;

    double complex *RESPONSE = (double complex *)calloc((size_t)N, sizeof(double complex));
    for (int i = 0; i < copy_len; i++) {
        RESPONSE[i] = (double)response_samples[start_idx + i] / 32768.0;
    }
    simple_fft(RESPONSE, N);

    int J = tsp_len / 2;
    double complex *INV_FILTER = (double complex *)malloc((size_t)N * sizeof(double complex));

    for (int k = 0; k <= N / 2; k++) {
        double theta = 2.0 * M_PI * J * pow((double)k / N, 2);
        INV_FILTER[k] = cos(theta) + I * sin(theta);
        if (k > 0 && k < N / 2) {
            INV_FILTER[N - k] = conj(INV_FILTER[k]);
        }
    }
    INV_FILTER[N / 2] = creal(INV_FILTER[N / 2]) + 0 * I;

    double complex *IR_FREQ = (double complex *)malloc((size_t)N * sizeof(double complex));
    double eps = 1e-10;
    for (int k = 0; k < N; k++) {
        double tsp_mag = cabs(TSP[k]);
        if (tsp_mag > eps) {
            IR_FREQ[k] = RESPONSE[k] * INV_FILTER[k];
        } else {
            IR_FREQ[k] = 0.0;
        }
    }

    simple_ifft(IR_FREQ, N);

    double max_amp = 0;
    for (int i = 0; i < N; i++) {
        double amp = fabs(creal(IR_FREQ[i]));
        if (amp > max_amp) max_amp = amp;
    }
    if (max_amp < 1e-10) max_amp = 1e-10;

    int16_t *ir_samples = (int16_t *)malloc((size_t)N * sizeof(int16_t));
    for (int i = 0; i < N; i++) {
        double sample = creal(IR_FREQ[i]) / max_amp * 0.9;
        if (sample > 1.0) sample = 1.0;
        if (sample < -1.0) sample = -1.0;
        ir_samples[i] = (int16_t)(sample * 32767.0);
    }

    if (write_wav(output_file, ir_samples, N, fs_tsp) < 0) {
        fprintf(stderr, "エラー: WAVファイルの書き込みに失敗\n");
        free(tsp_samples);
        free(response_samples);
        free(TSP);
        free(RESPONSE);
        free(INV_FILTER);
        free(IR_FREQ);
        free(ir_samples);
        return 1;
    }

    printf("完了: %s を保存しました。\n", output_file);
    printf("インパルス応答長: %d サンプル (%.3f 秒)\n", N, (double)N / fs_tsp);

    free(tsp_samples);
    free(response_samples);
    free(TSP);
    free(RESPONSE);
    free(INV_FILTER);
    free(IR_FREQ);
    free(ir_samples);

    return 0;
}
