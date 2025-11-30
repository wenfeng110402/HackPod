#include "config.h"
#include "PowerManager.h"
#include "InputManager.h"
#include "AudioManager.h"

// 实例化全局管理器对象
PowerManager powerMgr;
InputManager inputMgr;
// audioMgr 已经在 AudioManager.cpp 中定义

// 全局变量：
bool isAutomixMode = false; // 是否处于 Automix 模式 (现在称为随机播放模式)
static bool hasLowBatteryWarningPlayed = false; // 用于控制低电量警告只播放一次

// --- 核心逻辑：处理按键动作 ---
void handleInputAction(const char* action) {
    Serial.printf("Action: %s\n", action);

    if (strcmp(action, "PLAY") == 0) {
        audioMgr.playPause();
    } else if (strcmp(action, "NEXT") == 0) {
        audioMgr.next();
    } else if (strcmp(action, "PREV") == 0) {
        audioMgr.prev();
    } else if (strcmp(action, "VOLUP") == 0) {
        audioMgr.changeVolume(1);
    } else if (strcmp(action, "VOLDOWN") == 0) {
        audioMgr.changeVolume(-1);
    } else if (strcmp(action, "HACK_CLICK") == 0) {
        // Hack键单击：播报电量
        String batStatusFilePrefix = powerMgr.getBatteryStatusString(); // 返回 bv00, bv01 等
        String audioFile = String(SYSTEM_AUDIO_DIR) + "/" + batStatusFilePrefix + ".wav"; // 拼接成 /system/bv00.wav
        audioMgr.playSystemSound(audioFile);
    } else if (strcmp(action, "HACK_LONG") == 0) {
        // Hack键长按：切换播放模式 (随机播放 / 循环播放)
        PlayMode currentMode = audioMgr.getPlayMode();
        if (currentMode == PLAYMODE_LOOP_ALL) {
            audioMgr.setPlayMode(PLAYMODE_RANDOM);
            audioMgr.playSystemSound(String(SYSTEM_AUDIO_DIR) + "/sn02.wav"); // 播放切换到随机模式的提示音
            Serial.println("Switched to Random Play Mode.");
        } else { // 当前是随机播放模式，切换回循环播放
            audioMgr.setPlayMode(PLAYMODE_LOOP_ALL);
            audioMgr.playSystemSound(String(SYSTEM_AUDIO_DIR) + "/sn03.wav"); // 播放切换到循环模式的提示音
            Serial.println("Switched to Loop All Mode.");
        }
        // LED 指示：长按 Hack 键时，LED 闪烁一下或改变状态
        digitalWrite(LED_STAT_PIN, HIGH);
        delay(100);
        digitalWrite(LED_STAT_PIN, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- HackPod Starting ---");

    // 初始化 LED
    pinMode(LED_STAT_PIN, OUTPUT);
    digitalWrite(LED_STAT_PIN, LOW);

    // 初始化各个模块
    powerMgr.begin();
    // 传入回调函数，让 InputManager 知道按键触发时通知谁
    inputMgr.begin(handleInputAction);
    audioMgr.begin();

    // 播放开机音 (如果需要，可以在 system 文件夹放一个 power_on.wav)
    // audioMgr.playSystemSound(String(SYSTEM_AUDIO_DIR) + "/power_on.wav");

    Serial.println("--- Ready ---");
}

void loop() {
    // 必须在主循环中不断调用这些函数
    audioMgr.loop();
    inputMgr.loop();

    // --- 1. 电池状态更新与打印 & 低电量警告 ---
    // 每隔5秒读取并打印一次电池信息
    static unsigned long lastBatteryReadTime = 0;
    if (millis() - lastBatteryReadTime > 5000) { // 每5秒读取一次
        float voltage = powerMgr.getBatteryVoltage();
        int percentage = powerMgr.getBatteryPercentage();
        // getBatteryStatusString 现在返回 "bv00" 等文件名部分，这里仅用于打印
        String statusPrefix = powerMgr.getBatteryStatusString(); 

        Serial.print("Battery: ");
        Serial.print(voltage, 2); 
        Serial.print("V (");
        Serial.print(percentage);
        Serial.print("%), Status: ");
        Serial.println(statusPrefix); // 打印文件名后缀

        // 低电量警告逻辑 (仅当电量低于10%且未播放过警告时才播放)
        if (percentage <= 10 && !hasLowBatteryWarningPlayed) {
            audioMgr.playSystemSound(String(SYSTEM_AUDIO_DIR) + "/bn00.wav"); // 播放低电量警告音
            Serial.println("!! Low Battery Warning (10%) !!");
            hasLowBatteryWarningPlayed = true; // 设置标志，防止重复播放
        } else if (percentage > 10) {
            hasLowBatteryWarningPlayed = false; // 电量回升后重置警告标志
        }

        // LED 指示：如果电量低于20%，LED 慢速闪烁
        if (percentage < 20) {
            digitalWrite(LED_STAT_PIN, (millis() / 1000) % 2); // 每1秒闪烁一次
        } else if (audioMgr.getPlayMode() == PLAYMODE_RANDOM) {
            // 如果处于随机播放模式，LED 保持常亮（或者其他指示方式）
            digitalWrite(LED_STAT_PIN, HIGH);
        }
        else {
            digitalWrite(LED_STAT_PIN, LOW); // 正常电量且非随机模式，LED 关闭
        }

        lastBatteryReadTime = millis();
    }
    
    // --- 2. Automix 模式 LED 指示 (现在是随机播放模式) ---
    // 如果是随机播放模式，LED 持续亮起（或者有规律闪烁）
    // 这个逻辑已经整合到上面的电池状态 LED 逻辑中，避免冲突
    
}