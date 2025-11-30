#include "AudioManager.h"

AudioManager audioMgr; // 定义全局对象

void AudioManager::begin() {
    // 初始化 SD_MMC (使用 config.h 中的自定义引脚)
    // 注意: SD_CLX_PIN 是您 config.h 中的定义。标准是 SD_CLK_PIN。
    SD_MMC.setPins(SD_CLX_PIN, SD_CMD_PIN, SD_D0_PIN); 
    if (!SD_MMC.begin("/sdcard", true)) { // true 表示使用 1-bit 模式，可以根据需要改为 false 使用 4-bit
        Serial.println("SD Card Mount Failed!");
        return;
    }
    Serial.println("SD Card Mounted.");
    
    // 加载 /music 目录下的音乐文件
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
    Serial.printf("Playback %s\n", _audio.isPaused() ? "Paused" : "Resumed");
}

void AudioManager::next() {
    if (_playlist.empty()) return;
    playNextTrack(); // 调用新的函数来处理下一首的逻辑
}

void AudioManager::prev() {
    if (_playlist.empty()) return;
    // 上一曲逻辑
    _currentTrackIndex--;
    if (_currentTrackIndex < 0) {
        _currentTrackIndex = _playlist.size() - 1; // 回到列表末尾
    }
    playTrack(_currentTrackIndex);
}

void AudioManager::setVolume(int vol) {
    _volume = constrain(vol, 0, 21);
    _audio.setVolume(_volume);
    Serial.printf("Set Volume: %d\n", _volume);
}

void AudioManager::changeVolume(int change) {
    setVolume(_volume + change);
}

void AudioManager::playSystemSound(String filename) {
    // 确保播放系统音时不会被其他操作打断
    if(_audio.isRunning() && !_isPlayingSystem) {
        _audio.stopSong(); // 停止当前音乐
    }
    _isPlayingSystem = true;
    _audio.connecttoFS(SD_MMC, filename.c_str());
    Serial.print("Playing system sound: "); Serial.println(filename);
}

// 音频播放结束时的回调
void AudioManager::onEndOfStream() {
    Serial.println("End of stream detected.");
    if (_isPlayingSystem) {
        _isPlayingSystem = false; // 系统音播放完毕
        // 系统音播完后，停止播放，等待用户操作 (如果之前有音乐在放，需要在此处恢复，但目前简化处理)
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
    if (_playMode == PLAYMODE_RANDOM) {
        shufflePlaylist(); // 如果切换到随机模式，则打乱列表
        // 播放随机列表的第一首 (如果当前是第一首，实际会播放下一首)
        if (!_shuffledPlaylist.empty()) {
            _currentTrackIndex = 0; // 从随机列表的开头开始
            playTrack(_currentTrackIndex);
        }
    } else { // 循环播放模式，回到原始列表的当前歌曲
        // 确保当前歌曲在原始列表中的索引正确
        if (_currentTrackIndex != -1 && !_shuffledPlaylist.empty() && !_playlist.empty()) {
            String currentSong = _shuffledPlaylist[_currentTrackIndex];
            auto it = std::find(_playlist.begin(), _playlist.end(), currentSong);
            if (it != _playlist.end()) {
                _currentTrackIndex = std::distance(_playlist.begin(), it);
            } else {
                _currentTrackIndex = 0; // 找不到就从第一首开始
            }
        }
        playTrack(_currentTrackIndex); // 播放原始列表的当前歌曲
    }

    Serial.print("Play mode set to: ");
    if (mode == PLAYMODE_LOOP_ALL) Serial.println("Loop All");
    else if (mode == PLAYMODE_RANDOM) Serial.println("Random");
}

// 简单的文件夹扫描构建播放列表
void AudioManager::loadPlaylist(const char *dirname) {
    _playlist.clear();
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
                 // 确保存储完整路径，SD_MMC路径以 /sdcard 开头
                _playlist.push_back(String(dirname) + "/" + fileName);
                Serial.print("Found: "); Serial.println(_playlist.back());
            }
        }
        file = root.openNextFile();
    }
    root.close();
    Serial.printf("Loaded %d tracks.\n", _playlist.size());
    shufflePlaylist(); // 初次加载也打乱一次，用于随机模式
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
        Serial.printf("Invalid track index: %d\n", index);
    }
}

// 根据当前播放模式播放下一首
void AudioManager::playNextTrack() {
    if (_playlist.empty()) return;

    // 根据播放模式选择列表
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


// --- 连接第三方库的回调函数 ---
void audio_eof_mp3(const char *info) {
    Serial.print("EOF (MP3/WAV): "); Serial.println(info);
    audioMgr.onEndOfStream(); // 播放结束时调用 AudioManager 的回调
}

// 其他 audioI2S 库可能的回调函数 (仅作示例，不一定需要实现)
void audio_info(const char *info) {
    Serial.print("INFO: "); Serial.println(info);
}
void audio_id3data(const char *info) { // ID3标签信息
    // Serial.print("ID3: "); Serial.println(info);
}
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