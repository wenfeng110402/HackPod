// PowerManager.h
#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>   // 包含Arduino基本类型，例如float, String
#include "config.h"    // 包含我们的项目配置信息

// PowerManager 类定义
class PowerManager {
public:
    PowerManager(); // 构造函数
    void begin();   // 初始化函数，用于设置ADC等

    float getBatteryVoltage();      // 获取当前电池电压 (V)
    int getBatteryPercentage();     // 获取当前电池百分比 (%)
    String getBatteryStatusString(); // 获取电池状态字符串 (例如 "high", "medium", "low")

private:
    // 将原始ADC读数转换为ADC引脚上的电压
    float rawToAdcPinVoltage(int rawADC);
    // 从ADC引脚电压反推出电池原始电压 (考虑分压电阻)
    float adcPinVoltageToBatteryVoltage(float adcPinVoltage);
    // 将电压转换为百分比
    int voltageToPercentage(float voltage);
    // 判断电池状态
    String getVoltageStatus(float voltage);

    // ADC校准特性结构体，如果后续需要更精确的校准
    // esp_adc_cal_characteristics_t adc_chars; 
};

#endif // POWER_MANAGER_H