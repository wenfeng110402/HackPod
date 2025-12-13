//InputManager.cpp
#include "InputManager.h"

InputManager::InputManager() :
    btnPlay(B_PP_PIN, true, true),
    btnNext(B_NEXT_PIN, true, true),
    btnPrev(B_PREV_PIN, true, true),
    btnVolUp(B_VU_PIN, true, true),
    btnVolDown(B_VD_PIN, true, true),
    btnHack(B_HACK_PIN, true, true) {}

void InputManager::begin(ActionCallback callback) {
    _callback = callback;

    // 绑定按键单击事件
    btnPlay.attachClick([](void *scope) { ((InputManager *)scope)->_callback("PLAY"); }, this);
    btnNext.attachClick([](void *scope) { ((InputManager *)scope)->_callback("NEXT"); }, this);
    btnPrev.attachClick([](void *scope) { ((InputManager *)scope)->_callback("PREV"); }, this);
    btnVolUp.attachClick([](void *scope) { ((InputManager *)scope)->_callback("VOLUP"); }, this);
    btnVolDown.attachClick([](void *scope) { ((InputManager *)scope)->_callback("VOLDOWN"); }, this);
    
    // Hack键：单击播报电量，长按切换模式
    btnHack.attachClick([](void *scope) { ((InputManager *)scope)->_callback("HACK_CLICK"); }, this);
    btnHack.attachLongPressStart([](void *scope) { ((InputManager *)scope)->_callback("HACK_LONG"); }, this);
}

void InputManager::loop() {
    btnPlay.tick();
    btnNext.tick();
    btnPrev.tick();
    btnVolUp.tick();
    btnVolDown.tick();
    btnHack.tick();
}