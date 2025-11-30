// PowerManager.cpp - 匹配详细版 PowerManager.h 的实现

#include "PowerManager.h"

// 构造函数：在创建 PowerManager 对象时运行
PowerManager::PowerManager() {
    // 目前没有需要在这里立即初始化的成员变量
}

// 初始化函数：在 setup() 中调用，用于设置引脚等
void PowerManager::begin() {
    pinMode(BAT_ADC_PIN, INPUT); 
}

// --- 私有辅助函数实现 ---

// 功能：将 ADC 的原始读数（例如 0 到 4095）转换为 ADC 引脚上的实际电压。
float PowerManager::rawToAdcPinVoltage(int rawADC) {
    return (float)rawADC * (ADC_REFERENCE_VOLTAGE / ADC_MAX_READING);
}

// 功能：根据 ADC 引脚上的电压，结合分压电阻的比例，反推出电池的实际电压。
float PowerManager::adcPinVoltageToBatteryVoltage(float adcPinVoltage) {
    return adcPinVoltage * ((VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2);
}

// 功能：将电池电压转换为一个 0-100 的百分比。
int PowerManager::voltageToPercentage(float voltage) {
    if (voltage >= BATTERY_MAX_VOLT) return 100;
    if (voltage <= BATTERY_MIN_VOLT) return 0;

    float range = BATTERY_MAX_VOLT - BATTERY_MIN_VOLT;
    float currentLevel = voltage - BATTERY_MIN_VOLT;
    int percentage = (int)((currentLevel / range) * 100.0f);
    
    return constrain(percentage, 0, 100);
}

// 功能：根据电压判断当前的电量状态，并返回相应的字符串。
// 注意：这个函数返回的 "high", "medium", "low" 是内部逻辑，
//      外部调用时需要根据这些状态播放对应的bvXX.wav文件。
String PowerManager::getVoltageStatus(float voltage) {
    int percentage = voltageToPercentage(voltage); 
    
    if (percentage >= 75) { // 对应 bv00.wav
        return "bv00";
    } else if (percentage >= 50) { // 对应 bv01.wav
        return "bv01";
    } else if (percentage >= 25) { // 对应 bv02.wav
        return "bv02";
    } else if (percentage >= 10) { // 对应 bv03.wav
        return "bv03";
    } else { // 对应 bv04.wav
        return "bv04";
    }
}

// --- 公有功能函数实现 ---

// 功能：获取电池当前的实际电压。
float PowerManager::getBatteryVoltage() {
    long sumADC = 0;
    for (int i = 0; i < BATTERY_SAMPLES; i++) {
        sumADC += analogRead(BAT_ADC_PIN);
        delay(1);
    }
    int avgADC = sumADC / BATTERY_SAMPLES;

    float pinVoltage = rawToAdcPinVoltage(avgADC);
    return adcPinVoltageToBatteryVoltage(pinVoltage);
}

// 功能：获取电池当前的电量百分比。
int PowerManager::getBatteryPercentage() {
    float voltage = getBatteryVoltage();
    return voltageToPercentage(voltage);
}

// 功能：获取电池的当前状态描述字符串 (现在直接返回 bvXX 格式的文件名部分)。
String PowerManager::getBatteryStatusString() {
    float voltage = getBatteryVoltage();
    return getVoltageStatus(voltage);
}