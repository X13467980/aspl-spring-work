/*
 * ir_to_txt - インパルス応答WAVをテキストに変換（gnuplot用）
 * 出力: 時間(秒) 振幅（正規化 -1～1）
 *
 * 使い方: ./ir_to_txt impulse_response.wav output.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "使い方: %s impulse_response.wav output.txt\n", argv[0]);
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

    int fs = (int)header.sampleRate;
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

    /* 最大値で正規化 */
    int16_t max_val = 0;
    for (int i = 0; i < num_samples; i++) {
        int16_t a = (samples[i] >= 0) ? samples[i] : -samples[i];
        if (a > max_val) max_val = a;
    }
    double scale = (max_val > 0) ? 1.0 / max_val : 1.0;

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        fprintf(stderr, "エラー: %s を書けません\n", argv[2]);
        free(samples);
        return 1;
    }

    fprintf(out, "# 時間(秒)\t振幅\n");
    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / fs;
        double amp = (double)samples[i] * scale;
        fprintf(out, "%.6f\t%.6f\n", t, amp);
    }

    fclose(out);
    free(samples);
    printf("出力: %s (%d サンプル)\n", argv[2], num_samples);
    return 0;
}
