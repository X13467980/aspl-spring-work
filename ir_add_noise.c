/*
 * ir_add_noise - インパルス応答に白色ノイズを加え、指定SNR(dB)のデータを生成
 * SNR = 20*log10(signal_rms / noise_rms)
 *
 * 使い方: ./ir_add_noise input.wav output.wav SNR_dB
 * 例: ./ir_add_noise impulse_response_white.wav ir_white_sn20.wav 20
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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

static double rand_gaussian(void) {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    if (u1 < 1e-10) u1 = 1e-10;
    return sqrt(-2.0 * log(u1)) * cos(2.0 * 3.141592653589793 * u2);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "使い方: %s input.wav output.wav SNR_dB\n", argv[0]);
        return 1;
    }

    double snr_db = atof(argv[3]);
    if (snr_db <= 0) {
        fprintf(stderr, "エラー: SNR_dB は正の値にしてください\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, "エラー: %s を開けません\n", argv[1]);
        return 1;
    }

    WavHeader header;
    if (fread(&header, sizeof(WavHeader), 1, fp) != 1) {
        fclose(fp);
        return 1;
    }

    if (memcmp(header.riff, "RIFF", 4) != 0 ||
        memcmp(header.wave, "WAVE", 4) != 0 ||
        memcmp(header.fmt, "fmt ", 4) != 0 ||
        memcmp(header.data, "data", 4) != 0) {
        fclose(fp);
        return 1;
    }

    int num_samples = (int)(header.dataSize / 2);
    int16_t *samples = (int16_t *)malloc((size_t)num_samples * sizeof(int16_t));
    if (!samples) {
        fclose(fp);
        return 1;
    }

    if (fread(samples, sizeof(int16_t), (size_t)num_samples, fp) != (size_t)num_samples) {
        free(samples);
        fclose(fp);
        return 1;
    }
    fclose(fp);

    double sum_sq = 0;
    for (int i = 0; i < num_samples; i++) {
        double x = (double)samples[i] / 32768.0;
        sum_sq += x * x;
    }
    double signal_rms = sqrt(sum_sq / num_samples);
    if (signal_rms < 1e-10) {
        fprintf(stderr, "エラー: 信号がほぼゼロです\n");
        free(samples);
        return 1;
    }

    double noise_rms = signal_rms / pow(10.0, snr_db / 20.0);

    srand(42);
    for (int i = 0; i < num_samples; i++) {
        double noise = rand_gaussian() * noise_rms * 32768.0;
        double val = (double)samples[i] + noise;
        if (val > 32767) val = 32767;
        if (val < -32768) val = -32768;
        samples[i] = (int16_t)(val + (val >= 0 ? 0.5 : -0.5));
    }

    FILE *out = fopen(argv[2], "wb");
    if (!out) {
        fprintf(stderr, "エラー: %s を書けません\n", argv[2]);
        free(samples);
        return 1;
    }

    fwrite(&header, sizeof(WavHeader), 1, out);
    fwrite(samples, sizeof(int16_t), (size_t)num_samples, out);
    fclose(out);
    free(samples);

    printf("出力: %s (SNR=%.1f dB)\n", argv[2], snr_db);
    return 0;
}
