#pragma once
// Stub DSPlaySound.h — shadows the original for Android builds.
// Audio functions are no-ops (real audio handled by MuAudio / OpenSL ES).
// The SOUND_* constants are preserved so WSclient.cpp compiles unchanged.

#define MAX_CHANNEL 4

// Sound buffer IDs used by WSclient.cpp
#define SOUND_WIND01               0
#define SOUND_RAIN01               1
#define SOUND_FOREST01             2
#define SOUND_DUNGEON01            3
#define SOUND_TITLE01              4
#define SOUND_TOWER01              5
#define SOUND_WATER01              6
#define SOUND_DESERT01             7
#define SOUND_HUMAN_WALK_GROUND    8
#define SOUND_HUMAN_WALK_GRASS     9
#define SOUND_HUMAN_WALK_SNOW      10
#define SOUND_HUMAN_WALK_SWIM      11
#define SOUND_BIRD01               12
#define SOUND_BIRD02               13
#define SOUND_BAT01                14
#define SOUND_RAT01                15
#define SOUND_TRAP01               16
#define SOUND_DOOR01               17
#define SOUND_DOOR02               18
#define SOUND_ASSASSIN             19
#define SOUND_HEAVEN01             20
#define SOUND_THUNDERS01           21
#define SOUND_THUNDERS02           22
#define SOUND_THUNDERS03           23
#define SOUND_CLICK01              25
#define SOUND_ERROR01              26
#define SOUND_MENU01               27
#define SOUND_INTERFACE01          28
#define SOUND_GET_ITEM01           29
#define SOUND_DROP_ITEM01          30
#define SOUND_DROP_GOLD01          31
#define SOUND_DRINK01              32
#define SOUND_EAT_APPLE01          33
#define SOUND_HEART                34
#define SOUND_GET_ENERGY           35
#define SOUND_SHOUT01              36
#define SOUND_REPAIR               37
#define SOUND_WHISPER              38
#define SOUND_ALERT                39
#define SOUND_FRIEND_CHAT_ALERT    40
#define SOUND_BLOODCASTLE          41
#define SOUND_CHAOSCASTLE          42
#define SOUND_CHAOS_ENVIR          43
#define SOUND_EMPIREGUARDIAN_WEATHER_RAIN   44
#define SOUND_EMPIREGUARDIAN_WEATHER_FOG    45
#define SOUND_EMPIREGUARDIAN_WEATHER_STORM  46
#define SOUND_EMPIREGUARDIAN_INDOOR_SOUND   47
#define SOUND_MIX01                48
#define SOUND_BREAK01              49
#define SOUND_MONSTER              50
#define SOUND_MONSTER_ATTACK1      51
#define SOUND_MONSTER_DIE1         52
#define MAX_BUFFER                 256

struct OBJECT;

inline void LoadWaveFile(int, char*, int = MAX_CHANNEL, bool = false) {}
inline void PlayBuffer(int, OBJECT* = nullptr, bool = false) {}
inline void StopBuffer(int, bool = false) {}
inline void AllStopSound() {}
inline void FreeDirectSound() {}
