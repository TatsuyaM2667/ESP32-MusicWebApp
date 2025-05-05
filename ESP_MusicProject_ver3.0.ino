#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "DFRobotDFPlayerMini.h"

// WiFi設定
const char* ssid = "ESP32_Music";
const char* password = "";

// DNSサーバーとWebサーバーの初期化
DNSServer dnsServer;
WebServer server(80);
const byte DNS_PORT = 53;

// DFPlayer Mini設定
HardwareSerial myHardwareSerial(1); // Serial1
DFRobotDFPlayerMini myDFPlayer;

// 再生時間管理
unsigned long previousMillis = 0;
unsigned long interval = 1000; // 1秒間隔
int elapsedTime = 0;
int trackLength = 180; // 曲の長さ（秒）
bool isPlaying = false; // 再生状態を追跡

// 現在のイコライザモード
int currentEqMode = 0; // 初期値: NORMAL

// HTMLコンテンツ

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>オーディオプレイヤー</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, #141e30, #243b55);
            color: #fff;
            margin: 0;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .player-container {
            position: relative;
            display: flex;
            align-items: center;
            justify-content: center;
            width: 250px;
            height: 250px;
            background: radial-gradient(circle, #4e73df, #2a518c);
            border-radius: 50%;
            box-shadow: 0 0 30px rgba(78, 115, 223, 0.6);
            overflow: hidden;
        }

        .wave {
            position: absolute;
            top: 50%;
            left: 50%;
            width: 0;
            height: 0;
            border-radius: 50%;
            background: rgba(78, 115, 223, 0.3);
            transform: translate(-50%, -50%);
            animation: none;
        }

        .wave.active {
            animation: expandCircle 1.5s infinite;
        }

        @keyframes expandCircle {
            0% {
                width: 0;
                height: 0;
                opacity: 0.8;
            }
            100% {
                width: 250px;
                height: 250px;
                opacity: 0;
            }
        }

        .horizontal-controls {
            margin-top: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 10px;
        }

        .vertical-controls {
            display: flex;
            flex-direction: column;
            gap: 10px;
            margin-top: 20px;
            width: 300px;
        }

        .button {
            background: white;
            color: #4e73df;
            font-size: 1rem;
            font-weight: bold;
            padding: 10px 15px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            box-shadow: 0 4px 10px rgba(0, 0, 0, 0.2);
        }

        .button:hover {
            background: #f0f0f0;
        }

        .progress-bar, .volume-control, .equalizer {
            width: 100%;
        }

        .progress-bar input, .volume-control input {
            width: 100%;
        }

        .equalizer select {
            width: 100%;
            padding: 5px;
        }

        .time-display,
        .volume-display {
            font-size: 1.2rem;
            font-weight: bold;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="player-container">
        <div class="wave"></div>
        <div class="wave"></div>
        <div class="wave"></div>
    </div>
    <div class="horizontal-controls">
        <button id="prev" class="button">⏮️ 前の曲</button>
        <button id="play-pause" class="button">▶️ 再生</button>
        <button id="next" class="button">次の曲 ⏭️</button>
    </div>
    <div class="vertical-controls">
        <div class="progress-bar">
            <label for="progress">進行:</label>
            <input type="range" id="progress" min="0" max="100" value="0" readonly>
        </div>
        <div class="time-display" id="time-display">
            00:00 / 00:00
        </div>
        <div class="volume-control">
            <label for="volume">音量:</label>
            <input type="range" id="volume" min="0" max="30" value="15">
            <div class="volume-display" id="volume-display">
                音量: 15
            </div>
        </div>
        <div class="equalizer">
            <label for="equalizer">イコライザ:</label>
            <select id="equalizer">
                <option value="0">NORMAL</option>
                <option value="1">POP</option>
                <option value="2">ROCK</option>
                <option value="3">JAZZ</option>
                <option value="4">CLASSIC</option>
                <option value="5">BASS</option>
            </select>
        </div>
    </div>
    <script>
        const playPauseButton = document.getElementById('play-pause');
        const prevButton = document.getElementById('prev');
        const nextButton = document.getElementById('next');
        const volumeSlider = document.getElementById('volume');
        const volumeDisplay = document.getElementById('volume-display');
        const equalizerDropdown = document.getElementById('equalizer');
        const progressBar = document.getElementById('progress');
        const timeDisplay = document.getElementById('time-display');
        const waves = document.querySelectorAll('.wave');
        let isPlaying = false;

        // 再生/停止の切り替え
        playPauseButton.addEventListener('click', () => {
            isPlaying = !isPlaying;
            playPauseButton.textContent = isPlaying ? '⏸️ 停止' : '▶️ 再生';
            waves.forEach(wave => wave.classList.toggle('active', isPlaying));
            fetch(`/command?cmd=${isPlaying ? 'play' : 'pause'}`);
        });

        // 前の曲
        prevButton.addEventListener('click', () => {
            fetch('/command?cmd=prev');
        });

        // 次の曲
        nextButton.addEventListener('click', () => {
            fetch('/command?cmd=next');
        });

        // 音量調整
        volumeSlider.addEventListener('input', (event) => {
            const volume = event.target.value;
            volumeDisplay.textContent = `音量: ${volume}`;
            fetch(`/command?cmd=volume&value=${volume}`);
        });

        // イコライザ変更
        equalizerDropdown.addEventListener('change', (event) => {
            const eqSetting = event.target.value;
            fetch(`/command?cmd=eq&value=${eqSetting}`);
        });

        // 時間のフォーマット変換
        function formatTime(seconds) {
            const mins = Math.floor(seconds / 60);
            const secs = seconds % 60;
            return `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`;
        }

        // 再生状況の取得と更新
        setInterval(() => {
            fetch('/track-status')
                .then(response => response.json())
                .then(data => {
                    progressBar.max = data.trackLength;
                    progressBar.value = data.elapsedTime;
                    timeDisplay.textContent = `${formatTime(data.elapsedTime)} / ${formatTime(data.trackLength)}`;
                });
        }, 1000);
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.sendHeader("Content-Type", "text/html; charset=UTF-8");
    server.send_P(200, "text/html", htmlPage);
}

// コマンド処理
void handleCommand() {
    if (server.hasArg("cmd")) {
        String cmd = server.arg("cmd");
        Serial.println(cmd);

        if (cmd == "play") {
            myDFPlayer.start();
            isPlaying = true;
        } else if (cmd == "pause") {
            myDFPlayer.pause();
            isPlaying = false;
        } else if (cmd == "next") {
            myDFPlayer.next();
            elapsedTime = 0;
        } else if (cmd == "prev") {
            myDFPlayer.previous();
            elapsedTime = 0;
        } else if (cmd == "volume" && server.hasArg("value")) {
            int volume = server.arg("value").toInt();
            myDFPlayer.volume(volume);
        } else if (cmd == "eq" && server.hasArg("value")) {
            int eqSetting = server.arg("value").toInt();
            myDFPlayer.EQ(eqSetting);
            currentEqMode = eqSetting;
        }
    }
    server.send(200, "text/plain", "OK");
}

// 再生状況をJSONで送信
void handleTrackStatus() {
    unsigned long currentMillis = millis();
    if (isPlaying && currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        elapsedTime++;
        if (elapsedTime >= trackLength) {
            myDFPlayer.next();
            elapsedTime = 0;
        }
    }
    String json = "{\"elapsedTime\":" + String(elapsedTime) +
                  ",\"trackLength\":" + String(trackLength) + "}";
    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);

    // DFPlayer Miniの初期化
    myHardwareSerial.begin(9600, SERIAL_8N1, 0, 4); // RX: GPIO4, TX: GPIO0
    if (!myDFPlayer.begin(myHardwareSerial)) {
        Serial.println("DFPlayer Mini 初期化失敗");
        while (true); // 初期化失敗時に停止
    }
    myDFPlayer.volume(15); // 初期音量設定

    // WiFiアクセスポイント設定
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("アクセスポイントIP: ");
    Serial.println(myIP);

    // DNSサーバーの開始
    dnsServer.start(DNS_PORT, "*", myIP);

    // サーバールートのエンドポイント設定
    server.on("/", handleRoot); // ホームページを提供
    server.on("/command", handleCommand); // 再生や停止、次/前の曲などの操作コマンド
    server.on("/track-status", handleTrackStatus); // 再生時間やトラック情報の提供

    // サーバーの開始
    server.begin();
    Serial.println("HTTPサーバー開始完了");
}
void loop() {
    // DNSリクエストの処理
    dnsServer.processNextRequest();

    // HTTPクライアントリクエストの処理
    server.handleClient();

    // DFPlayer Miniのシリアルデータを確認
    static bool trackEnded = false; // トラック終了状態を追跡

    if (myDFPlayer.available()) {
        uint8_t type = myDFPlayer.readType(); // イベントタイプを取得

        // 曲が終了した場合の処理
        if (type == DFPlayerPlayFinished && !trackEnded) {
            trackEnded = true; // トラック終了状態を設定
            myDFPlayer.next(); // 次のトラックを再生
            elapsedTime = 0;   // 再生時間をリセット
        }
    }

    // 再生中の場合、経過時間を更新
    if (isPlaying) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis; // 最終チェック時間を更新
            elapsedTime++;                  // 経過時間を加算
        }
    }

    // トラック終了状態をリセット（再生中の場合）
    if (myDFPlayer.read() == 1) { // "1"は再生中の状態を表すと仮定
        trackEnded = false; // 再生中なら終了状態をリセット
    }
}
