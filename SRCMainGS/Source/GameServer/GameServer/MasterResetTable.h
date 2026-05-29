#pragma once
#include "User.h"
#include "Protocol.h"

#define MAX_ITEM_JEWEL_KOF			10

struct MASTER_RESET_TABLE_INFO
{
	int Index;
	int MinReset;
	int MaxReset;
	int Reset[MAX_ACCOUNT_LEVEL];
	int Level[MAX_ACCOUNT_LEVEL];
	int Money[MAX_ACCOUNT_LEVEL];
	int ItemIndex[MAX_ITEM_JEWEL_KOF];
	int ItemCount[MAX_ITEM_JEWEL_KOF];
};

struct MASTER_RESET_TABLE_REWARD_INFO
{
	int Index;
	int Point[MAX_ACCOUNT_LEVEL];
	int PackageBag[MAX_ACCOUNT_LEVEL];
};


struct PMSG_MASTER_RESET_TABLE_INFO
{
	PSWMSG_HEAD Head;
	int MasterReset;
	int MaxMasterReset;
	int MinLevel;
	int MinReset;
	DWORD ReqMoney;
	int RewardItem;
	int RewardPoint;
	int ItemIndex[MAX_ITEM_JEWEL_KOF];
	int ItemCount[MAX_ITEM_JEWEL_KOF];
};

class CMasterResetTable
{
public:
	CMasterResetTable();
	virtual~CMasterResetTable();

	void Init();
	void Load(char* path);
	int GetMasterMaxReset(int resetCount);
	int GetMasterResetCount(int resetCount, int accountLevel);
	int GetMasterResetLevel(int resetCount, int accountLevel);
	int GetMasterResetMoney(int resetCount, int accountLevel);
	int GetMasterResetJewel(int resetCount, int accountLevel, int* _ItemIndex, int* _ItemCount);
	int GetMasterResetJewel(LPOBJ lpObj, int resetCount);
	int CheckMasterResetJewel(LPOBJ lpObj, int resetCount, bool NoticeSend);
	int GetMasterResetPoint(int resetCount, int accountLevel);
	int GetMasterResetPackage(LPOBJ lpObj, int resetCount, int accountLevel);

	int GetMasterLevelPoint(LPOBJ lpObj, int _resetCount, int accountLevel);
	void send_player_info(LPOBJ lpObj, int resetCount, int accountLevel);

	MASTER_RESET_TABLE_REWARD_INFO* GetTableInfo(int index);
	std::vector<MASTER_RESET_TABLE_INFO>::iterator FindResetRange(int resetCount);
private:
	std::vector<MASTER_RESET_TABLE_INFO> m_ResetTableInfo;
	std::map<int, MASTER_RESET_TABLE_REWARD_INFO> m_ResetTableRewardInfo;
};

extern CMasterResetTable gMasterResetTable;
