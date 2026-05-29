// ResetTable.h: interface for the CResetTable class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"
#include "Protocol.h"

#define MAX_ITEM_JEWEL_KOF			10

struct RESET_TABLE_INFO
{
	int Index;
	int MinReset;
	int MaxReset;
	int Level[MAX_ACCOUNT_LEVEL];
	int Money[MAX_ACCOUNT_LEVEL];
	int ItemIndex[MAX_ITEM_JEWEL_KOF];
	int ItemCount[MAX_ITEM_JEWEL_KOF];
};

struct RESET_TABLE_REWARD_INFO
{
	int Index;
	int Point[MAX_ACCOUNT_LEVEL];
	int PackageBag[MAX_ACCOUNT_LEVEL];
};

struct PMSG_RESET_TABLE_STATE
{
	PSWMSG_HEAD Head;
	int MaxReset;
};

struct PMSG_RESET_TABLE_INFO
{
	PSWMSG_HEAD Head;
	int Reset;
	int MaxReset;
	int MinLevel;
	DWORD ReqMoney;
	int RewardItem;
	int RewardPoint;
	int ItemIndex[MAX_ITEM_JEWEL_KOF];
	int ItemCount[MAX_ITEM_JEWEL_KOF];
};

class CResetTable
{
public:
	CResetTable();
	virtual ~CResetTable();
	void Init();
	void Load(char* path);
	int GetResetMax(int resetCount);
	int GetResetLevel(int resetCount, int accountLevel);
	int GetResetMoney(int resetCount, int accountLevel);
	int GetResetJewel(int resetCount, int accountLevel, int* _ItemIndex, int* _ItemCount);
	int GetResetJewel(LPOBJ lpObj, int resetCount);
	int CheckResetJewel(LPOBJ lpObj, int resetCount, bool NoticeSend);
	int GetResetPoint(int resetCount, int accountLevel);
	int GetResetPackage(LPOBJ lpObj, int resetCount, int accountLevel);

	void send_player_info(LPOBJ lpObj, int resetCount, int accountLevel);

	RESET_TABLE_REWARD_INFO* GetTableInfo(int index);
	std::vector<RESET_TABLE_INFO>::iterator FindResetRange(int resetCount);
private:
	std::vector<RESET_TABLE_INFO> m_ResetTableInfo;
	std::map<int, RESET_TABLE_REWARD_INFO> m_ResetTableRewardInfo;
};

extern CResetTable gResetTable;
