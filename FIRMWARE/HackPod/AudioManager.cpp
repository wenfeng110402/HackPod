//AudioManager.cpp
#include "AudioManager.h"
#include <FS.h>
#include "SD_MMC.h" 
#include <algorithm> 
#include <vector>    
#include "config.h" // 包含所有引脚和常量定义

// ===============================================
// 1. 构造函数实现 (修复链接错误: undefined reference to 'AudioManager::AudioManager()')
// ===============================================

AudioManager::AudioManager() {
    // 初始化所有私有成员变量到默认值，使用 config.h 中的定义
    _volume = DEFAULT_VOLUME; 
    _currentTrackIndex = -1; // -1 表示没有歌曲在播放
    _playMode = PLAYMODE_LOOP_ALL; 
    _isPlayingSystem = false;
    Serial.println("AudioManager object constructed.");
}

// ===============================================
// 2. 全局对象定义和回调函数
// ===============================================

// 定义全局的 AudioManager 对象
AudioManager audioMgr; 

// --- 连接第三方库的回调函数 ---
// 在播放结束时，库会调用这个函数
void audio_eof_mp3(const char *info) {
    Serial.print("EOF (MP3/WAV): "); Serial.println(info);
    audioMgr.onEndOfStream(); // 播放结束时调用 AudioManager 的回调
}

// 其他 audioI2S 库可能的回调函数 (仅作示例，可根据需要添加实现)
void audio_info(const char *info) {
    Serial.print("INFO: "); Serial.println(info);
}
void audio_id3data(const char *info) { }
void audio_showstation(const char *info) {}
void audio_showstreaminfo(const char *info) {}
void audio_bitrate(const char *info) {}
void audio_commercial(const char *info) {}
void audio_icyurl(const char *info) {}
void audio_lasthost(const char *info) {}
void audio_loop(const char *info) {}
void audio_volume(int vol) {}
void audio_decode_err(int err) { // 解码错误
    Serial.printf("Decode Error: %d\n", err);
}


// ===============================================
// 3. AudioManager 成员函数实现
// ===============================================

void AudioManager::begin() {
    // 初始化 SD_MMC
    SD_MMC.setPins(SD_CLX_PIN, SD_CMD_PIN, SD_D0_PIN); 
    if (!SD_MMC.begin("/sdcard", true)) { // true 表示使用 1-bit 模式
        Serial.println("SD Card Mount Failed!");
        return;
    }
    Serial.println("SD Card Mounted.");
    
    // 加载 /music 目录下的音乐文件 (直接硬编码路径，避免使用 config.h 中不存在的宏)
    loadPlaylist("/music"); 

    // 初始化音频 I2S
    _audio.setPinout(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DATA_PIN);
    _audio.setVolume(_volume);
    Serial.println("Audio Initialized.");

    // 如果有音乐，自动开始播放第一首
    if (!_playlist.empty()) {
        playTrack(0); // 播放列表第一首
    }
}

void AudioManager::loop() {
    _audio.loop();
}

void AudioManager::playPause() {
    _audio.pauseResume();
    // 兼容旧库：直接打印操作，避免调用 isPaused()
    Serial.println("Playback status toggled (Paused/Resumed).");
}

void AudioManager::next() {
    if (_playlist.empty()) return;
    playNextTrack(); // 调用函数来处理下一首的逻辑
}

void AudioManager::prev() {
    if (_playlist.empty()) return;
    // 上一曲逻辑
    _currentTrackIndex--;
    if (_currentTrackIndex < 0) {
        // 回到列表末尾
        _currentTrackIndex = (_playMode == PLAYMODE_RANDOM && !_shuffledPlaylist.empty()) ? _shuffledPlaylist.size() - 1 : _playlist.size() - 1; 
    }
    playTrack(_currentTrackIndex);
}

void AudioManager::setVolume(int vol) {
    // 使用硬编码的最大音量 21 (ESP32-audioI2S 库的常见最大值)
    // 避免使用 config.h 中不存在的 MAX_VOLUME
    _volume = constrain(vol, 0, 21); 
    _audio.setVolume(_volume);
    Serial.printf("Set Volume: %d\n", _volume);
}

void AudioManager::changeVolume(int change) {
    setVolume(_volume + change);
}

void AudioManager::playSystemSound(String filename) {
    // 构造系统音文件的完整路径，使用您 config.h 中的宏 SYSTEM_AUDIO_DIR
    String path = String(SYSTEM_AUDIO_DIR) + "/" + filename;
    
    // 确保播放系统音时不会被其他操作打断
    if(_audio.isRunning() && !_isPlayingSystem) {
        _audio.stopSong(); // 停止当前音乐
    }
    
    _isPlayingSystem = true;
    _audio.connecttoFS(SD_MMC, path.c_str());
    Serial.print("Playing system sound: "); Serial.println(path);
}

// 音频播放结束时的回调
void AudioManager::onEndOfStream() {
    Serial.println("End of stream detected.");
    if (_isPlayingSystem) {
        _isPlayingSystem = false; // 系统音播放完毕
        _audio.stopSong(); 
        Serial.println("System sound finished.");
    } else {
        // 正常音乐播完，根据播放模式播放下一首
        playNextTrack();
    }
}

// 设置播放模式
void AudioManager::setPlayMode(PlayMode mode) {
    _playMode = mode;

    if (mode == PLAYMODE_RANDOM) {
        shufflePlaylist(); 
        if (!_shuffledPlaylist.empty()) {
            _currentTrackIndex = 0; 
            playTrack(_currentTrackIndex);
        }
    } else { 
        // 切换回循环模式时，确保索引指向正确的歌曲
        if (_currentTrackIndex != -1 && !_shuffledPlaylist.empty() && !_playlist.empty()) {
            String currentSong = _shuffledPlaylist[_currentTrackIndex];
            auto it = std::find(_playlist.begin(), _playlist.end(), currentSong);
            if (it != _playlist.end()) {
                _currentTrackIndex = std::distance(_playlist.begin(), it);
            } else {
                _currentTrackIndex = 0; 
            }
        }
        playTrack(_currentTrackIndex); 
    }

    Serial.print("Play mode set to: ");
    if (mode == PLAYMODE_LOOP_ALL) Serial.println("Loop All (sn03.wav)");
    else if (mode == PLAYMODE_RANDOM) Serial.println("Random (sn02.wav)");
}

// 简单的文件夹扫描构建播放列表
void AudioManager::loadPlaylist(const char *dirname) {
    _playlist.clear();
    _shuffledPlaylist.clear();

    File root = SD_MMC.open(dirname);
    if (!root || !root.isDirectory()) {
        Serial.printf("Failed to open music directory: %s\n", dirname);
        return;
    }
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String fileName = String(file.name());
            // 支持 .mp3 和 .wav 文件
            if (fileName.endsWith(".mp3") || fileName.endsWith(".MP3") || 
                fileName.endsWith(".wav") || fileName.endsWith(".WAV")) {
                
                // 存储完整路径
                _playlist.push_back(String(dirname) + "/" + fileName);
                Serial.print("Found: "); Serial.println(_playlist.back());
            }
        }
        file = root.openNextFile();
    }
    root.close();
    Serial.printf("Loaded %d tracks.\n", _playlist.size());
    shufflePlaylist(); 
}

// 播放指定索引的歌曲 (根据当前播放模式选择列表)
void AudioManager::playTrack(int index) {
    if (_playlist.empty()) {
        Serial.println("Playlist is empty.");
        return;
    }

    const std::vector<String>& currentList = (_playMode == PLAYMODE_RANDOM && !_shuffledPlaylist.empty()) ? _shuffledPlaylist : _playlist;

    if (index >= 0 && index < currentList.size()) {
        Serial.print("Playing: "); Serial.println(currentList[index]);
        _audio.connecttoFS(SD_MMC, currentList[index].c_str());
        _currentTrackIndex = index;
    } else {
        Serial.printf("Invalid track index: %d. Resetting index to 0.\n", index);
        _currentTrackIndex = 0; // 重置索引
    }
}

// 根据当前播放模式播放下一首
void AudioManager::playNextTrack() {
    if (_playlist.empty()) return;

    const std::vector<String>& currentList = (_playMode == PLAYMODE_RANDOM && !_shuffledPlaylist.empty()) ? _shuffledPlaylist : _playlist;

    _currentTrackIndex++;
    if (_currentTrackIndex >= currentList.size()) {
        _currentTrackIndex = 0; // 循环到列表开头
    }
    playTrack(_currentTrackIndex);
}

// 打乱播放列表
void AudioManager::shufflePlaylist() {
    _shuffledPlaylist = _playlist; // 复制原始列表
    if (!_shuffledPlaylist.empty()) {
        std::random_shuffle(_shuffledPlaylist.begin(), _shuffledPlaylist.end()); 
        Serial.println("Playlist shuffled.");
    }
}