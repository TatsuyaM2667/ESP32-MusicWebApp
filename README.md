#  ESP32-MusicWebApp

ESP32を使った、Webブラウザから音楽を制御できるシンプルな音楽再生アプリです。Wi-Fi機能付きのESP32を活用し、ローカルネットワーク経由で曲の再生、停止、音量調節、イコライザなどを操作可能です。

##  特徴
- ESP32とDFPlayer miniで音楽の再生
- Webブラウザを使った直感的な操作UI
- 音量調節を視覚的に調節
- イコライザ(ノーマル・ポップ・ジャズ・クラッシク）を選択可能


## 必要なもの
- ESP32開発ボード
- Arduino IDE
- USBケーブル
- スピーカー
- DFPlayer mini
  
  ## 配線図

| DFPlayer Mini | ESP32 GPIO | 備考           |
|:-------------:|:----------:|:---------------|
| VCC           | 5V         | 電源           |
| GND           | GND        | グラウンド     |
| TX            | GPIO0      | ESP32 RX (Serial1) |
| RX            | GPIO4      | ESP32 TX (Serial1) |
| SPK_1         | スピーカー | 　　　　　　　　　|
| SPK_2         | スピーカー |                |


## 👨‍💻 作者
- [TatsuyaM2667](https://github.com/TatsuyaM2667)

---
## License
MIT License (Attribution Required — © 2025 TatsuyaM2667)

