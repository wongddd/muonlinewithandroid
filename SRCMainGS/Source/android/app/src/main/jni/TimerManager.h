#pragma once

#include <cstdint>
#include <chrono>
#include <functional>
#include <vector>
#include <string>

// TimerManager — 替代 Windows SetTimer/KillTimer 和 CTimer2
//
// 用法:
//   TimerManager::update()  — 每帧调用一次
//   int id = TimerManager::setInterval(5000, []{ ... });        // 每5秒
//   int id = TimerManager::setTimeout(20000, []{ ... });        // 20秒后一次
//   TimerManager::clearTimer(id);

namespace TimerManager {

using TimerCallback = std::function<void()>;

struct TimerEntry {
    int id;
    int64_t intervalMs;     // 0 = 一次性
    int64_t nextFireMs;     // 下次触发时间 (steady_clock epoch)
    TimerCallback callback;
    bool recurring;
};

// 初始化 (以当前时间为基准)
void init();

// 每帧调用: 检查并触发到期定时器
void update();

// 设置循环定时器 (每 intervalMs 毫秒触发一次)
int setInterval(int64_t intervalMs, TimerCallback callback);

// 设置一次性定时器 (delayMs 毫秒后触发一次)
int setTimeout(int64_t delayMs, TimerCallback callback);

// 清除定时器
void clearTimer(int id);

// 获取当前时间 (steady_clock epoch, 毫秒)
int64_t nowMs();

// 调试: 活跃定时器数量
size_t activeCount();

} // namespace TimerManager
