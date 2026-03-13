# 録音ファイル

収録実験で取得した音声ファイルを格納しています。

## ファイル一覧

| ファイル | 説明 |
|----------|------|
| tsp_1.wav ～ tsp_10.wav | TSP信号の応答（録音） |
| tsp_signal.wav | TSP信号（生成元） |
| white_noise_recorded.wav | ホワイトノイズの応答（録音） |
| white_noise_recorded_sn20.wav | ホワイトノイズの応答（録音・別条件） |
| audio-interface.wav | オーディオインターフェース関連 |

## バックアップについて

**重要:** これらのファイルは Git で管理されていません（容量のため）。

紛失・上書きを防ぐため、以下のバックアップを推奨します：

1. **外付けHDD / USB** にコピー
2. **クラウドストレージ**（Google Drive, iCloud 等）にアップロード
3. **別のフォルダ** にコピーを保存

```bash
# 例: デスクトップにバックアップ
cp -r recordings ~/Desktop/aspl_recordings_backup_$(date +%Y%m%d)
```
