#pragma once
#include "DataServerProtocol.h"

#define HOLY_INVENTORY_SIZE				21

struct SDHP_INVENTORY_HOLY_RECV
{
	PSBMSG_HEAD header; // C1:27:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_INVENTORY_HOLY_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:31
	char account[11];
	char name[11];
	BYTE Inventory[HOLY_INVENTORY_SIZE][12];
};

struct SDHP_INVENTORY_HOLY_SEND
{
	PSWMSG_HEAD header; // C2:27:00
	WORD index;
	char account[11];
	BYTE Inventory[HOLY_INVENTORY_SIZE][12];
};

class CGMHolyItem
{
public:
	CGMHolyItem() {};
	~CGMHolyItem() {};

	static CGMHolyItem* Instance()
	{
		static CGMHolyItem sInstance;
		return &sInstance;
	}

	void GDHolyInventoryRecv(SDHP_INVENTORY_HOLY_RECV* lpMsg, int index);
	void GDHolyInventorySaveRecv(SDHP_INVENTORY_HOLY_SAVE_RECV* lpMsg);
};

#define GMHolyItem						(CGMHolyItem::Instance())