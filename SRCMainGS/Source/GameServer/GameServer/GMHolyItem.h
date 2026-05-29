#pragma once
#include "ItemManager.h"
#include "Protocol.h"
#define MAX_JEWEL_PACKAGE		3

struct SDHP_INVENTORY_HOLY_SEND
{
	PSBMSG_HEAD header; // C1:27:00
	WORD index;
	char account[11];
	char name[11];
};

struct SDHP_INVENTORY_HOLY_RECV
{
	PSWMSG_HEAD header; // C2:27:00
	WORD index;
	char account[11];
	BYTE Inventory[HOLY_INVENTORY_SIZE][12];
};

struct SDHP_INVENTORY_HOLY_SAVE_SEND
{
	PSWMSG_HEAD header; // C2:0x41:0x01
	char account[11];
	char name[11];
	BYTE Inventory[HOLY_INVENTORY_SIZE][12];
};

struct PMSG_INVENTORY_HOLY
{
	BYTE Inventory[12];
};

struct PMSG_INVENTORY_HOLY_SEND
{
	PSWMSG_HEAD header; // C1:BF:51
	BYTE Value;
};

struct PMSG_INVENTORY_HOLY_RECV
{
	PSBMSG_HEAD header; // C1:BF:51
	BYTE btIndex;
	BYTE btType;
	DWORD btValue;
};

typedef struct
{
	int ItemIndex;
	int ItemLevel;
	int BundledIndex;
	int ValueUnit;
	int ValuePack[MAX_JEWEL_PACKAGE];
}template_holy;



class CGMHolyItem
{
public:
	CGMHolyItem();
	virtual~CGMHolyItem();

	static CGMHolyItem* Instance()
	{
		static CGMHolyItem sInstance;
		return &sInstance;
	}

	void LoadInfo(char* filename);
	bool GetInfo(int index, template_holy* lpInfo);



	void DGHolyInventoryReq(int aIndex);
	void DGHolyInventoryRecv(SDHP_INVENTORY_HOLY_RECV* lpMsg);
	void DGHolyInventorySave(int aIndex);
	void GCInventoryListSend(int aIndex);
	void ItemByteConvert(int index, BYTE* lpMsg, HOLY_INVENTORY lpItem);
	void DBItemByteConvert(int index, BYTE* lpMsg, HOLY_INVENTORY* lpItem);

	void GCInventoryRemove(int aIndex, PMSG_INVENTORY_HOLY_RECV* Data);
	void GCInventoryAddItem(int aIndex, PMSG_INVENTORY_HOLY_RECV* Data);
private:
	__int64 bItemCountMax;
	__int64 bItemCountMin;
	std::vector<template_holy> m_bItems;
};

#define GMHolyItem						(CGMHolyItem::Instance())