#pragma once
#include "User.h"

#ifdef CHAOS_MACHINE_EXTENSION
#define MIX_INVENTORY_SIZE			120
#define MAX_MIX_NAME_SIZE			32
#define MIX_INVENTORY_RANGE(x)		(((x)<0)?0:((x)>=MIX_INVENTORY_SIZE)?0:1)

enum _MIX_SOURCE_STATUS { MIX_SOURCE_ERROR, MIX_SOURCE_NO, MIX_SOURCE_PARTIALLY, MIX_SOURCE_YES };

enum _SPECIAL_ITEM_RECIPE_
{
	RCP_SP_EXCELLENT = 1,
	RCP_SP_ADD380ITEM = 2,
	RCP_SP_SETITEM = 4,
	RCP_SP_HARMONY = 8,
	RCP_SP_SOCKETITEM = 16,
};

typedef struct
{
	int mainIndex;
	int minIndex;
	int maxIndex;
	BYTE minLevel;
	BYTE maxLevel;
	BYTE minDurability;
	BYTE maxDurability;
	BYTE skill;
	BYTE luck;
	BYTE minOpc;
	BYTE maxOpc;
	BYTE exeItem;
	BYTE accItem;
	BYTE minCnt;
	BYTE maxCnt;
}MIX_RECIPIENT;

typedef struct
{
	int m_iMixIndex;
	std::string m_iName;
	BYTE m_iSuccessRate[MAX_ACCOUNT_LEVEL];
	BYTE m_iRequiredType;
	DWORD m_RequiredZen;
	int m_iMainItem;
	//--
	int TalismanOfChaos;
	int TalismanOfLuck;
	int TalismanLuckRate;
	int TalismanCnt;

	std::vector<MIX_RECIPIENT> mixRecipient;
	std::vector<MIX_RECIPIENT> mixSuccessfully;
}MIX_TEMPLATE;

struct RECIPIENT_CHECK
{
	int SpecialItem;
	int iNumMixItem;

	RECIPIENT_CHECK()
		:SpecialItem(0), iNumMixItem(0)
	{
	}
};


typedef struct
{
	PSWMSG_HEAD  Header;
	BYTE         Value;
} PHEADER_MIX_DEFAULT, * LPPHEADER_MIX_DEFAULT;

//**********************************************//
//************ GameServer -> DataServer ************//
//**********************************************//

struct SDHP_REQUEST_MIX_GOBBLIN_ITEM
{
	PSBMSG_HEAD header; // C1:05:00
	WORD        index;
	char        Account[11];
	char        Name[11];
};

struct SDHP_MIX_GOBBLIN_ITEM_DEFAULT
{
	PSWMSG_HEAD header; // C2:27:30
	WORD        index;
	char        iName[11];
	char        Account[11];
	BYTE        m_iMixIndex;
	BYTE        m_iMixState;
	BYTE        Inventory[MIX_INVENTORY_SIZE][16];
};
//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

typedef struct
{
	PSWMSG_HEAD Header;
	BYTE        Value;
	BYTE        iSuccessRate;
	BYTE        iRequiredType;
	WORD        iTalismanOfLuck;
	WORD        iTalismanOfChaos;
	WORD        iTalismanLuckRate;
	DWORD       iRequiredZen;
} PHEADER_MIX_INFORMATION, * LPPHEADER_MIX_INFORMATION;

struct PMSG_MIX_ITEM_LIST
{
	WORD iMixIndex;
	char iName[MAX_MIX_NAME_SIZE];
};

struct PMSG_DEFAULT_ITEM_LIST
{
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};
//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//




typedef std::map<int, MIX_TEMPLATE> type_map_mixgoblin;

class CMixGoblinExpansion
{
public:
	CMixGoblinExpansion();
	virtual~CMixGoblinExpansion();
	void Release();

	void OpenFile(std::string filename);

	bool IsGoblinMix(int mainKey);
	MIX_TEMPLATE* FindTemplate(int mixIndex);
	void push_back(int index, int Type,  MIX_RECIPIENT Info);
	//--
	void GDMixInventoryReq(LPOBJ lpObj);
	void GDMixInventorySave(LPOBJ lpObj);
	void GDMixInventoryRecv(SDHP_MIX_GOBBLIN_ITEM_DEFAULT* lpMsg);

	//--
	void winMixInfo(LPOBJ lpObj);
	void InventoryMixList(LPOBJ lpObj);
	void MixRecipientIndex(LPOBJ lpObj, BYTE mixIndex);
	void MixRecipientInfo(LPOBJ lpObj, MIX_TEMPLATE* Info);
	void ReceiveMixInfo(int aIndex, BYTE* ReceiveBuffer);
	bool TalkNpc(LPOBJ lpObj, int npc, int map, int Tx, int Ty);

	void Clear(LPOBJ lpObj);
	bool CheckRecipe(CItem* pItem, MIX_RECIPIENT* pMixItems, RECIPIENT_CHECK* Check);
	void GenerateMix(LPOBJ lpObj, MIX_TEMPLATE* info);
	void CreateMix(int aIndex, BYTE* ReceiveBuffer);
private:
	int Npc;
	int Map;
	int PosX;
	int PosY;
	int max_mix;
	type_map_mixgoblin mixgoblin;
};

extern CMixGoblinExpansion gpMixGoblinExpansion;

#define GoblinMixExpansion			(&gpMixGoblinExpansion)

#endif // CHAOS_MACHINE_EXTENSION
