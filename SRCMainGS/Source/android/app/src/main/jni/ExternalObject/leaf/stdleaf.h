#pragma once
// Stub leaf/stdleaf.h — shadows the original for Android builds.
// Only GetTimeString is used by WSclient.cpp.
#include <string>
#include <ctime>

namespace leaf {
    inline void GetTimeString(std::string& str) {
        time_t now = time(nullptr);
        struct tm tm_buf;
        struct tm* t = localtime(&now);
        char buf[64];
        if (t) {
            snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02d:000",
                     t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                     t->tm_hour, t->tm_min, t->tm_sec);
        } else {
            snprintf(buf, sizeof(buf), "2000/01/01 00:00:00:000");
        }
        str = buf;
    }
}
