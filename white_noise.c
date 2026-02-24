#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

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
    /* パラメータ（コマンドラインで上書き可能） */
    const uint32_t sampleRate = 48000;
    uint32_t duration = 180;
    const char *filename = "white_noise_180s.wav";

    if (argc > 1) duration = (uint32_t)atoi(argv[1]);
    if (argc > 2) filename = argv[2];

    const uint64_t numSamples = (uint64_t)sampleRate * duration;

    int16_t *buffer = (int16_t *)malloc(numSamples * sizeof(int16_t));
    if (buffer == NULL) {
        fprintf(stderr, "エラー: メモリ確保に失敗しました。\n");
        return 1;
    }

    /* 1. 白色信号の生成 */
    srand((unsigned int)time(NULL));
    for (uint64_t i = 0; i < numSamples; i++) {
        double randValue = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        buffer[i] = (int16_t)(randValue * 0.5 * 32767);
    }

    /* 2. WAVヘッダの設定 */
    WavHeader header = {
        .riff = {'R', 'I', 'F', 'F'},
        .fileSize = (uint32_t)(36 + (numSamples * 2)),
        .wave = {'W', 'A', 'V', 'E'},
        .fmt = {'f', 'm', 't', ' '},
        .fmtSize = 16,
        .audioFormat = 1,
        .numChannels = 1,
        .sampleRate = sampleRate,
        .byteRate = sampleRate * 2,
        .blockAlign = 2,
        .bitsPerSample = 16,
        .data = {'d', 'a', 't', 'a'},
        .dataSize = (uint32_t)(numSamples * 2)
    };

    /* 3. 書き出し */
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "エラー: ファイルを開けませんでした。\n");
        free(buffer);
        return 1;
    }

    fwrite(&header, sizeof(WavHeader), 1, fp);
    fwrite(buffer, sizeof(int16_t), numSamples, fp);

    fclose(fp);
    free(buffer);

    printf("生成完了: %s (fs:%u Hz, %u 秒)\n", filename, sampleRate, duration);
    return 0;
}
