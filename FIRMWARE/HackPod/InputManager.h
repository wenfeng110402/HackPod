//InputManager.h
#pragma once
#include <OneButton.h>
#include "config.h"
#include <functional>

// 定义一个回调函数类型，用于通知主程序按键事件
typedef std::function<void(const char*)> ActionCallback;

class InputManager {
public:
    InputManager();
    void begin(ActionCallback callback); // 初始化并绑定回调函数
    void loop(); // 需要在主循环中调用

private:
    OneButton btnPlay;
    OneButton btnNext;
    OneButton btnPrev;
    OneButton btnVolUp;
    OneButton btnVolDown;
    OneButton btnHack;
    ActionCallback _callback;
};