# aspl-spring-work

ASPL（音情報処理研究室）の春期休暇課題用リポジトリ。音声測定・解析に使う信号を生成する C プログラムです。

## 概要

外部ライブラリに依存せず、WAV ファイル形式でテスト信号を出力します。

## ツール一覧

| プログラム | 説明 |
|------------|------|
| **tsp_gen** | TSP（Time Stretched Pulse）信号の生成。周波数領域で位相を設計し IFFT で時間領域に変換。インパルス応答測定に使用。 |
| **white_noise** | ホワイトノイズの生成。48 kHz・指定秒数の WAV を出力。 |
| **tsp_to_ir** | TSP信号とその応答からインパルス応答を算出。周波数領域で逆フィルタ（down-TSP）を適用。 |
| **adaptive_filter** | 白色信号とその応答から NLMS 適応フィルタでインパルス応答を算出。 |
| **ir_analyze** | インパルス応答から残響時間（T10, T20, RT60）を算出。Schroeder積分で残響曲線を計算し、線形回帰で減衰時間を求める。 |
| **ir_to_inverse** | インパルス応答から逆フィルタ（逆特性）を算出。周波数領域で正則化付き逆フィルタを計算し、時間領域で出力。（発展課題） |

## ビルド方法

```bash
make
```

個別にビルドする場合：

```bash
gcc -o tsp_gen tsp_gen.c -lm
gcc -o white_noise white_noise.c
gcc -o tsp_to_ir tsp_to_ir.c -lm
gcc -o adaptive_filter adaptive_filter.c -lm
gcc -o ir_analyze ir_analyze.c -lm
gcc -o ir_to_inverse ir_to_inverse.c -lm
```

## 使用方法

### TSP信号の生成

```bash
./tsp_gen
```

- 出力: `tsp_signal.wav`（約 5.46 秒、262144 サンプル）

### TSPからインパルス応答を算出

```bash
./tsp_to_ir [TSP信号] [TSP応答] [出力ファイル]
```

例（1個）：
```bash
./tsp_to_ir recordings/tsp_signal.wav impulse_response_tsp.wav recordings/tsp_1.wav
```

例（複数→時間領域で平均してノイズ低減）：
```bash
./tsp_to_ir recordings/tsp_signal.wav impulse_response_tsp.wav \
  recordings/tsp_1.wav recordings/tsp_2.wav ... recordings/tsp_10.wav
```

tsp_1～10 を平均してインパルス応答を算出：
```bash
make tsp_to_ir_all
```
→ `impulse_response_tsp.wav` を生成（10回平均）

### 適応フィルタでインパルス応答を算出

```bash
./adaptive_filter white_noise.wav response.wav output.wav [filter_len]
```

例（180秒の白色信号、フィルタ長1秒=48000サンプル）：
```bash
./adaptive_filter white_noise_180s.wav recordings/white_noise_recorded.wav impulse_response_white.wav 48000
```
※ 180秒分の処理のため、完了まで数分かかります。

### 残響時間を解析

```bash
./ir_analyze [インパルス応答.wav] [残響曲線出力.txt]
```

例：

```bash
./ir_analyze impulse_response_tsp.wav
./ir_analyze impulse_response_white.wav decay_curve.txt
```

- デフォルト入力: `impulse_response.wav`
- 第2引数指定時: 残響曲線（時間 vs dB）をテキストファイルに出力

### 逆フィルタを算出（発展課題）

```bash
./ir_to_inverse [インパルス応答.wav] [逆フィルタ出力.wav]
```

例：

```bash
./ir_to_inverse impulse_response_tsp.wav inverse_filter_tsp.wav
./ir_to_inverse impulse_response_white.wav inverse_filter_white.wav
```

- デフォルト: 入力 `impulse_response.wav`、出力 `inverse_filter.wav`
- 正則化付き逆フィルタ H_inv = conj(H) / (|H|² + ε) を周波数領域で計算

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

## 録音ファイル

収録実験で取得したファイルは `recordings/` に格納しています。  
**バックアップを推奨します。** 詳細は [recordings/README.md](recordings/README.md) を参照。

## 課題内容

詳細は [docs/春期休暇課題.md](docs/春期休暇課題.md) を参照してください。
