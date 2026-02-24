# aspl-spring-work

ASPL（音情報処理研究室）の春期休暇課題用リポジトリ。音声測定・解析に使う信号を生成する C プログラムです。

## 概要

外部ライブラリに依存せず、WAV ファイル形式でテスト信号を出力します。

## ツール一覧

| プログラム | 説明 |
|------------|------|
| **tsp_gen** | TSP（Time Stretched Pulse）信号の生成。周波数領域で位相を設計し IFFT で時間領域に変換。インパルス応答測定に使用。 |
| **white_noise** | ホワイトノイズの生成。48 kHz・指定秒数の WAV を出力。 |

## ビルド方法

```bash
make
```

個別にビルドする場合：

```bash
gcc -o tsp_gen tsp_gen.c -lm
gcc -o white_noise white_noise.c
```

## 使用方法

### TSP信号の生成

```bash
./tsp_gen
```

- 出力: `tsp_signal.wav`（約 5.46 秒、262144 サンプル）

### ホワイトノイズの生成

```bash
./white_noise [秒数] [出力ファイル名]
```

例：

```bash
./white_noise                    # デフォルト: 180秒 → white_noise_180s.wav
./white_noise 60 my_noise.wav    # 60秒 → my_noise.wav
```

## 出力仕様

| 項目 | 仕様 |
|------|------|
| サンプリング周波数 | 48 kHz |
| チャンネル | モノラル |
| ビット深度 | 16 bit |
| 形式 | WAV（PCM） |

## 課題内容

詳細は [docs/春期休暇課題.md](docs/春期休暇課題.md) を参照してください。
