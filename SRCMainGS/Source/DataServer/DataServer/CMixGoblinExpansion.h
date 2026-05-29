#pragma once
#include "DataServerProtocol.h"

#ifdef CHAOS_MACHINE_EXTENSION

#define MIX_INVENTORY_SIZE			120

struct SDHP_MIX_GOBLIN_ITEM
{
	PSBMSG_HEAD header; // C1:05:00
	WORD index;
	char Account[11];
	char Name[11];
};

struct SDHP_MIX_GOBLIN_ITEM_SEND
{
	PSWMSG_HEAD header; // C2:27:30
	WORD index;
	char iName[11];
	char Account[11];
	BYTE m_iMixIndex;
	BYTE m_iMixState;
	BYTE Inventory[MIX_INVENTORY_SIZE][16];
};

struct SDHP_MIX_GOBBLIN_ITEM_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:27:30
	WORD index;
	char iName[11];
	char Account[11];
	BYTE m_iMixIndex;
	BYTE m_iMixState;
	BYTE Inventory[MIX_INVENTORY_SIZE][16];
};


class CMixGoblinExpansion
{
public:
	CMixGoblinExpansion();
	~CMixGoblinExpansion();

	void GDInventoryRecv(SDHP_MIX_GOBLIN_ITEM* lpMsg, int index);
	void GDInventorySaveRecv(SDHP_MIX_GOBBLIN_ITEM_SAVE_RECV* lpMsg);
};

extern CMixGoblinExpansion gGoblinExpansion;

#endif // CHAOS_MACHINE_EXTENSION
