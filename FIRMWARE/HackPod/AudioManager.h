//AudioManager.h
#pragma once
#include <Arduino.h>
#include <SD_MMC.h>
#include <Audio.h> // 需要安装 ESP32-audioI2S 库
#include <vector>
#include <algorithm> // 用于 std::random_shuffle
#include "config.h"

// 定义播放模式
enum PlayMode {
    PLAYMODE_LOOP_ALL,  // 循环播放所有
    PLAYMODE_RANDOM,    // 随机播放
    PLAYMODE_SINGLE_LOOP // 单曲循环 (暂时未实现，但可以在此处定义)
};

class AudioManager {
public:
    AudioManager();
    void begin();
    void loop();
    void playPause();
    void next();
    void prev();
    void setVolume(int vol);
    void changeVolume(int change);
    void playSystemSound(String filename); // 播放系统提示音
    void onEndOfStream(); // 音频播放结束回调

    // 新增：设置和获取播放模式
    void setPlayMode(PlayMode mode);
    PlayMode getPlayMode() { return _playMode; }


private:
    Audio _audio;
    int _volume = DEFAULT_VOLUME;
    std::vector<String> _playlist;      // 原始播放列表
    std::vector<String> _shuffledPlaylist; // 随机模式下的播放列表
    int _currentTrackIndex = -1;
    bool _isPlayingSystem = false;      // 是否正在播放系统音
    PlayMode _playMode = PLAYMODE_LOOP_ALL; // 默认循环播放

    void loadPlaylist(const char *dirname); // 加载音乐文件到播放列表
    void playTrack(int index); // 播放指定索引的歌曲
    void playNextTrack(); // 根据当前播放模式决定下一首
    void shufflePlaylist(); // 打乱播放列表
};

extern AudioManager audioMgr; // 声明全局对象以便回调使用