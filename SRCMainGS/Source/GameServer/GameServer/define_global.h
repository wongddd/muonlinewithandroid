#pragma once
#define EQUIPMENT_NEW_LENGTH				13//25
#define MAX_CHARACTER_LENGTH				10
#define CHAOS_MACHINE_EXTENSION

#define INVENTORY_HOLY_JOH
#define TRADE_SYSTEM_COIN
#define INVENTORY_EXT_BUY
#define INVASION_TIME_MANAGER_KILL
#define EVENT_FIND_PATH_NPC_CLIK
#define CHARACTER_EQUIPEMENT_EXT
#define PET_SYSTEM_GAIN_EXP
#define SHUTDOWN_CORE_WING4
#define PARTY_PASSWORD_JOIN
#define CB_GETMIXRATE							1
#define CB_DANGKYINGAME							1


#define INDEX_SET(index, data) \
	index[0] = SET_NUMBERHB(data); \
	index[1] = SET_NUMBERLB(data);

#define INDEX_GET(index) \
	MAKE_NUMBERW(index[0], index[1]);

#define INDEX_DATA(name) \
	BYTE m_prop##name[2]; \
	WORD Get##name() { return INDEX_GET(this->m_prop##name); } \
	void Set##name(WORD value) { INDEX_SET(this->m_prop##name, value); }