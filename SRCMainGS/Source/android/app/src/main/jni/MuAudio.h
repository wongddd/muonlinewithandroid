#pragma once

#include <cstdint>

// ============================================================================
// MuAudio — OpenSL ES 音频系统（替代 DirectSound + DSPlaySound）
// ============================================================================
//
// 原始接口映射:
//   PlayBuffer(id)     → playSound(id)
//   StopBuffer(id)     → stopSound(id)
//   AllStopSound()     → stopAllSounds()
//   SetVolume(id, vol) → setVolume(id, vol)
//   LoadWaveFile(id)   → loadSound(id, filename)
//   InitDirectSound()  → init()
//   FreeDirectSound()  → destroy()
//
// 架构:
//   - OpenSL ES 引擎 + OutputMix
//   - 16 个缓冲队列播放器池 (并发音效)
//   - WAV 文件 PCM 解析 (RIFF/WAVE, 8/16-bit, mono/stereo)
//   - 音量控制 (0..100, 映射到 millibel)
//   - BGM 通过 Java MediaPlayer (Phase 8.2)

namespace MuAudio {

// ============================================================================
// 公开接口（等价于原始 DSPlaySound API）
// ============================================================================

// 初始化 OpenSL ES 引擎和输出混音器
// assetManager: 用于加载 WAV 文件
bool init(void* assetManager);

// 销毁音频系统
void destroy();

// 加载 WAV 文件到指定 Buffer ID
// id: 音效 ID (对应 DSPlaySound.h 中的 SOUND_* 常量)
// filename: assets 中的文件路径 (如 "Sound/Click_01.wav")
bool loadSound(int id, const char* filename);

// 播放音效
// id: 音效 ID
// loop: 是否循环播放
// 返回: 播放槽位索引 (-1 = 失败/无空闲槽位)
int playSound(int id, bool loop = false);

// 停止播放指定音效 ID 的所有实例
void stopSound(int id);

// 停止所有音效 (不释放音效数据)
void stopAllSounds();

// 设置音效音量
// id: 音效 ID (对后续播放生效)
// volume: 0..100 (0 = 静音, 100 = 最大)
void setVolume(int id, int volume);

// 设置主音量 (影响所有音效)
// volume: 0..100
void setMasterVolume(int volume);

// 暂停/恢复所有音频 (应用生命周期)
void pauseAll();
void resumeAll();

// 检查音效是否已加载
bool isSoundLoaded(int id);

// ============================================================================
// 辅助
// ============================================================================

// 应用进入后台/恢复前台时调用
void onPause();
void onResume();

} // namespace MuAudio
