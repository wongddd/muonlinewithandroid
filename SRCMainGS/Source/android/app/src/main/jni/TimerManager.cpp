#include "TimerManager.h"

#include <android/log.h>
#include <algorithm>

#define LOG_TAG "TimerManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace TimerManager {

static std::vector<TimerEntry> g_timers;
static int g_nextId = 1;
static int64_t g_baseTime = 0;

static int64_t steadyNowMs() {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

void init() {
    g_baseTime = steadyNowMs();
    g_timers.clear();
    g_nextId = 1;
    LOGI("TimerManager initialized");
}

void update() {
    int64_t now = steadyNowMs();

    for (auto& timer : g_timers) {
        if (now >= timer.nextFireMs) {
            if (timer.callback) {
                timer.callback();
            }

            if (timer.recurring) {
                timer.nextFireMs = now + timer.intervalMs;
            } else {
                timer.intervalMs = -1; // 标记为已触发的一次性定时器
            }
        }
    }

    // 清理已触发的一次性定时器
    g_timers.erase(
        std::remove_if(g_timers.begin(), g_timers.end(),
                       [](const TimerEntry& t) { return t.intervalMs < 0 && !t.recurring; }),
        g_timers.end());
}

int setInterval(int64_t intervalMs, TimerCallback callback) {
    int64_t now = steadyNowMs();
    TimerEntry entry;
    entry.id = g_nextId++;
    entry.intervalMs = intervalMs;
    entry.nextFireMs = now + intervalMs;
    entry.callback = std::move(callback);
    entry.recurring = true;
    g_timers.push_back(entry);
    return entry.id;
}

int setTimeout(int64_t delayMs, TimerCallback callback) {
    int64_t now = steadyNowMs();
    TimerEntry entry;
    entry.id = g_nextId++;
    entry.intervalMs = delayMs;
    entry.nextFireMs = now + delayMs;
    entry.callback = std::move(callback);
    entry.recurring = false;
    g_timers.push_back(entry);
    return entry.id;
}

void clearTimer(int id) {
    g_timers.erase(
        std::remove_if(g_timers.begin(), g_timers.end(),
                       [id](const TimerEntry& t) { return t.id == id; }),
        g_timers.end());
}

int64_t nowMs() {
    return steadyNowMs();
}

size_t activeCount() {
    return g_timers.size();
}

} // namespace TimerManager
