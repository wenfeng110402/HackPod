#ifndef CONFIG_H
#define CONFIG_H

// 1.i2s dac
#define I2S_BCK_PIN 12
#define I2S_LRCK_PIN 11
#define I2S_DATA_PIN 10    //data out pin

// 2.sdcard
#define SD_CLX_PIN 46
#define SD_CMD_PIN 45
#define SD_D0_PIN 38
#define SD_CD_PIN 37

// 3.buttons
#define B_PP_PIN 4 //play/pause
#define B_NEXT_PIN 6
#define B_PREV_PIN 5
#define B_VU_PIN 7
#define B_VD_PIN 8
#define B_HACK_PIN 9

// 4.BattVolt ADC
#define BAT_ADC_PIN 14

// 5.LEDStatus
#define LED_STAT_PIN 33

// --- 6. 其他配置常量 ---
#define DEFAULT_VOLUME      15 // 默认音量 (ESP32-audioI2S 库通常是 0-21)
#define BATTERY_MAX_VOLT    4.20f // 电池充满时的电压
#define BATTERY_MIN_VOLT    3.30f // 电池低电量时的电压 (低于此值可能关机或警告)
#define BATTERY_SAMPLES     50   // ADC 读取平均采样次数，用于提高精度，减少噪声
// 电池分压电阻值，用于计算实际电压
#define VOLTAGE_DIVIDER_R1  100000.0f // R_BAT_TOP 100k欧姆
#define VOLTAGE_DIVIDER_R2  200000.0f // R_BAT_BOTTOM 200k欧姆
#define ADC_REFERENCE_VOLTAGE 3.3f    // ESP32-S3 ADC 的参考电压 (LDO输出给ESP32的电压)
#define ADC_MAX_READING     4095.0f   // ESP32-S3 ADC 的最大读数 (12位ADC)

// --- 7. 系统文件路径 ---
// 用于存放电量播报音频的文件夹和文件名前缀
#define SYSTEM_AUDIO_DIR    "/system"
#define BAT_AUDIO_PREFIX    "battery_" // 例如 battery_high.mp3, battery_medium.mp3

#endif // CONFIG_H 结束宏定义
