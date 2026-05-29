// ItemManager.h: interface for the CItemManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "Protocol.h"
#include "User.h"


#define MAX_ITEM_SECTION 16
#define MAX_ITEM_TYPE 512
#define MAX_ITEM (MAX_ITEM_SECTION*MAX_ITEM_TYPE)
#define MAX_ITEM_INFO 12

#define REQUEST_EQUIPMENT_INVENTORY								0
#define REQUEST_EQUIPMENT_TRADE									1
#define REQUEST_EQUIPMENT_STORAGE								2
#define REQUEST_EQUIPMENT_CHAOS_MIX								3
#define REQUEST_EQUIPMENT_MYSHOP								4
#define REQUEST_EQUIPMENT_TRAINER_MIX							5
#define REQUEST_EQUIPMENT_ELPIS_MIX								6
#define REQUEST_EQUIPMENT_OSBOURNE_MIX							7
#define REQUEST_EQUIPMENT_JERRIDON_MIX							8
#define REQUEST_EQUIPMENT_CHAOS_CARD_MIX						9
#define REQUEST_EQUIPMENT_CHERRYBLOSSOM_MIX						10
#define REQUEST_EQUIPMENT_EXTRACT_SEED_MIX						11
#define REQUEST_EQUIPMENT_SEED_SPHERE_MIX						12
#define REQUEST_EQUIPMENT_ATTACH_SOCKET_MIX						13
#define REQUEST_EQUIPMENT_DETACH_SOCKET_MIX						14


#define GET_ITEM(x,y)					(((x)*MAX_ITEM_TYPE)+(y))
#define CHECK_ITEM(x)					(((x)<0)?-1:((x)>=MAX_ITEM)?-1:x)
#define INVENTORY_RANGE(x)				(((x)<0)?0:((x)>=INVENTORY_SIZE)?0:1)
#ifdef CHARACTER_EQUIPEMENT_EXT
#define INVENTORY_WEAR_RANGE(x)			(((x)<0)?0:((x)>=INVENTORY_WEAR_SIZE)?(((x)>=EQUIPMENT_MUUN && (x)<=EQUIPMENT_FLAG_NAT)?1:0):1)
#define INVENTORY_FULL_RANGE(x)			(((x)<0)?0:((x)>=INVENTORY_EXT4_SIZE)?(((x)>=EQUIPMENT_MUUN && (x)<=EQUIPMENT_FLAG_NAT)?1:0):1)
#else
#if(GAMESERVER_UPDATE>=701)
#define INVENTORY_WEAR_RANGE(x) (((x)<0)?0:((x)>=INVENTORY_WEAR_SIZE)?(((x)==236)?1:0):1)
#define INVENTORY_FULL_RANGE(x) (((x)<0)?0:((x)>=INVENTORY_EXT4_SIZE)?(((x)==236)?1:0):1)
#else
#define INVENTORY_WEAR_RANGE(x) (((x)<0)?0:((x)>=INVENTORY_WEAR_SIZE)?0:1)
#define INVENTORY_FULL_RANGE(x) (((x)<0)?0:((x)>=INVENTORY_EXT4_SIZE)?0:1)
#endif
#endif // CHARACTER_EQUIPEMENT_EXT
#define INVENTORY_BASE_RANGE(x)			(((x)<INVENTORY_WEAR_SIZE)?0:((x)>=INVENTORY_SIZE)?0:1)
#define INVENTORY_MAIN_RANGE(x)			(((x)<INVENTORY_WEAR_SIZE)?0:((x)>=INVENTORY_MAIN_SIZE)?0:1)
#define INVENTORY_EXT1_RANGE(x)			(((x)<INVENTORY_MAIN_SIZE)?0:((x)>=INVENTORY_EXT1_SIZE)?0:1)
#define INVENTORY_EXT2_RANGE(x)			(((x)<INVENTORY_EXT1_SIZE)?0:((x)>=INVENTORY_EXT2_SIZE)?0:1)
#define INVENTORY_EXT3_RANGE(x)			(((x)<INVENTORY_EXT2_SIZE)?0:((x)>=INVENTORY_EXT3_SIZE)?0:1)
#define INVENTORY_EXT4_RANGE(x)			(((x)<INVENTORY_EXT3_SIZE)?0:((x)>=INVENTORY_EXT4_SIZE)?0:1)
#define INVENTORY_SHOP_RANGE(x)			(((x)<INVENTORY_EXT4_SIZE)?0:((x)>=INVENTORY_FULL_SIZE)?0:1)
#define TRADE_RANGE(x)					(((x)<0)?0:((x)>=TRADE_SIZE)?0:1)
#define WAREHOUSE_RANGE(x)				(((x)<0)?0:((x)>=WAREHOUSE_SIZE)?0:1)
#define WAREHOUSE_MAIN_RANGE(x)			(((x)<0)?0:((x)>=WAREHOUSE_EXT1_SIZE)?0:1)
#define WAREHOUSE_EXT1_RANGE(x)			(((x)<WAREHOUSE_EXT1_SIZE)?0:((x)>=WAREHOUSE_SIZE)?0:1)
#define CHAOS_BOX_RANGE(x)				(((x)<0)?0:((x)>=CHAOS_BOX_SIZE)?0:1)



#define NAME_CLASS_ARRAY {"DarkWizard", "DarkKnight", "FairyElf", "MagicGladiator", "DarkLord", "Summoner", "RageFighter", "GrowLancer", "RuneWizard", "Slayer", "GunCrusher", "Kundun", "LemuriaMage", "IllusionKnight", "Alquimista"}
#define NAME_CLASS_ARRAY2 {"DW", "DK", "FE", "MG", "DL", "SM", "RG", "GL", "RW", "SL", "GC", "KN", "LM", "IK", "AQ"}

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_ITEM_GET_RECV
{
	PBMSG_HEAD header; // C1:22
	BYTE index[2];
};

struct PMSG_ITEM_DROP_RECV
{
	PBMSG_HEAD header; // C1:23
	BYTE x;
	BYTE y;
	BYTE slot;
};

struct PMSG_ITEM_MOVE_RECV
{
	PBMSG_HEAD header; // C1:24
	BYTE SourceFlag;
	BYTE SourceSlot;
	BYTE ItemInfo[MAX_ITEM_INFO];
	BYTE TargetFlag;
	BYTE TargetSlot;
};

struct PMSG_ITEM_USE_RECV
{
	PBMSG_HEAD header; // C1:26
	BYTE SourceSlot;
	BYTE TargetSlot;
	BYTE type;
};

struct PMSG_ITEM_BUY_RECV
{
	PBMSG_HEAD header; // C1:32
	BYTE slot;
};

struct PMSG_ITEM_SELL_RECV
{
	PBMSG_HEAD header; // C1:33
	BYTE slot;
};

struct PMSG_ITEM_REPAIR_RECV
{
	PBMSG_HEAD header; // C1:34
	BYTE slot;
	BYTE type;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//
#ifdef INVENTORY_EXT_BUY
struct PMSG_CREATE_INVENTARIO_SEND
{
	PSBMSG_HEAD header; // C3:22
	BYTE ExtInventory;
	DWORD InventoryExtPrice;
};
#endif // INVENTORY_EXT_BUY

struct PMSG_ITEM_GET_SEND
{
	PBMSG_HEAD header; // C3:22
	BYTE result;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_ITEM_DROP_SEND
{
	PBMSG_HEAD header; // C1:23
	BYTE result;
	BYTE slot;
};

struct PMSG_ITEM_MOVE_SEND
{
	PBMSG_HEAD header; // C3:24
	BYTE SubCode;
	BYTE Index;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_ITEM_CHANGE_SEND
{
	PBMSG_HEAD header; // C1:25
	BYTE index[2];
	BYTE ItemInfo[MAX_ITEM_INFO];
	BYTE slot;
#if(GAMESERVER_UPDATE>=701)
	BYTE attribute;
#endif
};

struct PMSG_ITEM_DELETE_SEND
{
	PBMSG_HEAD header; // C1:28
	BYTE slot;
	BYTE flag;
};

struct PMSG_ITEM_DUR_SEND
{
	PBMSG_HEAD header; // C1:2A
	BYTE slot;
	BYTE dur;
	BYTE flag;
};

struct PMSG_ITEM_BUY_SEND
{
	PBMSG_HEAD header; // C1:32
	BYTE result;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_ITEM_SELL_SEND
{
	PBMSG_HEAD header; // C1:33
	BYTE result;
	DWORD money;
};

struct PMSG_ITEM_REPAIR_SEND
{
	PBMSG_HEAD header; // C1:34
	DWORD money;
};

struct PMSG_ITEM_LIST_SEND
{
	PSWMSG_HEAD header; // C4:F3:10
	BYTE count;
};

struct PMSG_ITEM_LIST
{
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

struct PMSG_ITEM_EQUIPMENT_SEND
{
	PSBMSG_HEAD header; // C1:F3:13
	BYTE index[2];
	DWORD Equipment[EQUIPMENT_NEW_LENGTH];
};

struct PMSG_ITEM_MODIFY_SEND
{
	PSBMSG_HEAD header; // C1:F3:14
	BYTE slot;
	BYTE ItemInfo[MAX_ITEM_INFO];
};

typedef struct
{
	PSBMSG_HEAD  header;
	BYTE          ID[10];
	BYTE          Item[MAX_ITEM_INFO];
} PHEADER_DEFAULT_SUBCODE_CHAT_ITEM, * LPPHEADER_DEFAULT_SUBCODE_CHAT_ITEM;

struct PMSG_ITEM_BUY_NEW
{
	PSBMSG_HEAD header; // C1:32
	BYTE slot;
};

//**********************************************//
//**********************************************//
//**********************************************//
enum cStatType
{
	STRENGTH,
	AGILITY,
	VITALITY,
	ENERGY,
	LEADERSHIP,
	MAX_STAT_TYPE
};

struct ITEM_INFO
{
	int Index;
	//char Path[MAX_PATH];
	char Model[MAX_PATH];
	std::string Name;
	BYTE Kind1;
	BYTE Kind2;
	BYTE Kind3;
	bool TwoHand;
	int Level;
	int Slot;
	int m_wSkillIndex;
	BYTE Width;
	BYTE Height;
	BYTE DropItem;
	DWORD DamageMin;
	DWORD DamageMax;
	int MagicDamageRate;
	int Defense;
	int MagicDefense;
	int DefenseSuccessRate;
	BYTE AttackSpeed;
	BYTE WalkSpeed;
	int Durability;
	int MagicDurability;
	int Value;
	int BuyMoney;
	BYTE Resistance[MAX_RESISTANCE_TYPE];
	DWORD RequireLevel;
	DWORD RequireStrength;
	DWORD RequireDexterity;
	DWORD RequireEnergy;
	DWORD RequireVitality;
	DWORD RequireLeadership;
	BYTE RequireClass[MAX_CLASS];
	//--
	int SkillIndex;
	//--
	//bool DropItem;
	//bool Trade;
	//bool StorePersonal;
	//bool WhareHouse;
	//bool SellNpc;
	//bool Expensive;
	//bool Repair;
	//WORD Overlap;
	//WORD PcFlag;
	//WORD MuunFlag;
	//bool Dropinventory;
	//BYTE arg_683;
	//BYTE arg_684;
	DWORD PowerATTK;
	//WORD arg_688;
	//WORD arg_690;
	const char* GetName() {
		return this->Name.data();
	}
};

class Script_Item //-- item bmd
{
public:
/*+000*/ int Type;
/*+004*/ short Index;
/*+006*/ short SubIndex;
///*+008*/ char Path[MAX_PATH];
///*+268*/ char Model[MAX_PATH];
/*+528*/ char Name[64];
/*+592*/ BYTE Kind1;
/*+593*/ BYTE Kind2;
/*+594*/ BYTE Kind3;
/*+595*/ bool TwoHand;
/*+596*/ WORD Level;
/*+598*/ BYTE m_byItemSlot;
/*+600*/ WORD m_wSkillIndex;
/*+602*/ BYTE Width;
/*+603*/ BYTE Height;
/*+604*/ WORD DamageMin;
/*+606*/ WORD DamageMax;
/*+608*/ BYTE SuccessfulBlocking;
/*+610*/ WORD Defense;
/*+612*/ WORD MagicDefense;
/*+614*/ BYTE WeaponSpeed;
/*+615*/ BYTE WalkSpeed;
/*+616*/ BYTE Durability;
/*+617*/ BYTE MagicDurability;
/*+620*/ DWORD MagicPower;
/*+624*/ WORD RequireStrength;
/*+626*/ WORD RequireDexterity;
/*+628*/ WORD RequireEnergy;
/*+630*/ WORD RequireVitality;
/*+632*/ WORD RequireCharisma;
/*+634*/ WORD RequireLevel;
/*+636*/ BYTE Value;
/*+640*/ int iZen;
/*+644*/ BYTE AttType;
/*+645*/ BYTE RequireClass[MAX_CLASS];
/*+659*/ BYTE Resistance[MAX_RESISTANCE_TYPE];
/*+667*/ bool Dropinventory;
/*+668*/ bool Trade;
/*+669*/ bool StorePersonal;
/*+670*/ bool WhareHouse;
/*+671*/ bool SellNpc;
/*+672*/ bool Expensive;
/*+673*/ bool Repair;
/*+676*/ WORD Overlap;
/*+678*/ WORD PcFlag;
/*+680*/ WORD MuunFlag;
/*+686*/ DWORD PowerATTK;
};

typedef std::map<int, ITEM_INFO> type_map_item;

class CItemManager
{
public:
	CItemManager();
	virtual ~CItemManager();
	void Load(char* filename);
	//void ExportConfigPet();
#ifndef GAMESERVER_EDITH_EXPORT
	void ExportTXT(char* filename);
#endif // GAMESERVER_CLIENTE_UPDATE >= 16
	void ExportBMD(char* filename);
	void ExportModel(char* filename);

	bool GetInfo(int index, ITEM_INFO* lpInfo);
	int GetItemSkill(int index);
	int CheckItemSkill(int index);
	int GetItemTwoHand(int index);
	int AdquireSkill(int Itemindex);
	bool IsInventoryItem(int Itemindex);
	int GetItemDurability(int index, int level, int NewOption, int SetOption);
	int GetItemRepairMoney(CItem* lpItem, int type);
	int GetInventoryItemSlot(LPOBJ lpObj, int index, int level);
	int GetInventoryItemCount(LPOBJ lpObj, int index, int level);
	int GetInventoryEmptySlotCount(LPOBJ lpObj);
	int GetInventoryMaxValue(LPOBJ lpObj);
	bool CheckItemRequireLevel(LPOBJ lpObj, CItem* lpItem);
	bool CheckItemRequireStrength(LPOBJ lpObj, CItem* lpItem);
	bool CheckItemRequireDexterity(LPOBJ lpObj, CItem* lpItem);
	bool CheckItemRequireVitality(LPOBJ lpObj, CItem* lpItem);
	bool CheckItemRequireEnergy(LPOBJ lpObj, CItem* lpItem);
	bool CheckItemRequireLeadership(LPOBJ lpObj, CItem* lpItem);
	bool CheckItemRequireClass(LPOBJ lpObj, int index);
	bool CheckItemMoveToInventory(LPOBJ lpObj, CItem* lpItem, int slot);
	bool CheckItemMoveToTrade(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag);
	bool CheckItemMoveToVault(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag);
	bool CheckItemMoveToChaos(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag);
#ifdef CHAOS_MACHINE_EXTENSION
	bool CheckItemMoveToMixExpansion(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag);
#endif // CHAOS_MACHINE_EXTENSION

	bool CheckItemMoveToBlock(LPOBJ lpObj, CItem* lpItem);
	bool CheckItemInventorySpace(LPOBJ lpObj, int index);
	bool CheckItemInventorySpace(LPOBJ lpObj, int index, int count, bool copy, bool copy2);
	bool CheckItemInventorySpace(BYTE* InventoryMap, int MaxY, int width, int height);
	bool CheckItemInventorySpace(LPOBJ lpObj, int width, int height);
	//-----------
	void InventoryItemSet(int aIndex, int slot, BYTE type);
	void InventoryItemSet(BYTE* InventoryMap, int slot, int width, int height, BYTE type);
	BYTE InventoryRectCheck(int aIndex, int x, int y, int width, int height);
	BYTE InventoryRectCheck(BYTE* InventoryMap, int x, int y, int width, int height);
	BYTE InventoryInsertItem(int aIndex, CItem item);
	BYTE InventoryAddItem(int aIndex, CItem item, int slot);
	void InventoryDelItem(int aIndex, int slot);
	bool InventoryBreakItemStack(LPOBJ lpObj, CItem* lpItem, int slot);
	bool InventoryInsertItemStack(LPOBJ lpObj, CItem* lpItem);
	bool InventoryAddItemStack(LPOBJ lpObj, int SourceSlot, int TargetSlot);
	//-----------
	void TradeItemSet(int aIndex, int slot, BYTE type);
	BYTE TradeRectCheck(int aIndex, int x, int y, int width, int height);
	BYTE TradeInsertItem(int aIndex, CItem item);
	BYTE TradeAddItem(int aIndex, CItem item, int slot);
	void TradeDelItem(int aIndex, int slot);
	//-----------
	void WarehouseItemSet(int aIndex, int slot, BYTE type);
	BYTE WarehouseRectCheck(int aIndex, int x, int y, int width, int height);
	BYTE WarehouseInsertItem(int aIndex, CItem item);
	BYTE WarehouseAddItem(int aIndex, CItem item, int slot);
	void WarehouseDelItem(int aIndex, int slot);
	//-----------
	void ChaosBoxItemSet(int aIndex, int slot, BYTE type);
	BYTE ChaosBoxRectCheck(int aIndex, int x, int y, int width, int height);
	BYTE ChaosBoxInsertItem(int aIndex, CItem item);
	BYTE ChaosBoxAddItem(int aIndex, CItem item, int slot);
	void ChaosBoxDelItem(int aIndex, int slot);
	//-----------
#ifdef CHAOS_MACHINE_EXTENSION
	void MixExpansionItemSet(int aIndex, int slot, BYTE type);
	BYTE MixExpansionRectCheck(int aIndex, int x, int y, int width, int height);
	BYTE MixExpansionInsertItem(int aIndex, CItem item);
	BYTE MixExpansionAddItem(int aIndex, CItem item, int slot);
	void MixExpansionDelItem(int aIndex, int slot);
#endif // CHAOS_MACHINE_EXTENSION
	//-----------
	void ItemByteConvert(BYTE* lpMsg, CItem item);
	void DBItemByteConvert(BYTE* lpMsg, CItem* lpItem);
	bool ConvertItemByte(CItem* lpItem, BYTE* lpMsg);
	bool IsValidItem(LPOBJ lpObj, CItem* lpItem);
	void UpdateInventoryViewport(int aIndex, int slot);
	void DeleteInventoryItemCount(LPOBJ lpObj, int index, int level, int count);
	void DecreaseItemDur(LPOBJ lpObj, int slot, int dur);
	int RepairItem(LPOBJ lpObj, CItem* lpItem, int slot, int type);
	BYTE MoveItemToInventoryFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToInventoryFromTrade(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToInventoryFromWarehouse(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToInventoryFromChaosBox(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToInventoryFromPersonalShop(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToTradeFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToTradeFromTrade(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToTradeFromEventInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToWarehouseFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToWarehouseFromWarehouse(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToChaosBoxFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToChaosBoxFromChaosBox(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToPersonalShopFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToPersonalShopFromPersonalShop(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToEventInventoryFromTrade(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToEventInventoryFromEventInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToMuunInventoryFromMuunInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);

#ifdef CHAOS_MACHINE_EXTENSION
	BYTE MoveItemToMixExpansionFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
	BYTE MoveItemToInventoryFromMixExpansion(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag);
#endif // CHAOS_MACHINE_EXTENSION

	bool runtime_action_item(LPOBJ lpObj, CItem* lpItem, BYTE iSourceIndex, BYTE iTargetIndex);

#ifdef INVENTORY_EXT_BUY
	void CGInventoryBuyRecv(PMSG_ITEM_BUY_RECV* lpMsg, int aIndex);
#endif // INVENTORY_EXT_BUY

	void CGItemGetRecv(PMSG_ITEM_GET_RECV* lpMsg, int aIndex);
	void CGItemDropRecv(PMSG_ITEM_DROP_RECV* lpMsg, int aIndex);
	bool CGPkDrop(PMSG_ITEM_DROP_RECV* lpMsg, int aIndex);
	void CGItemMoveRecv(PMSG_ITEM_MOVE_RECV* lpMsg, int aIndex);
	void CGItemUseRecv(PMSG_ITEM_USE_RECV* lpMsg, int aIndex);
	void CGItemBuyRecv(PMSG_ITEM_BUY_RECV* lpMsg, int aIndex, int ok);
	void CGItemSellRecv(PMSG_ITEM_SELL_RECV* lpMsg, int aIndex);
	void CGItemBreakRecv(PMSG_ITEM_REPAIR_RECV* lpMsg, int aIndex);
	void CGItemRepairRecv(PMSG_ITEM_REPAIR_RECV* lpMsg, int aIndex);
	void GCItemMoveSend(int aIndex, BYTE result, BYTE slot, BYTE* ItemInfo);
	void GCItemChangeSend(int aIndex, BYTE slot);
	void GCItemDeleteSend(int aIndex, BYTE slot, BYTE flag);
	void GCItemDurSend(int aIndex, BYTE slot, BYTE dur, BYTE flag);
	void GCItemListSend(int aIndex, bool SendTime = false);
	void GCItemEquipmentSend(int aIndex);
	void GCItemModifySend(int aIndex, BYTE slot);
	const char* GetItemName(int index);
	void CGItemBuyConfirmRecv(PMSG_ITEM_BUY_NEW* lpMsg, int aIndex);
	void CGItemPostChatRecv(PMSG_ITEM_BUY_RECV* lpMsg, int aIndex);

private:
	type_map_item m_ItemInfo;
};

extern CItemManager gItemManager;
