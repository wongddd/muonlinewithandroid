// Item.h: interface for the CItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#define HOLY_INVENTORY_SIZE 21

#define INVENTORY_WEAR_SIZE 12
#define INVENTORY_MAIN_SIZE 76
#if(GAMESERVER_UPDATE>=602)
#define INVENTORY_EXT1_SIZE 108
#define INVENTORY_EXT2_SIZE 140
#define INVENTORY_EXT3_SIZE 172
#define INVENTORY_EXT4_SIZE 204
#define INVENTORY_FULL_SIZE 236
#else
#define INVENTORY_EXT1_SIZE 76
#define INVENTORY_EXT2_SIZE 76
#define INVENTORY_EXT3_SIZE 76
#define INVENTORY_EXT4_SIZE 76
#define INVENTORY_FULL_SIZE 108
#endif
#if(GAMESERVER_UPDATE>=701)
#define INVENTORY_SIZE 237
#elif(GAMESERVER_UPDATE>=602)

#ifdef CHARACTER_EQUIPEMENT_EXT
#define INVENTORY_SIZE 254
#else
#define INVENTORY_SIZE 236
#endif // CHARACTER_EQUIPEMENT_EXT


#else
#define INVENTORY_SIZE 108
#endif
#define TRADE_SIZE 32
#if(GAMESERVER_UPDATE>=602)
#define WAREHOUSE_EXT1_SIZE 120
#define WAREHOUSE_SIZE 240
#else
#define WAREHOUSE_EXT1_SIZE 120
#define WAREHOUSE_SIZE 120
#endif
#define CHAOS_BOX_SIZE 32
#define MAX_EXC_OPTION 6
#define MAX_SOCKET_OPTION 5
#define MAX_SPECIAL 14
#define MAX_RESISTANCE_TYPE 7
#define MAX_PET_LEVEL 50
#define MAX_ITEM_PRICE 3000000000
#define MAX_TYPE_PLAYER_BOTS 7 //mc

#ifdef CHARACTER_EQUIPEMENT_EXT
enum
{
	EQUIPMENT_WEAPON_R = 0,
	EQUIPMENT_WEAPON_L = 1,
	EQUIPMENT_HELM = 2,
	EQUIPMENT_ARMOR = 3,
	EQUIPMENT_PANTS = 4,
	EQUIPMENT_GLOVES = 5,
	EQUIPMENT_BOOTS = 6,
	EQUIPMENT_WING = 7,
	EQUIPMENT_HELPER = 8,
	EQUIPMENT_NECKLACE = 9,
	EQUIPMENT_RING_R = 10,
	EQUIPMENT_RING_L = 11,
	EQUIPMENT_MUUN = 237,
	EQUIPMENT_EAGLE = 238,
	EQUIPMENT_ERING_R = 239,
	EQUIPMENT_ERING_L = 240,
	EQUIPMENT_COSTUME = 241, //Non
	EQUIPMENT_FLAG_NAT = 242,
	//--
	//EQUIPMENT_VIEW_WEAPON_R = 241,
	//EQUIPMENT_VIEW_WEAPON_L = 242,
	//EQUIPMENT_VIEW_HELM = 243,
	//EQUIPMENT_VIEW_ARMOR = 244,
	//EQUIPMENT_VIEW_PANTS = 245,
	//EQUIPMENT_VIEW_GLOVES = 246,
	//EQUIPMENT_VIEW_BOOTS = 247,
	//EQUIPMENT_VIEW_WING = 248,
	MAX_EQUIPMENT_INDEX = 16,
};
#endif // CHARACTER_EQUIPEMENT_EXT

namespace ItemKind1
{
	enum
	{
		WEAPON = 1,
		PENDANT = 2,
		ARMOR = 3,
		RING = 4,
		WINGS = 6,
		JEWEL = 9,
		SKILL = 10,
		EVENT_INVENTORY = 11,
		MUUN_INVENTORY = 12,
		MASTERY_WEAPON = 14,
		MASTERY_ARMOR_1 = 15,
		MASTERY_ARMOR_2 = 18,

		EARRING = 19,
		GUARDIAN_MOUNT = 20,
		SOUL_CONVERTER = 21,
		SOUL_CONVERTER1 = 21,
		SOUL_CONVERTER2 = 22,
		SOUL_CONVERTER3 = 23,
		SOUL_CONVERTER4 = 24,
		PENTAGRAM_OPTION = 25,
		ERRTEL_OPTION = 26,
		MASTERY_SHILED = 27,
	};
};

namespace ItemKind2
{
	enum
	{
		NONE = 0,
		SWORD = 1,
		MAGIC_SWORD = 2,
		FIST = 3,
		AXE = 4,
		MACE = 5,
		SCEPTER = 6,
		LANCE = 7,
		BOW = 8,
		CROSSBOW = 9,
		ARROW = 10,
		BOLT = 11,
		STAFF = 12,
		STICK = 13,
		BOOK = 14,
		SHIELD = 15,
		HELM = 16,
		ARMOR = 17,
		PANTS = 18,
		GLOVES = 19,
		BOOTS = 20,
		MINI_WINGS = 22,
		WINGS_LVL_1 = 23,
		//--
		WINGS_LVL_2 = 24,
		WINGS_LVL_3 = 25,
		LORD_CAPE = 26,
		CLOAK_LVL_2 = 27,
		MONSTER_WINGS = 28,
		WINGS_CONQUERORS = 60,
		WINGS_ANGEL_AND_DEVIL = 62,
		WINGS_LVL_4 = 76,
		WINGS_POWER = 80,
		//--
		PHYSICAL_PENDANT = 29,
		WIZARD_PENDANT = 30,
		RING = 31,
		TRANSFORMATION_RING_1 = 32,
		TRANSFORMATION_RING_2 = 33,
		SHOP_PENDANT = 34,
		SHOP_RING = 35,
		SKILL_SCROLL_1 = 36,
		SKILL_PARCHMENT = 37,
		SKILL_ORB = 38,
		SKILL_SCROLL_2 = 39,
		SEED = 40,
		SPHERE = 41,
		SEED_SPHERE = 42,
		PENTAGRAM = 43,
		ERRTEL = 44,
		HELPER = 45,
		HELPER_MOUNT = 46,
		INVENTORY_ITEM = 47,
		BUFF_SCROLL = 48,
		BUFF_ELIXIR = 49,
		BUFF_POTION = 50,
		POTION = 51,
		EVENT_TICKET = 53,
		LUCKY_TICKET = 54,
		QUEST = 55,
		JEWEL = 56,
		GEMSTONE = 58,
		REFINING_STONE = 59,
		DECORATION_RING = 61,
		MUUN = 63,
		MUUN_MOUNT = 64,
		WIZARDRY_STONE = 67,
		SUMMON_ITEM = 68,
		EXPANSION_RADIANCE = 70,
		BOX_1 = 71,
		BOX_2 = 72,
		BUFF_TALISMAN = 73,
		BUFF_ITEM = 74,
		QUIVER = 75,
		EARRING = 77,
		RUNIC_MACE = 78,
		GUARDIAN_MOUNT = 79,
		SHORT_SWORD = 81,
		MAGIC_GUN = 84,
		MAGIC_STICK = 89,
		MAGIC_ORB = 90,
	};
};

namespace ItemKind3
{
	enum
	{
		NONE = 0,
		COMMON = 1,
		SOCKET = 2,
		PVP = 3,
		LUCKY = 4,
		BOUND = 5,
		ARCHANGEL_WEAPON = 6,
		CHAOS_WEAPON = 7,
		BLESSED_ARCHANGEL_WEAPON = 8,
		PET_GAIN_EXPERIENCE = 9,
	};
};

enum eSpecialNumber
{
	SPECIAL_OPTION1 = 0,
	SPECIAL_OPTION2 = 1,
	SPECIAL_OPTION3 = 2,
	SPECIAL_EXCELLENT1 = 3,
	SPECIAL_EXCELLENT2 = 4,
	SPECIAL_EXCELLENT3 = 5,
	SPECIAL_EXCELLENT4 = 6,
	SPECIAL_EXCELLENT5 = 7,
	SPECIAL_EXCELLENT6 = 8,
	SPECIAL_COMMON1 = 9,
	SPECIAL_COMMON2 = 10,
	SPECIAL_COMMON3 = 11,
	SPECIAL_COMMON4 = 12,
	SPECIAL_COMMON5 = 13,
};

class CPetItemExp
{
public:
	CPetItemExp() // OK
	{
		this->m_DarkSpiritExpTable[0] = 0;
		this->m_DarkSpiritExpTable[1] = 0;

		this->m_DarkHorseExpTable[0] = 0;
		this->m_DarkHorseExpTable[1] = 0;

		this->m_DarkMuunExpTable[0] = 0;
		this->m_DarkMuunExpTable[1] = 0;

		for (int n = 2; n < MAX_PET_LEVEL + 2; n++)
		{
			this->m_DarkSpiritExpTable[n] = ((((n + 10) * n) * n) * n) * 100;
		}

		for (int n = 2; n < MAX_PET_LEVEL + 2; n++)
		{
			this->m_DarkHorseExpTable[n] = ((((n + 10) * n) * n) * n) * 100;
		}

		for (int n = 2; n < MAX_PET_LEVEL + 2; n++)
		{
			this->m_DarkMuunExpTable[n] = ((((n + 10) * n) * n) * n) * 100;
		}
	};
public:
	int m_DarkMuunExpTable[MAX_PET_LEVEL + 2];
	int m_DarkSpiritExpTable[MAX_PET_LEVEL + 2];
	int m_DarkHorseExpTable[MAX_PET_LEVEL + 2];
};

class HOLY_INVENTORY
{
public:
	HOLY_INVENTORY() : bItemindex(-1), bItemCount(0), bItemLevel(-1) {
	};

public:
	short bItemindex;
	short bItemLevel;
	__int64 bItemCount;
};

class CItem
{
public:
	CItem();
	void Clear();
	bool IsItem();
	bool IsExcItem();
	bool IsSetItem();
	bool IsJewelOfHarmonyItem();
	bool Is380Item();
	bool IsSocketItem();
	bool IsLuckyItem();
	bool IsPentagramItem();
	bool IsPentagramJewel();
	bool IsPentagramMithril();
	bool IsEventItem();
	bool IsMuunItem();
	bool IsMuunUtil();
	int  IsClassBot(char aClass, int ChangeUP);//MC bot
	bool IsClass(int Class, int ChangeUp);
	void Convert(int index, BYTE Option1, BYTE Option2, BYTE Option3, BYTE NewOption, BYTE SetOption, BYTE JewelOfHarmonyOption, BYTE ItemOptionEx, BYTE SocketOption[MAX_SOCKET_OPTION], BYTE SocketOptionBonus);
	void Value();
	void OldValue();
	void PetValue();
	void SetPetItemInfo(int PetLevel, int PetExp);
	int GetDamageMin();
	int GetDamageMax();
	int GetDefense();
	int GetDefenseSuccessRate();
	int GetBookSuccessRate();
	bool WeaponDurabilityDown(int aIndex, int defense, int type);
	bool ArmorDurabilityDown(int aIndex, int damage);
	bool WingDurabilityDown(int aIndex, int decrease);
	bool PendantDurabilityDown(int aIndex, int decrease);
	bool RingDurabilityDown(int aIndex, int decrease);
	bool LuckyDurabilityDown(int aIndex, int decrease);
	bool CheckDurabilityState();
	bool AddPetItemExp(int amount);
	bool DecPetItemExp(int amount);

	//--
	void SetStatRequired(int Key, int value);
	bool IsItemSkill();

	bool IsInventoryItem() const;
	bool IsGuardianMount() const;
	bool IsWingLvl1() const;
	bool IsWingLvl2() const;
	bool IsWingLvl3() const;
	bool IsWingMonster() const;
	bool IsMiniWing() const;
	bool IsSpecialWing() const;
	bool IsWingLvl4() const;
	bool IsSocketPendant() const;
public:
	DWORD m_Serial;
	short m_Index;
	short m_Level;
	BYTE m_Slot;
	BYTE m_Class;
	BYTE m_TwoHand;
	BYTE m_AttackSpeed;
	BYTE m_WalkSpeed;
	WORD m_DamageMin;
	WORD m_DamageMax;
	BYTE m_DefenseSuccessRate;
	WORD m_Defense;
	WORD m_MagicDefense;
	BYTE m_Speed;
	WORD m_BaseDamageMin;
	WORD m_BaseDefense;
	WORD m_MagicDamageRate;
	float m_Durability;
	float m_BaseDurability;
	WORD m_DurabilitySmall;
	WORD m_SpecialIndex[MAX_SPECIAL];
	WORD m_SpecialValue[MAX_SPECIAL];
	WORD m_RequireStrength;
	WORD m_RequireDexterity;
	WORD m_RequireEnergy;
	WORD m_RequireLevel;
	WORD m_RequireVitality;
	WORD m_RequireLeadership;
	WORD m_Leadership;
	BYTE m_Resistance[MAX_RESISTANCE_TYPE];
	DWORD m_SellMoney;
	DWORD m_BuyMoney;
	//DWORD GetNumber();//MC
	//int GetSize(int& w, int& h);//MC
	int m_Value;
	int m_PShopValue;
#if(GAMESERVER_UPDATE>=603)
	int m_PShopJoBValue;
	int m_PShopJoSValue;
	int m_PShopJoCValue;
	int m_PShopType;
#endif
	int PCPoint;//MC
	int m_PcPointValue;
	int m_ItemExist;
	int m_OldSellMoney;
	int m_OldBuyMoney;
	BYTE m_Option1;
	BYTE m_Option2;
	BYTE m_Option3;
	BYTE m_NewOption;
	float m_DurabilityState[4];
	float m_CurrentDurabilityState;
	bool m_SkillChange;
	bool m_QuestItem;
	BYTE m_SetOption;
	BYTE m_SetAddStat;
	bool m_IsValidItem;
	bool m_IsExpiredItem;
	bool m_IsPeriodicItem;
	bool m_LoadPeriodicItem;
	int m_PeriodicItemTime;
	int m_LoadPetItemInfo;
	int m_PetItemLevel;
	int m_PetItemExp;
	BYTE m_JewelOfHarmonyOption;
	WORD m_SubRequireStr;
	WORD m_SubRequireDex;
	BYTE m_ItemOptionEx;
	BYTE m_SocketOption[MAX_SOCKET_OPTION];
	BYTE m_SocketOptionBonus;
	//MC bot
	int m_Number;
	BYTE m_RequireClass[MAX_TYPE_PLAYER_BOTS];
	//MC bot
	int Zen;
	int Coin1;
	int Coin2;
	int Coin3;
	int Sell;
	//--
	BYTE Kind1;
	BYTE Kind2;
	BYTE Kind3;

	DWORD PowerATTK;
};
