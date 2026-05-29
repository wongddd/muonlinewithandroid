// ItemManager.cpp: implementation of the CItemManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemManager.h"
#include "BloodCastle.h"
#include "CashShop.h"
#include "CastleSiegeSync.h"
#include "CustomJewel.h"
#include "DSProtocol.h"
#include "Duel.h"
#include "EffectManager.h"
#include "EventInventory.h"
#include "Fruit.h"
#include "IllusionTemple.h"
#include "ItemBagManager.h"
#include "ItemMove.h"
#include "ItemStack.h"
#include "JewelOfHarmonyOption.h"
#include "Kalima.h"
#include "LifeStone.h"
#include "LuckyItem.h"
#include "Map.h"
#include "Mercenary.h"
#include "Message.h"
#include "MossMerchant.h"
#include "MuunSystem.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "PentagramSystem.h"
#include "PersonalShop.h"
#include "Quest.h"
#include "QuestObjective.h"
#include "ServerInfo.h"
#include "ShopManager.h"
#include "SkillManager.h"
#include "SummonScroll.h"
#include "Trade.h"
#include "Util.h"
#include "SetItemOption.h"
#include "Viewport.h"
#include "Warehouse.h"
#include "Path.h"
#include "CMixGoblinExpansion.h"

CItemManager gItemManager;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemManager::CItemManager() // OK
{
	this->m_ItemInfo.clear();
}

CItemManager::~CItemManager() // OK
{
}

void CItemManager::Load(char* filename) // OK
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(filename);

	this->m_ItemInfo.clear();

	if (res.status != pugi::status_ok)
	{
		ErrorMessageBox("Error loading file: %s\nDescription: %s\nOffset: %lld", filename, res.description(), res.offset);
		return;
	}

	char* NameClass[] = NAME_CLASS_ARRAY;

	pugi::xml_node root = file.child("ItemList");

	for (pugi::xml_node child = root.child("Section"); child; child = child.next_sibling())
	{
		int Section = child.attribute("Index").as_int();

		for (pugi::xml_node Item = child.child("Item"); Item; Item = Item.next_sibling())
		{
			ITEM_INFO Item_info;

			memset(&Item_info, 0, sizeof(Item_info));

			Item_info.Index = SafeGetItem(GET_ITEM(Section, Item.attribute("Index").as_int()));

			Item_info.Name = Item.attribute("Name").as_string();

			Item_info.Kind1 = Item.attribute("Kind1").as_int();

			Item_info.Kind2 = Item.attribute("Kind2").as_int();

			Item_info.Kind3 = Item.attribute("Kind3").as_int();

			Item_info.Slot = Item.attribute("Slot").as_int();

			Item_info.Width = Item.attribute("Width").as_int();

			Item_info.Height = Item.attribute("Height").as_int();

			Item_info.DropItem = Item.attribute("Drop").as_int();

			Item_info.Level = Item.attribute("DropLevel").as_int();

			Item_info.m_wSkillIndex = Item.attribute("SkillIndex").as_int();

			Item_info.Value = Item.attribute("Value").as_int();

			Item_info.BuyMoney = Item.attribute("iZen").as_int();

			if (Section != 14)
			{
				Item_info.TwoHand = Item.attribute("TwoHand").as_int();

				Item_info.DamageMin = Item.attribute("DamageMin").as_int();

				Item_info.DamageMax = Item.attribute("DamageMax").as_int();

				Item_info.Defense = Item.attribute("Defense").as_int();

				Item_info.MagicDefense = Item.attribute("MagicDefense").as_int();

				Item_info.DefenseSuccessRate = Item.attribute("SuccessfulBlocking").as_int();

				Item_info.AttackSpeed = Item.attribute("WeaponSpeed").as_int();

				Item_info.WalkSpeed = Item.attribute("WalkSpeed").as_int();

				if (Item_info.Kind2 == ItemKind2::STAFF
					|| Item_info.Kind2 == ItemKind2::STICK
					|| Item_info.Kind2 == ItemKind2::BOOK
					|| Item_info.Kind2 == ItemKind2::RUNIC_MACE
					|| Item_info.Kind2 == ItemKind2::MAGIC_GUN
					|| Item_info.Kind2 == ItemKind2::MAGIC_STICK
					|| Item_info.Kind2 == ItemKind2::MAGIC_ORB
					)
				{
					Item_info.Durability = Item.attribute("MagicDurability").as_int();
					Item_info.MagicDurability = Item_info.Durability;
				}
				else
				{
					Item_info.Durability = Item.attribute("Durability").as_int();
					Item_info.MagicDurability = 0;
				}

				Item_info.MagicDamageRate = Item.attribute("Magicpower").as_int();

				if (Section == 13)
				{
					Item_info.Resistance[0] = Item.attribute("IceRes").as_int();
					Item_info.Resistance[1] = Item.attribute("PoisonRes").as_int();
					Item_info.Resistance[2] = Item.attribute("LightRes").as_int();
					Item_info.Resistance[3] = Item.attribute("FireRes").as_int();
					Item_info.Resistance[4] = Item.attribute("EarthRes").as_int();
					Item_info.Resistance[5] = Item.attribute("WindRes").as_int();
					Item_info.Resistance[6] = Item.attribute("WaterRes").as_int();
				}

				Item_info.RequireLevel = Item.attribute("ReqLevel").as_int();

				if (Section != 13)
				{
					Item_info.RequireStrength = Item.attribute("ReqStrength").as_int();
					Item_info.RequireDexterity = Item.attribute("ReqDexterity").as_int();
					Item_info.RequireEnergy = Item.attribute("ReqEnergy").as_int();
					Item_info.RequireVitality = Item.attribute("ReqVitality").as_int();
					Item_info.RequireLeadership = Item.attribute("ReqCharisma").as_int();
				}

				for (int i = 0; i < MAX_CLASS; i++)
				{
					Item_info.RequireClass[i] = Item.attribute(NameClass[i]).as_int();
				}
			}
			else
			{
				Item_info.Durability = 1;
			}

			if (Item_info.Kind1 == ItemKind1::SKILL)
			{
				Item_info.SkillIndex = Item.attribute("SkillIndex").as_int();
			}

			if (Item_info.Kind1 == ItemKind1::GUARDIAN_MOUNT)
			{
				Item_info.Durability = 0xff;
			}

			/*Item_info.Trade = Item.attribute("Trade").as_int();

			Item_info.StorePersonal = Item.attribute("StorePersonal").as_int();

			Item_info.WhareHouse = Item.attribute("WhareHouse").as_int();

			Item_info.SellNpc = Item.attribute("SellNpc").as_int();

			Item_info.Expensive = Item.attribute("Expensive").as_int();

			Item_info.Repair = Item.attribute("Repair").as_int();

			Item_info.Overlap = Item.attribute("Overlap").as_int();

			Item_info.PcFlag = Item.attribute("PcFlag").as_int();

			Item_info.MuunFlag = Item.attribute("MuunFlag").as_int();

			Item_info.Dropinventory = Item.attribute("DropInventory").as_int();*/

			Item_info.PowerATTK = Item.attribute("ATTK").as_int();

			//strncpy(Item_info.Path, Item.attribute("Path").as_string(), sizeof(Item_info.Path) - 1);

			strncpy(Item_info.Model, Item.attribute("Model").as_string(), sizeof(Item_info.Model) - 1);

			this->m_ItemInfo.insert(type_map_item::value_type(Item_info.Index, Item_info));
		}
	}

	//this->ExportTXT(gPath.GetFullPath("Item\\Item.txt"));
}

#ifndef GAMESERVER_EDITH_EXPORT
void CItemManager::ExportTXT(char* filename)
{
	FILE* fp = fopen(filename, "wb");

	if (fp)
	{
		char* NameClass[] = NAME_CLASS_ARRAY2;

		for (size_t i = 0; i < MAX_ITEM_SECTION; i++)
		{
			std::string strText = "";

			writeFile(fp, (DWORD)i, 0, true);


			writeFile(fp, "Index", strlen("Index") + 3, false);
			writeFile(fp, "Kind1", strlen("Kind1") + 3, false);
			writeFile(fp, "Kind2", strlen("Kind2") + 3, false);
			writeFile(fp, "Kind3", strlen("Kind3") + 3, false);
			writeFile(fp, "Slot", strlen("Slot") + 3, false);
			writeFile(fp, "Width", strlen("Width") + 3, false);
			writeFile(fp, "Height", strlen("Height") + 3, false);
			writeFile(fp, "DropItem", strlen("DropItem") + 2, false);
			writeFile(fp, "SkillIndex", strlen("SkillIndex") + 2, false);

			writeFile(fp, "Name", 60, false);

			if (i >= 0 && i < 12)
			{
				if (i >= 0 && i < 6)
				{
					writeFile(fp, "TwoHand", strlen("TwoHand") + 3, false);
					writeFile(fp, "Level", strlen("Level") + 3, false);
					writeFile(fp, "PowerATTK", strlen("PowerATTK") + 2, false);
					writeFile(fp, "DamageMin", strlen("DamageMin") + 2, false);
					writeFile(fp, "DamageMax", strlen("DamageMax") + 2, false);
					writeFile(fp, "WeaponSpeed", strlen("WeaponSpeed") + 2, false);
					writeFile(fp, "Durability", strlen("Durability") + 2, false);
					writeFile(fp, "MagicDurability", strlen("MagicDurability") + 2, false);
					writeFile(fp, "Magicpower", strlen("Magicpower") + 2, false);
				}
				else
				{
					writeFile(fp, "Level", strlen("Level") + 3, false);
					writeFile(fp, "PowerATTK", strlen("PowerATTK") + 2, false);
					writeFile(fp, "Defense", strlen("Defense") + 3, false);

					if(i == 6)
						writeFile(fp, "SuccessfulBlocking", strlen("SuccessfulBlocking") + 2, false);
					else if (i >= 7 && i <= 9)
						writeFile(fp, "MagicDefense", strlen("MagicDefense") + 2, false);
					else if (i == 10)
						writeFile(fp, "AttackSpeed", strlen("AttackSpeed") + 2, false);
					else if (i == 11)
						writeFile(fp, "WalkSpeed", strlen("WalkSpeed") + 2, false);

					writeFile(fp, "Durability", strlen("Durability") + 2, false);
				}
				writeFile(fp, "ReqLevel", strlen("ReqLevel") + 2, false);
				writeFile(fp, "ReqStrength", strlen("ReqStrength") + 2, false);
				writeFile(fp, "ReqDexterity", strlen("ReqDexterity") + 2, false);
				writeFile(fp, "ReqEnergy", strlen("ReqEnergy") + 2, false);
				writeFile(fp, "ReqVitality", strlen("ReqVitality") + 2, false);
				writeFile(fp, "ReqCharisma", strlen("ReqCharisma") + 2, false);
			}

			if (i != 14)
			{
				for (int i = 0; i < MAX_CLASS; i++)
				{
					writeFile(fp, NameClass[i], strlen(NameClass[i]) + 2, false);
				}
			}


			writeFile(fp, "\n", 0, false);

			for (size_t j = 0; j < MAX_ITEM_TYPE; j++)
			{
				ITEM_INFO item_info;

				if (!this->GetInfo(GET_ITEM(i, j), &item_info))
				{
					continue;
				}
				writeFile(fp, j, strlen("Index") + 3, false);
				writeFile(fp, item_info.Kind1, strlen("Kind1") + 3, false);
				writeFile(fp, item_info.Kind2, strlen("Kind2") + 3, false);
				writeFile(fp, item_info.Kind3, strlen("Kind3") + 3, false);
				writeFile(fp, item_info.Slot, strlen("Slot") + 3, false);
				writeFile(fp, item_info.Width, strlen("Width") + 3, false);
				writeFile(fp, item_info.Height, strlen("Height") + 3, false);
				writeFile(fp, item_info.DropItem, strlen("DropItem") + 2, false);
				writeFile(fp, item_info.m_wSkillIndex, strlen("SkillIndex") + 2, false);

				writeFile(fp, item_info.Name.data(), 60, false, true);

				if (i >= 0 && i < 12)
				{
					if (i >= 0 && i < 6)
					{
						writeFile(fp, item_info.TwoHand, strlen("TwoHand") + 3, false);
						writeFile(fp, item_info.Level, strlen("Level") + 3, false);
				
						writeFile(fp, item_info.PowerATTK, strlen("PowerATTK") + 2, false);
						writeFile(fp, item_info.DamageMin, strlen("DamageMin") + 2, false);
						writeFile(fp, item_info.DamageMax, strlen("DamageMax") + 2, false);
						writeFile(fp, item_info.AttackSpeed, strlen("WeaponSpeed") + 2, false);
						writeFile(fp, item_info.Durability, strlen("Durability") + 2, false);
						writeFile(fp, item_info.MagicDurability, strlen("MagicDurability") + 2, false);
						writeFile(fp, item_info.MagicDamageRate, strlen("Magicpower") + 2, false);
					}
					else
					{
						writeFile(fp, item_info.Level, strlen("Level") + 3, false);
						writeFile(fp, item_info.PowerATTK, strlen("PowerATTK") + 2, false);
						writeFile(fp, item_info.Defense, strlen("Defense") + 3, false);
				
						if (i == 6)
							writeFile(fp, item_info.DefenseSuccessRate, strlen("SuccessfulBlocking") + 2, false);
						else if (i >= 7 && i <= 9)
							writeFile(fp, item_info.MagicDefense, strlen("MagicDefense") + 2, false);
						else if (i == 10)
							writeFile(fp, item_info.AttackSpeed, strlen("AttackSpeed") + 2, false);
						else if (i == 11)
							writeFile(fp, item_info.WalkSpeed, strlen("WalkSpeed") + 2, false);
				
						writeFile(fp, item_info.Durability, strlen("Durability") + 2, false);
					}
					writeFile(fp, item_info.RequireLevel, strlen("ReqLevel") + 2, false);
					writeFile(fp, item_info.RequireStrength, strlen("ReqStrength") + 2, false);
					writeFile(fp, item_info.RequireDexterity, strlen("ReqDexterity") + 2, false);
					writeFile(fp, item_info.RequireEnergy, strlen("ReqEnergy") + 2, false);
					writeFile(fp, item_info.RequireVitality, strlen("ReqVitality") + 2, false);
					writeFile(fp, item_info.RequireLeadership, strlen("ReqCharisma") + 2, false);
				}
				else if (i == 12)
				{
					writeFile(fp, item_info.Level, strlen("Level") + 3, false);
					writeFile(fp, item_info.PowerATTK, strlen("PowerATTK") + 2, false);
					writeFile(fp, item_info.Defense, strlen("Defense") + 3, false);
					writeFile(fp, item_info.Durability, strlen("Durability") + 2, false);
					writeFile(fp, item_info.DamageMin, strlen("DamageMin") + 2, false);
					writeFile(fp, item_info.DamageMax, strlen("DamageMax") + 2, false);
					writeFile(fp, item_info.RequireLevel, strlen("ReqLevel") + 2, false);
					writeFile(fp, item_info.RequireStrength, strlen("ReqStrength") + 2, false);
					writeFile(fp, item_info.RequireDexterity, strlen("ReqDexterity") + 2, false);
					writeFile(fp, item_info.RequireEnergy, strlen("ReqEnergy") + 2, false);
					writeFile(fp, item_info.RequireVitality, strlen("ReqVitality") + 2, false);
					writeFile(fp, item_info.RequireLeadership, strlen("ReqCharisma") + 2, false);
				}

				if (i != 14)
				{
					for (int i = 0; i < MAX_CLASS; i++)
					{
						writeFile(fp, item_info.RequireClass[i], strlen(NameClass[i]) + 2, false);
					}
				}



				writeFile(fp, "\n", 0, false);
			}
			writeFile(fp, "end\n", 0, true);
		}
		fclose(fp);
	}
}
#endif //GAMESERVER_EDITH_EXPORT

void CItemManager::ExportBMD(char* filename)
{
	std::vector<Script_Item> ItemMemory;

	for (type_map_item::iterator it = m_ItemInfo.begin(); it != m_ItemInfo.end(); ++it)
	{
		Script_Item info;
		ITEM_INFO* ItemInfo = &it->second;

		memset(&info, 0, sizeof(info));

		info.Type = ItemInfo->Index;

		info.Index = (int)(ItemInfo->Index / 512);

		info.SubIndex = (int)(ItemInfo->Index % 512);

		info.Kind1 = ItemInfo->Kind1;

		info.Kind2 = ItemInfo->Kind2;

		info.Kind3 = ItemInfo->Kind3;

		info.m_byItemSlot = ItemInfo->Slot;

		info.Width = ItemInfo->Width;

		info.Height = ItemInfo->Height;

		info.Level = ItemInfo->Level;

		info.Value = ItemInfo->Value;

		info.iZen = ItemInfo->BuyMoney;

		info.m_wSkillIndex = ItemInfo->m_wSkillIndex;

		if (info.Index != 14)
		{
			info.TwoHand = ItemInfo->TwoHand;

			info.DamageMin = ItemInfo->DamageMin;

			info.DamageMax = ItemInfo->DamageMax;

			info.Defense = ItemInfo->Defense;

			info.MagicDefense = ItemInfo->MagicDefense;

			info.SuccessfulBlocking = ItemInfo->DefenseSuccessRate;

			info.WeaponSpeed = ItemInfo->AttackSpeed;

			info.WalkSpeed = ItemInfo->WalkSpeed;

			info.Durability = ItemInfo->Durability;

			info.MagicDurability = ItemInfo->MagicDurability;

			info.MagicPower = ItemInfo->MagicDamageRate;

			info.RequireLevel = ItemInfo->RequireLevel;

			if (info.Index != 13)
			{
				info.RequireStrength = ItemInfo->RequireStrength;

				info.RequireDexterity = ItemInfo->RequireDexterity;

				info.RequireEnergy = ItemInfo->RequireEnergy;

				info.RequireVitality = ItemInfo->RequireVitality;

				info.RequireCharisma = ItemInfo->RequireLeadership;
			}

			int ItemOption = gSetItemType.GetSetItemStatType(ItemInfo->Index);

			switch (ItemOption)
			{
			case SET_ITEM_OPTION_ADD_STRENGTH:
				info.AttType = 1;
				break;
			case SET_ITEM_OPTION_ADD_DEXTERITY:
				info.AttType = 2;
				break;
			case SET_ITEM_OPTION_ADD_VITALITY:
				info.AttType = 4;
				break;
			case SET_ITEM_OPTION_ADD_ENERGY:
				info.AttType = 3;
				break;
			case SET_ITEM_OPTION_ADD_LEADERSHIP:
				info.AttType = 5;
				break;
			default:
				info.AttType = 0;
				break;
			}
		}

		for (int i = 0; i < MAX_RESISTANCE_TYPE; i++)
		{
			info.Resistance[i] = ItemInfo->Resistance[i];
		}

		for (int i = 0; i < MAX_CLASS; i++)
		{
			info.RequireClass[i] = ItemInfo->RequireClass[i];
		}

		//info.Trade = ItemInfo->Trade;
		//
		//info.StorePersonal = ItemInfo->StorePersonal;
		//
		//info.WhareHouse = ItemInfo->WhareHouse;
		//
		//info.SellNpc = ItemInfo->SellNpc;
		//
		//info.Expensive = ItemInfo->Expensive;
		//
		//info.Repair = ItemInfo->Repair;
		//
		//info.Overlap = ItemInfo->Overlap;

		//info.PcFlag = ItemInfo->PcFlag;

		//info.MuunFlag = ItemInfo->MuunFlag;

		//info.Dropinventory = ItemInfo->Dropinventory;

		info.PowerATTK = ItemInfo->PowerATTK;

		strcpy_s(info.Name, ItemInfo->Name.data());

		//strcpy_s(info.Path, ItemInfo->Path);

		//strcpy_s(info.Model, ItemInfo->Model);

		ItemMemory.push_back(info);
	}

	PackFileEncrypt(filename, (BYTE*)ItemMemory.data(), ItemMemory.size(), sizeof(Script_Item), 0xE2F1);
}

void CItemManager::ExportModel(char* filename)
{
	FILE* fp = fopen(filename, "wb");
	if (fp)
	{
		std::vector<Script_Item> ItemMemory;

		char temp[128]; // Buffer temporal para cada campo
		char buffer[512]; // Tamaño suficiente para una línea completa

		for (type_map_item::iterator it = m_ItemInfo.begin(); it != m_ItemInfo.end(); ++it)
		{
			ITEM_INFO* item_info = &it->second;

			__int64 index = item_info->Index;
			__int64 nType = (index >> 9);
			__int64 nIndex = (index % 512);
			__int64 Kind1 = item_info->Kind1;
			__int64 Kind2 = item_info->Kind2;
			__int64 Kind3 = item_info->Kind3;

			buffer[0] = '\0';

			// Formatear y concatenar cada campo al buffer
			sprintf(temp, "%-5lld ", index);
			strcat(buffer, temp);

			sprintf(temp, "%-5lld ", nType);
			strcat(buffer, temp);

			sprintf(temp, "%-5lld ", nIndex);
			strcat(buffer, temp);

			sprintf(temp, "Kind1: %-5lld ", Kind1);
			strcat(buffer, temp);

			sprintf(temp, "Kind2: %-5lld ", Kind2);
			strcat(buffer, temp);

			sprintf(temp, "Kind3: %-5lld ", Kind3);
			strcat(buffer, temp);

			// Agregar el modelo con comillas
			strcat(buffer, "\"");
			strcat(buffer, item_info->Model);
			strcat(buffer, "\"\n");

			// Escribir el buffer al archivo
			fwrite(buffer, 1, strlen(buffer), fp);
		}
		fclose(fp);
	}
}

bool CItemManager::GetInfo(int index, ITEM_INFO* lpInfo) // OK
{
	std::map<int, ITEM_INFO>::iterator it = this->m_ItemInfo.find(index);

	if (it == this->m_ItemInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

int CItemManager::GetItemSkill(int index) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(index, &ItemInfo) == 0)
	{
		return -1;
	}
	else
	{
		return ItemInfo.m_wSkillIndex;
	}

}

int CItemManager::CheckItemSkill(int index)
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(index, &ItemInfo) == 0 || ItemInfo.m_wSkillIndex == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

const char* CItemManager::GetItemName(int index) // OK
{
	std::map<int, ITEM_INFO>::iterator it = this->m_ItemInfo.find(index);

	if (it == this->m_ItemInfo.end())
	{
		return "No Name";
	}
	else
	{
		return it->second.Name.data();
	}
}

int CItemManager::GetItemTwoHand(int index) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(index, &ItemInfo) == 0)
	{
		return -1;
	}
	else
	{
		return ItemInfo.TwoHand;
	}
}

int CItemManager::AdquireSkill(int Itemindex)
{
	int acquire_skill = -1;

	ITEM_INFO ItemInfo;

	if (this->GetInfo(Itemindex, &ItemInfo) != 0)
	{
		if (ItemInfo.Kind1 == ItemKind1::SKILL && ItemInfo.SkillIndex > 0)
		{
			acquire_skill = ItemInfo.SkillIndex;
		}
	}


	return acquire_skill;
}

bool CItemManager::IsInventoryItem(int Itemindex)
{
	bool success = false;

	ITEM_INFO ItemInfo;

	if (this->GetInfo(Itemindex, &ItemInfo) != 0)
	{
		if (ItemInfo.Kind2 == ItemKind2::INVENTORY_ITEM
			|| ItemInfo.Kind2 == ItemKind2::BUFF_ITEM
			|| ItemInfo.Kind2 == ItemKind2::GUARDIAN_MOUNT
			)
		{
			success = true;
		}
	}

	return success;
}

int CItemManager::GetItemDurability(int index, int level, int NewOption, int SetOption) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(index, &ItemInfo) == 0)
	{
		return 0;
	}

	if (index == GET_ITEM(12, 144) || index == GET_ITEM(12, 146) || gPentagramSystem.IsPentagramItem(index) != 0) // Mithril Fragment,Elixir Fragment
	{
		return ItemInfo.Durability;
	}

	if (index == GET_ITEM(14, 21) || index == GET_ITEM(14, 29) || index == GET_ITEM(14, 100) || index == GET_ITEM(14, 215)) // Rena,Symbol of Kundun,Lucky Coin,Card Piece
	{
		return 1;
	}

	int MaxStack = gItemStack.GetItemMaxStack(index, level);

	if ((index >= GET_ITEM(14, 0) && index <= GET_ITEM(14, 6))
		|| (index >= GET_ITEM(14, 35) && index <= GET_ITEM(14, 40))
		|| index == GET_ITEM(14, 70) || index == GET_ITEM(14, 71) || index == GET_ITEM(14, 133))
	{
		if (MaxStack > 0)
			return MaxStack;
		else
			return 1;
	}

	if (MaxStack > 0)
	{
		if (index == GET_ITEM(4, 7) || index == GET_ITEM(4, 15))
			return MaxStack;
		else
			return 1;
	}

	if (index == GET_ITEM(13, 18) || index == GET_ITEM(13, 29) || index == GET_ITEM(13, 51) || index == GET_ITEM(14, 19) || index == GET_ITEM(14, 102) || index == GET_ITEM(14, 109) || index == GET_ITEM(14, 110)) // Invisibility Cloak,Armor of Guardsman,Scroll of Blood,Devil's Invitation,Gaion's Order,Complete Secromicon,Sign of Dimensions
	{
		return 1;
	}

	int dur = 0;

	if (level >= 5)
	{
		if (level == 10)
		{
			dur = ItemInfo.Durability + ((level * 2) - 3);
		}
		else if (level == 11)
		{
			dur = ItemInfo.Durability + ((level * 2) - 1);
		}
		else if (level == 12)
		{
			dur = ItemInfo.Durability + ((level * 2) + 2);
		}
		else if (level == 13)
		{
			dur = ItemInfo.Durability + ((level * 2) + 6);
		}
		else if (level == 14)
		{
			dur = ItemInfo.Durability + ((level * 2) + 11);
		}
		else if (level == 15)
		{
			dur = ItemInfo.Durability + ((level * 2) + 17);
		}
		else
		{
			dur = ItemInfo.Durability + ((level * 2) - 4);
		}
	}
	else
	{
		dur = ItemInfo.Durability + level;
	}

	if (index != GET_ITEM(0, 19) && index != GET_ITEM(2, 13) && index != GET_ITEM(4, 18) && index != GET_ITEM(5, 10) && index != GET_ITEM(5, 36) && ItemInfo.Slot != 7) // Sword of Archangel,Scepter of Archangel,Crossbow of Archangel,Staff of Archangel,Stick of Archangel
	{
		if (SetOption != 0)
		{
			dur += 20;
		}
		else if (NewOption != 0)
		{
			dur += 15;
		}
	}

	return ((dur > 255) ? 255 : dur);
}

int CItemManager::GetItemRepairMoney(CItem* lpItem, int type) // OK
{
	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if (lpItem->m_Durability == lpItem->m_BaseDurability)
	{
		return 0;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(lpItem->m_Index, &ItemInfo) == 0)
	{
		return 0;
	}

	if (ItemInfo.Durability == 0)
	{
		return 0;
	}

	int RepairMoney = 0;

	if (lpItem->m_Index == GET_ITEM(13, 4) || lpItem->m_Index == GET_ITEM(13, 5)
#ifdef PET_SYSTEM_GAIN_EXP
		|| (lpItem->Kind3 == ItemKind3::PET_GAIN_EXPERIENCE)
#endif // PET_SYSTEM_GAIN_EXP
		) // Dark Horse,Dark Reaven
	{
		RepairMoney = lpItem->m_BuyMoney / 1;
	}
	else
	{
		RepairMoney = lpItem->m_BuyMoney / 3;
	}

	RepairMoney = ((RepairMoney > MAX_MONEY) ? MAX_MONEY : RepairMoney);

	RepairMoney = ((RepairMoney >= 100) ? ((RepairMoney / 10) * 10) : RepairMoney);

	RepairMoney = ((RepairMoney >= 1000) ? ((RepairMoney / 100) * 100) : RepairMoney);

	float sq1 = sqrt((float)RepairMoney);
	float sq2 = sqrt(sq1);

	float value = (float)((((3.0 * sq1) * sq2) * (1 - (lpItem->m_Durability / lpItem->m_BaseDurability))) + 1);

	if (lpItem->m_Durability <= 0)
	{
		if (lpItem->m_Index == GET_ITEM(13, 4) || lpItem->m_Index == GET_ITEM(13, 5)
#ifdef PET_SYSTEM_GAIN_EXP
			|| (lpItem->Kind3 == ItemKind3::PET_GAIN_EXPERIENCE)
#endif // PET_SYSTEM_GAIN_EXP
			) // Dark Horse,Dark Reaven
		{
			value *= (float)2.0f;
		}
		else
		{
			value *= (float)1.4f;
		}
	}

	int money = (int)((type == 1) ? (value * 2.5f) : value);

	money = ((money >= 100) ? ((money / 10) * 10) : money);

	money = ((money >= 1000) ? ((money / 100) * 100) : money);

	return money;
}

int CItemManager::GetInventoryItemSlot(LPOBJ lpObj, int index, int level) // OK
{
	int MaxValue = this->GetInventoryMaxValue(lpObj);

	for (int n = 0; n < MaxValue; n++)
	{
		if (lpObj->Inventory[n].IsItem() != 0)
		{
			if (lpObj->Inventory[n].m_Index == index && (level == -1 || lpObj->Inventory[n].m_Level == level))
			{
				return n;
			}
		}
	}

	return -1;
}

int CItemManager::GetInventoryItemCount(LPOBJ lpObj, int index, int level) // OK
{
	int count = 0;

	int MaxValue = this->GetInventoryMaxValue(lpObj);

	for (int n = 0; n < MaxValue; n++)
	{
		CItem* lpItem = &lpObj->Inventory[n];

		if (lpItem->IsItem() != 0)
		{
			if (lpItem->m_Index == index && (level == -1 || lpItem->m_Level == level))
			{
				if (gItemStack.GetItemMaxStack(lpItem->m_Index, lpItem->m_Level) == 0)
				{
					count++;
				}
				else
				{
					count += (int)lpItem->m_Durability;
				}
			}
		}
	}

	return count;
}

int CItemManager::GetInventoryEmptySlotCount(LPOBJ lpObj) // OK
{
	int count = 0;

	int MaxValue = this->GetInventoryMaxValue(lpObj) - INVENTORY_WEAR_SIZE;

	for (int n = 0; n < MaxValue; n++)
	{
		if (lpObj->InventoryMap[n] == 0xFF)
		{
			count++;
		}
	}

	return count;
}

int CItemManager::GetInventoryMaxValue(LPOBJ lpObj) // OK
{
	int MaxValue = INVENTORY_MAIN_SIZE;

	switch (lpObj->ExtInventory)
	{
	case 0:
		MaxValue = INVENTORY_MAIN_SIZE;
		break;
	case 1:
		MaxValue = INVENTORY_EXT1_SIZE;
		break;
	case 2:
		MaxValue = INVENTORY_EXT2_SIZE;
		break;
	case 3:
		MaxValue = INVENTORY_EXT3_SIZE;
		break;
	case 4:
		MaxValue = INVENTORY_EXT4_SIZE;
		break;
	default:
		MaxValue = INVENTORY_EXT4_SIZE;
		break;
	}

	return MaxValue;
}

bool CItemManager::CheckItemRequireLevel(LPOBJ lpObj, CItem* lpItem) // OK
{
	if (lpObj->GetLevel() >= lpItem->m_RequireLevel)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool CItemManager::CheckItemRequireStrength(LPOBJ lpObj, CItem* lpItem) // OK
{
	if ((lpObj->Strength + lpObj->AddStrength) >= (lpItem->m_RequireStrength - lpItem->m_SubRequireStr))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool CItemManager::CheckItemRequireDexterity(LPOBJ lpObj, CItem* lpItem) // OK
{
	if ((lpObj->Dexterity + lpObj->AddDexterity) >= (lpItem->m_RequireDexterity - lpItem->m_SubRequireDex))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool CItemManager::CheckItemRequireVitality(LPOBJ lpObj, CItem* lpItem) // OK
{
	if ((lpObj->Vitality + lpObj->AddVitality) >= lpItem->m_RequireVitality)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool CItemManager::CheckItemRequireEnergy(LPOBJ lpObj, CItem* lpItem) // OK
{
	if ((lpObj->Energy + lpObj->AddEnergy) >= lpItem->m_RequireEnergy)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool CItemManager::CheckItemRequireLeadership(LPOBJ lpObj, CItem* lpItem) // OK
{
	if ((lpObj->Leadership + lpObj->AddLeadership) >= lpItem->m_RequireLeadership)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool CItemManager::CheckItemRequireClass(LPOBJ lpObj, int index) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(index, &ItemInfo) == 0)
	{
		return 0;
	}

	if (CHECK_RANGE(lpObj->Class, MAX_CLASS) == 0)
	{
		return 0;
	}

	if (ItemInfo.RequireClass[lpObj->Class] == 0)
	{
		return 0;
	}

	if ((lpObj->ChangeUp + 1) >= ItemInfo.RequireClass[lpObj->Class])
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

bool CItemManager::CheckItemMoveToInventory(LPOBJ lpObj, CItem* lpItem, int slot) // OK
{
	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if (INVENTORY_WEAR_RANGE(slot) == 0)
	{
		return 1;
	}

	if (this->CheckItemRequireLevel(lpObj, lpItem) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "CheckItemRequireLevel");
		return false;
	}

	if (this->CheckItemRequireStrength(lpObj, lpItem) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "CheckItemRequireStrength");
		return false;
	}

	if (this->CheckItemRequireDexterity(lpObj, lpItem) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "CheckItemRequireDexterity");
		return false;
	}

	if (this->CheckItemRequireVitality(lpObj, lpItem) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "CheckItemRequireVitality");
		return false;
	}

	if (this->CheckItemRequireEnergy(lpObj, lpItem) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "CheckItemRequireEnergy");
		return false;
	}

	if (this->CheckItemRequireLeadership(lpObj, lpItem) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "CheckItemRequireLeadership");
		return false;
	}

	if (this->CheckItemRequireClass(lpObj, lpItem->m_Index) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "CheckItemRequireClass");
		return false;
	}

	if (lpItem->m_Slot != slot && slot != 0 && slot != 1 && slot != 10 && slot != 11)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "diferent slot ring o sword");
		return false;
	}

	if (lpItem->m_Slot != slot && ((slot == 0 || slot == 1) && lpItem->m_Slot != 0 && lpItem->m_Slot != 1))
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "diferent slot sword");
		return false;
	}

	if (lpItem->m_Slot != slot && ((slot == 10 || slot == 11) && lpItem->m_Slot != 10 && lpItem->m_Slot != 11))
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "diferent slot ring");
		return false;
	}

	if (lpItem->m_TwoHand && (slot == 1 || (slot == 0 && lpObj->Inventory[1].IsItem() != 0)))
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "m_TwoHand");
		return false;
	}

	if (slot == 0 && lpObj->Inventory[1].IsItem() != 0)
	{
		int kind2 = lpObj->Inventory[1].Kind2;

		if (lpItem->Kind2 == ItemKind2::BOW && kind2 != ItemKind2::ARROW && kind2 != ItemKind2::QUIVER)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "ARROW | QUIVER");
			return false;
		}

		if (lpItem->Kind2 == ItemKind2::CROSSBOW && kind2 != ItemKind2::BOLT && kind2 != ItemKind2::QUIVER)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "BOLT | QUIVER");
			return false;
		}
	}

	if (lpObj->Map == MAP_ATLANS)
	{
		if (lpItem->m_Index == GET_ITEM(13, 2) || lpItem->m_Index == GET_ITEM(13, 3)) // Uniria,Dinorant
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "MAP_ATLANS");
			return false;
		}
	}

	if (CC_MAP_RANGE(lpObj->Map) != 0 || IT_MAP_RANGE(lpObj->Map) != 0)
	{
		if (lpItem->m_Index == GET_ITEM(13, 10) || (lpItem->m_Index >= GET_ITEM(13, 39) && lpItem->m_Index <= GET_ITEM(13, 42)) || lpItem->m_Index == GET_ITEM(13, 68) || lpItem->m_Index == GET_ITEM(13, 76) || lpItem->m_Index == GET_ITEM(13, 122)) // Rings
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "ring transformation en evento");
			return false;
		}
	}

	if (lpItem->m_Index == GET_ITEM(13, 10) || (lpItem->m_Index == GET_ITEM(13, 20) && lpItem->m_Level == 0) || (lpItem->m_Index >= GET_ITEM(13, 38) && lpItem->m_Index <= GET_ITEM(13, 42)) || lpItem->m_Index == GET_ITEM(13, 68) || lpItem->m_Index == GET_ITEM(13, 76) || lpItem->m_Index == GET_ITEM(13, 107) || lpItem->m_Index == GET_ITEM(13, 122)) // Rings
	{
		if (lpObj->Inventory[slot ^ 1].IsItem() != 0 && lpObj->Inventory[slot ^ 1].m_Index == lpItem->m_Index)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "ring transformation");
			return false;
		}
	}

	return true;
}

bool CItemManager::CheckItemMoveToTrade(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag) // OK
{
	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if (gServerInfo.m_TradeItemBlockLucky != 0 && lpItem->IsLuckyItem() != 0)
	{
		return 0;
	}

	if (lpItem->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	if (lpItem->IsPentagramItem() != 0)
	{
		if (gPentagramSystem.CheckPentagramSocket(lpObj, lpItem) != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(499));
			return 0;
		}
	}

	if (gItemMove.CheckItemMoveAllowTrade(lpItem->m_Index) == 0)
	{
		return 0;
	}

	if (this->CheckItemMoveToBlock(lpObj, lpItem) == 0)
	{
		return 0;
	}

	//itemLock
	if (lpObj->Lock > 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		return 0;
	}

	if (lpObj->TradeDuel != 0)
	{
		return 0;
	}

	return 1;
}

bool CItemManager::CheckItemMoveToVault(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag) // OK
{
	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if (gServerInfo.m_TradeItemBlockLucky != 0 && lpItem->IsLuckyItem() != 0)
	{
		return 0;
	}

	if (lpItem->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	if (lpItem->IsPentagramItem() != 0) {
		if (gPentagramSystem.CheckPentagramSocket(lpObj, lpItem) != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(499));
			return 0;
		}
	}

	if (gItemMove.CheckItemMoveAllowVault(lpItem->m_Index) == 0)
	{
		return 0;
	}

	//itemLock
	if (lpObj->Lock > 0 && lpObj->LoadGuildWarehouse == 1)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		return 0;
	}

	return 1;
}

bool CItemManager::CheckItemMoveToChaos(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag) // OK
{
	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if (lpItem->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	if (TargetFlag == 16 && lpItem->IsLuckyItem() == 0)
	{
		return 0;
	}

	if (TargetFlag != 8 && TargetFlag != 16 && lpItem->IsLuckyItem() != 0)
	{
		return 0;
	}

	if (lpItem->IsPentagramItem() != 0)
	{
		if (gPentagramSystem.CheckPentagramSocket(lpObj, lpItem) != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(499));
			return 0;
		}
	}

	if (lpObj->Lock > 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		return 0;
	}

	return 1;
}

#ifdef CHAOS_MACHINE_EXTENSION
bool CItemManager::CheckItemMoveToMixExpansion(LPOBJ lpObj, CItem* lpItem, BYTE TargetFlag)
{
	if (lpItem->IsItem() == 0)
	{
		return false;
	}

	if (lpItem->m_IsPeriodicItem != 0)
	{
		return false;
	}

	if (lpItem->IsPentagramItem() != 0)
	{
		if (gPentagramSystem.CheckPentagramSocket(lpObj, lpItem) != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(499));
			return 0;
		}
	}

	if (lpObj->Lock > 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		return 0;
	}

	return true;
}
#endif // CHAOS_MACHINE_EXTENSION


bool CItemManager::CheckItemMoveToBlock(LPOBJ lpObj, CItem* lpItem) // OK
{
	if (gServerInfo.m_TradeItemBlock == 0 || lpObj->Authority == 32)
	{
		return 1;
	}

	if (gServerInfo.m_TradeItemBlockExc != 0 && lpItem->IsExcItem() != 0 && GetNewOptionCount(lpItem->m_NewOption) >= gServerInfo.m_TradeItemBlockExc)
	{
		return 0;
	}

	if (gServerInfo.m_TradeItemBlockSet != 0 && lpItem->IsSetItem() != 0)
	{
		return 0;
	}

	if (gServerInfo.m_TradeItemBlockHarmony != 0 && lpItem->IsJewelOfHarmonyItem() != 0)
	{
		return 0;
	}

	return 1;
}

bool CItemManager::CheckItemInventorySpace(LPOBJ lpObj, int index) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(index, &ItemInfo) == 0)
	{
		return 0;
	}

	int MaxY = (this->GetInventoryMaxValue(lpObj) - INVENTORY_WEAR_SIZE) / 8;

	for (int y = 0; y < MaxY; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (lpObj->InventoryMap[((y * 8) + x)] == 0xFF)
			{
				if (this->InventoryRectCheck(lpObj->Index, x, y, ItemInfo.Width, ItemInfo.Height) != 0xFF)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

bool CItemManager::CheckItemInventorySpace(LPOBJ lpObj, int index, int count, bool copy, bool copy2)
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(index, &ItemInfo) == 0)
	{
		return false;
	}

	int MaxY = (this->GetInventoryMaxValue(lpObj) - INVENTORY_WEAR_SIZE) / 8;

	BYTE InventoryMap[INVENTORY_FULL_SIZE];

	memcpy(InventoryMap, lpObj->InventoryMap, INVENTORY_FULL_SIZE);

	int totalAdd = 0;

	for (size_t i = 0; i < count; i++)
	{
		if (CheckItemInventorySpace(&InventoryMap[0], MaxY, ItemInfo.Width, ItemInfo.Height))
		{
			totalAdd++;
		}
	}

	return totalAdd == count;
}

bool CItemManager::CheckItemInventorySpace(BYTE* InventoryMap, int MaxY, int width, int height)
{
	for (int y = 0; y < MaxY; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (InventoryMap[((y * 8) + x)] == 0xFF)
			{
				BYTE indexslot = this->InventoryRectCheck(InventoryMap, x, y, width, height);

				if (indexslot != 0xFF)
				{
					InventoryItemSet(InventoryMap, indexslot, 1, width, height);
					return true;
				}
			}
		}
	}

	return 0;
}

bool CItemManager::CheckItemInventorySpace(LPOBJ lpObj, int width, int height) // OK
{
	int MaxY = (this->GetInventoryMaxValue(lpObj) - INVENTORY_WEAR_SIZE) / 8;

	for (int y = 0; y < MaxY; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (lpObj->InventoryMap[((y * 8) + x)] == 0xFF)
			{
				if (this->InventoryRectCheck(lpObj->Index, x, y, width, height) != 0xFF)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

void CItemManager::InventoryItemSet(int aIndex, int slot, BYTE type) // OK
{
	if (INVENTORY_BASE_RANGE(slot) == 0)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(gObj[aIndex].Inventory[slot].m_Index, &ItemInfo) == 0)
	{
		return;
	}

	int x = (slot - INVENTORY_WEAR_SIZE) % 8;
	int y = (slot - INVENTORY_WEAR_SIZE) / 8;

	if (INVENTORY_MAIN_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 8))
	{
		return;
	}

	if (INVENTORY_EXT1_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 12))
	{
		return;
	}

	if (INVENTORY_EXT2_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 16))
	{
		return;
	}

	if (INVENTORY_EXT3_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 20))
	{
		return;
	}

	if (INVENTORY_EXT4_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 24))
	{
		return;
	}

	if (INVENTORY_SHOP_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 28))
	{
		return;
	}

	for (int sy = 0; sy < ItemInfo.Height; sy++)
	{
		for (int sx = 0; sx < ItemInfo.Width; sx++)
		{
			gObj[aIndex].InventoryMap[(((sy + y) * 8) + (sx + x))] = type;
		}
	}
}

void CItemManager::InventoryItemSet(BYTE* InventoryMap, int slot, int width, int height, BYTE type)
{
	if (INVENTORY_BASE_RANGE(slot) == 0)
	{
		return;
	}

	int x = (slot - INVENTORY_WEAR_SIZE) % 8;
	int y = (slot - INVENTORY_WEAR_SIZE) / 8;

	for (int sy = 0; sy < height; sy++)
	{
		for (int sx = 0; sx < width; sx++)
		{
			InventoryMap[(((sy + y) * 8) + (sx + x))] = type;
		}
	}
}

BYTE CItemManager::InventoryRectCheck(int aIndex, int x, int y, int width, int height) // OK
{
	int slot = ((y * 8) + x) + INVENTORY_WEAR_SIZE;

	if (INVENTORY_MAIN_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 8))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 12))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 16))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 20))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 24))
	{
		return 0xFF;
	}

	if (INVENTORY_SHOP_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 28))
	{
		return 0xFF;
	}

	for (int sy = 0; sy < height; sy++)
	{
		for (int sx = 0; sx < width; sx++)
		{
			if (gObj[aIndex].InventoryMap[(((sy + y) * 8) + (sx + x))] != 0xFF)
			{
				return 0xFF;
			}
		}
	}

	return slot;
}

BYTE CItemManager::InventoryRectCheck(BYTE* InventoryMap, int x, int y, int width, int height)
{
	int slot = ((y * 8) + x) + INVENTORY_WEAR_SIZE;

	if (INVENTORY_MAIN_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 8))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 12))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 16))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 20))
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 24))
	{
		return 0xFF;
	}

	if (INVENTORY_SHOP_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 28))
	{
		return 0xFF;
	}

	for (int sy = 0; sy < height; sy++)
	{
		for (int sx = 0; sx < width; sx++)
		{
			if (InventoryMap[(((sy + y) * 8) + (sx + x))] != 0xFF)
			{
				return 0xFF;
			}
		}
	}

	return slot;
}

BYTE CItemManager::InventoryInsertItem(int aIndex, CItem item) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	int MaxY = (this->GetInventoryMaxValue(&gObj[aIndex]) - INVENTORY_WEAR_SIZE) / 8;

	for (int y = 0; y < MaxY; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (gObj[aIndex].InventoryMap[((y * 8) + x)] == 0xFF)
			{
				BYTE slot = this->InventoryRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height);

				if (slot != 0xFF)
				{
					gObj[aIndex].Inventory[slot] = item;
					this->InventoryItemSet(aIndex, slot, 1);
					return slot;
				}
			}
		}
	}

	return 0xFF;
}

BYTE CItemManager::InventoryAddItem(int aIndex, CItem item, int slot) // OK
{
	if (INVENTORY_RANGE(slot) == 0)
	{
		return 0xFF;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Inventory[slot].IsItem() != 0 || item.IsItem() == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_WEAR_RANGE(slot) == 0)
	{
		ITEM_INFO ItemInfo;

		if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
		{
			return 0xFF;
		}

		int x = (slot - INVENTORY_WEAR_SIZE) % 8;
		int y = (slot - INVENTORY_WEAR_SIZE) / 8;

		if (this->InventoryRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height) == 0xFF)
		{
			return 0xFF;
		}

		lpObj->Inventory[slot] = item;

		this->InventoryItemSet(aIndex, slot, 1);

		return slot;
	}
	else
	{
		if (this->CheckItemRequireClass(lpObj, item.m_Index) == 0)
		{
			return 0xFF;
		}

		lpObj->Inventory[slot] = item;

		if (lpObj->Inventory[slot].m_Option1 != 0)
		{
			gSkillManager.AddSkillWeapon(lpObj, lpObj->Inventory[slot].m_SpecialIndex[SPECIAL_OPTION1], lpObj->Inventory[slot].m_Level);
		}

		return slot;
	}
}

void CItemManager::InventoryDelItem(int aIndex, int slot) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (INVENTORY_WEAR_RANGE(slot) == 0)
	{
		this->InventoryItemSet(aIndex, slot, 0xFF);
		lpObj->Inventory[slot].Clear();
	}
	else
	{
		if (lpObj->Inventory[slot].m_Option1 != 0)
		{
			gSkillManager.DelSkillWeapon(lpObj, lpObj->Inventory[slot].m_SpecialIndex[SPECIAL_OPTION1], lpObj->Inventory[slot].m_Level);
		}
		lpObj->Inventory[slot].Clear();
	}
}

bool CItemManager::InventoryBreakItemStack(LPOBJ lpObj, CItem* lpItem, int slot)
{
	int MaxStack, CreateItemIndex;

	if (lpItem->IsItem() == 0)
	{
		return false;
	}

	if ((MaxStack = gItemStack.GetItemMaxStack(lpItem->m_Index, lpItem->m_Level)) <= 0)
	{
		return false;
	}

	if (lpItem->m_Durability > 1)
	{
		if (gItemStack.IsBreakUp(lpItem->m_Index, lpItem->m_Level) != 0)
		{
			if (this->CheckItemInventorySpace(lpObj, lpItem->m_Index) == true)
			{
				DecreaseItemDur(lpObj, slot, 1);
				GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, lpItem->m_Index, lpItem->m_Level, 0, lpItem->m_Option1, lpItem->m_Option2, lpItem->m_Option3, -1, 0, 0, 0, 0, 0, 0xFF, 0);
			}
		}
	}

	return false;
}

bool CItemManager::InventoryInsertItemStack(LPOBJ lpObj, CItem* lpItem) // OK
{
	int MaxStack, CreateItemIndex;

	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if ((MaxStack = gItemStack.GetItemMaxStack(lpItem->m_Index, lpItem->m_Level)) <= 0)
	{
		return 0;
	}

	int MaxValue = this->GetInventoryMaxValue(lpObj);

	for (int n = INVENTORY_WEAR_SIZE; n < MaxValue; n++)
	{
		if (lpObj->Inventory[n].IsItem() != 0)
		{
			if (lpObj->Inventory[n].m_Index == lpItem->m_Index && lpObj->Inventory[n].m_Level == lpItem->m_Level && lpObj->Inventory[n].m_SocketOptionBonus == lpItem->m_SocketOptionBonus)
			{
				if (lpObj->Inventory[n].m_Durability < MaxStack)
				{
					int AddDur = (int)(MaxStack - lpObj->Inventory[n].m_Durability);

					AddDur = ((AddDur > lpItem->m_Durability) ? (int)lpItem->m_Durability : AddDur);

					lpItem->m_Durability -= AddDur;

					lpObj->Inventory[n].m_Durability += AddDur;

					if (lpObj->Inventory[n].m_Durability == MaxStack && (CreateItemIndex = gItemStack.GetCreateItemIndex(lpItem->m_Index, lpItem->m_Level)) != -1)
					{
						this->InventoryDelItem(lpObj->Index, n);
						this->GCItemDeleteSend(lpObj->Index, n, 1);
						GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, CreateItemIndex, (BYTE)lpItem->m_Level, 1, 0, 0, 0, lpObj->Index, 0, 0, 0, 0, 0, lpItem->m_SocketOptionBonus, 0);

						if (lpItem->m_Durability < 1)
						{
							return 1;
						}
					}
					else
					{
						this->GCItemDurSend(lpObj->Index, n, (BYTE)lpObj->Inventory[n].m_Durability, 0);
						if (lpItem->m_Durability < 1) { return 1; }
					}
				}
			}
		}
	}

	return 0;
}

bool CItemManager::InventoryAddItemStack(LPOBJ lpObj, int SourceSlot, int TargetSlot) // OK
{
	int MaxStack, CreateItemIndex;

	if (lpObj->Inventory[SourceSlot].IsItem() == 0 || lpObj->Inventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	if (lpObj->Inventory[SourceSlot].m_Index != lpObj->Inventory[TargetSlot].m_Index || lpObj->Inventory[SourceSlot].m_Level != lpObj->Inventory[TargetSlot].m_Level || lpObj->Inventory[SourceSlot].m_SocketOptionBonus != lpObj->Inventory[TargetSlot].m_SocketOptionBonus)
	{
		return 0;
	}

	if ((MaxStack = gItemStack.GetItemMaxStack(lpObj->Inventory[SourceSlot].m_Index, lpObj->Inventory[SourceSlot].m_Level)) <= 0)
	{
		return 0;
	}

	if (lpObj->Inventory[TargetSlot].m_Durability >= MaxStack)
	{
		return 0;
	}

	int AddDur = (int)(MaxStack - lpObj->Inventory[TargetSlot].m_Durability);

	AddDur = ((AddDur > lpObj->Inventory[SourceSlot].m_Durability) ? (int)lpObj->Inventory[SourceSlot].m_Durability : AddDur);

	lpObj->Inventory[SourceSlot].m_Durability -= AddDur;

	lpObj->Inventory[TargetSlot].m_Durability += AddDur;

	if (lpObj->Inventory[TargetSlot].m_Durability == MaxStack && (CreateItemIndex = gItemStack.GetCreateItemIndex(lpObj->Inventory[SourceSlot].m_Index, lpObj->Inventory[SourceSlot].m_Level)) != -1)
	{
		this->InventoryDelItem(lpObj->Index, TargetSlot);
		this->GCItemDeleteSend(lpObj->Index, TargetSlot, 1);
		GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, CreateItemIndex, (BYTE)lpObj->Inventory[SourceSlot].m_Level, 0, 0, 0, 0, lpObj->Index, 0, 0, 0, 0, 0, lpObj->Inventory[SourceSlot].m_SocketOptionBonus, 0);
	}
	else
	{
		this->GCItemDurSend(lpObj->Index, TargetSlot, (BYTE)lpObj->Inventory[TargetSlot].m_Durability, 0);
	}

	if (lpObj->Inventory[SourceSlot].m_Durability < 1)
	{
		this->InventoryDelItem(lpObj->Index, SourceSlot);
		this->GCItemDeleteSend(lpObj->Index, SourceSlot, 1);
	}

	return 1;
}

void CItemManager::TradeItemSet(int aIndex, int slot, BYTE type) // OK
{
	if (TRADE_RANGE(slot) == 0)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(gObj[aIndex].Trade[slot].m_Index, &ItemInfo) == 0)
	{
		return;
	}

	int x = slot % 8;
	int y = slot / 8;

	if ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 4)
	{
		return;
	}

	for (int sy = 0; sy < ItemInfo.Height; sy++)
	{
		for (int sx = 0; sx < ItemInfo.Width; sx++)
		{
			gObj[aIndex].TradeMap[(((sy + y) * 8) + (sx + x))] = type;
		}
	}
}

BYTE CItemManager::TradeRectCheck(int aIndex, int x, int y, int width, int height) // OK
{
	if ((x + width) > 8 || (y + height) > 4)
	{
		return 0xFF;
	}

	for (int sy = 0; sy < height; sy++)
	{
		for (int sx = 0; sx < width; sx++)
		{
			if (gObj[aIndex].TradeMap[(((sy + y) * 8) + (sx + x))] != 0xFF)
			{
				return 0xFF;
			}
		}
	}

	return ((y * 8) + x);
}

BYTE CItemManager::TradeInsertItem(int aIndex, CItem item) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (gObj[aIndex].TradeMap[((y * 8) + x)] == 0xFF)
			{
				BYTE slot = this->TradeRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height);

				if (slot != 0xFF)
				{
					gObj[aIndex].Trade[slot] = item;
					this->TradeItemSet(aIndex, slot, 1);
					return slot;
				}
			}
		}
	}

	return 0xFF;
}

BYTE CItemManager::TradeAddItem(int aIndex, CItem item, int slot) // OK
{
	if (TRADE_RANGE(slot) == 0)
	{
		return 0xFF;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Trade[slot].IsItem() != 0 || item.IsItem() == 0)
	{
		return 0xFF;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	int x = slot % 8;
	int y = slot / 8;

	if (this->TradeRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height) == 0xFF)
	{
		return 0xFF;
	}

	lpObj->Trade[slot] = item;
	this->TradeItemSet(aIndex, slot, 1);
	return slot;
}

void CItemManager::TradeDelItem(int aIndex, int slot) // OK
{
	this->TradeItemSet(aIndex, slot, 0xFF);
	gObj[aIndex].Trade[slot].Clear();
}

void CItemManager::WarehouseItemSet(int aIndex, int slot, BYTE type) // OK
{
	if (WAREHOUSE_RANGE(slot) == 0)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(gObj[aIndex].Warehouse[slot].m_Index, &ItemInfo) == 0)
	{
		return;
	}

	int x = slot % 8;
	int y = slot / 8;

	if (WAREHOUSE_MAIN_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 15))
	{
		return;
	}

	if (WAREHOUSE_EXT1_RANGE(slot) != 0 && ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 30))
	{
		return;
	}

	for (int sy = 0; sy < ItemInfo.Height; sy++)
	{
		for (int sx = 0; sx < ItemInfo.Width; sx++)
		{
			gObj[aIndex].WarehouseMap[(((sy + y) * 8) + (sx + x))] = type;
		}
	}
}

BYTE CItemManager::WarehouseRectCheck(int aIndex, int x, int y, int width, int height) // OK
{
	int slot = ((y * 8) + x);

	if (WAREHOUSE_MAIN_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 15))
	{
		return 0xFF;
	}

	if (WAREHOUSE_EXT1_RANGE(slot) != 0 && ((x + width) > 8 || (y + height) > 30))
	{
		return 0xFF;
	}

	for (int sy = 0; sy < height; sy++)
	{
		for (int sx = 0; sx < width; sx++)
		{
			if (gObj[aIndex].WarehouseMap[(((sy + y) * 8) + (sx + x))] != 0xFF)
			{
				return 0xFF;
			}
		}
	}

	return slot;
}

BYTE CItemManager::WarehouseInsertItem(int aIndex, CItem item) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	int MaxY = ((gObj[aIndex].ExtWarehouse == 0) ? WAREHOUSE_EXT1_SIZE : WAREHOUSE_SIZE) / 8;

	for (int y = 0; y < MaxY; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (gObj[aIndex].WarehouseMap[((y * 8) + x)] == 0xFF)
			{
				BYTE slot = this->WarehouseRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height);

				if (slot != 0xFF)
				{
					gObj[aIndex].Warehouse[slot] = item;
					this->WarehouseItemSet(aIndex, slot, 1);
					return slot;
				}
			}
		}
	}

	return 0xFF;
}

BYTE CItemManager::WarehouseAddItem(int aIndex, CItem item, int slot) // OK
{
	if (WAREHOUSE_RANGE(slot) == 0)
	{
		return 0xFF;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Warehouse[slot].IsItem() != 0 || item.IsItem() == 0)
	{
		return 0xFF;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	int x = slot % 8;
	int y = slot / 8;

	if (this->WarehouseRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height) == 0xFF)
	{
		return 0xFF;
	}

	lpObj->Warehouse[slot] = item;
	this->WarehouseItemSet(aIndex, slot, 1);
	return slot;
}

void CItemManager::WarehouseDelItem(int aIndex, int slot) // OK
{
	this->WarehouseItemSet(aIndex, slot, 0xFF);
	gObj[aIndex].Warehouse[slot].Clear();
}

void CItemManager::ChaosBoxItemSet(int aIndex, int slot, BYTE type) // OK
{
	if (CHAOS_BOX_RANGE(slot) == 0)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(gObj[aIndex].ChaosBox[slot].m_Index, &ItemInfo) == 0)
	{
		return;
	}

	int x = slot % 8;
	int y = slot / 8;

	if ((x + ItemInfo.Width) > 8 || (y + ItemInfo.Height) > 4)
	{
		return;
	}

	for (int sy = 0; sy < ItemInfo.Height; sy++)
	{
		for (int sx = 0; sx < ItemInfo.Width; sx++)
		{
			gObj[aIndex].ChaosBoxMap[(((sy + y) * 8) + (sx + x))] = type;
		}
	}
}

BYTE CItemManager::ChaosBoxRectCheck(int aIndex, int x, int y, int width, int height) // OK
{
	if ((x + width) > 8 || (y + height) > 4)
	{
		return 0xFF;
	}

	for (int sy = 0; sy < height; sy++)
	{
		for (int sx = 0; sx < width; sx++)
		{
			if (gObj[aIndex].ChaosBoxMap[(((sy + y) * 8) + (sx + x))] != 0xFF)
			{
				return 0xFF;
			}
		}
	}

	return ((y * 8) + x);
}

BYTE CItemManager::ChaosBoxInsertItem(int aIndex, CItem item) // OK
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (gObj[aIndex].ChaosBoxMap[((y * 8) + x)] == 0xFF)
			{
				BYTE slot = this->ChaosBoxRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height);

				if (slot != 0xFF)
				{
					gObj[aIndex].ChaosBox[slot] = item;
					this->ChaosBoxItemSet(aIndex, slot, 1);
					return slot;
				}
			}
		}
	}

	return 0xFF;
}

BYTE CItemManager::ChaosBoxAddItem(int aIndex, CItem item, int slot) // OK
{
	if (CHAOS_BOX_RANGE(slot) == 0)
	{
		return 0xFF;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->ChaosBox[slot].IsItem() != 0 || item.IsItem() == 0)
	{
		return 0xFF;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	int x = slot % 8;
	int y = slot / 8;

	if (this->ChaosBoxRectCheck(aIndex, x, y, ItemInfo.Width, ItemInfo.Height) == 0xFF)
	{
		return 0xFF;
	}

	lpObj->ChaosBox[slot] = item;
	this->ChaosBoxItemSet(aIndex, slot, 1);
	return slot;
}

void CItemManager::ChaosBoxDelItem(int aIndex, int slot) // OK
{
	this->ChaosBoxItemSet(aIndex, slot, 0xFF);
	gObj[aIndex].ChaosBox[slot].Clear();
}

#ifdef CHAOS_MACHINE_EXTENSION
void CItemManager::MixExpansionItemSet(int aIndex, int slot, BYTE type)
{
	if (MIX_INVENTORY_RANGE(slot) == 0)
	{
		return;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(gObj[aIndex].InventoryMix[slot].m_Index, &ItemInfo) == 0)
	{
		return;
	}

	gObj[aIndex].InventoryMixMap[slot] = type;
}

BYTE CItemManager::MixExpansionRectCheck(int aIndex, int x, int y, int width, int height)
{
	if ((x + width) > 8 || (y + height) > 15)
	{
		return 0xFF;
	}

	for (int sy = 0; sy < height; sy++)
	{
		for (int sx = 0; sx < width; sx++)
		{
			if (gObj[aIndex].InventoryMixMap[(((sy + y) * 8) + (sx + x))] != 0xFF)
			{
				return 0xFF;
			}
		}
	}

	return ((y * 8) + x);
}

BYTE CItemManager::MixExpansionInsertItem(int aIndex, CItem item)
{
	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	for (int y = 0; y < 15; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (gObj[aIndex].InventoryMixMap[((y * 8) + x)] == 0xFF)
			{
				BYTE slot = this->MixExpansionRectCheck(aIndex, x, y, 1, 1);

				if (slot != 0xFF)
				{
					gObj[aIndex].InventoryMix[slot] = item;
					this->MixExpansionItemSet(aIndex, slot, 1);
					return slot;
				}
			}
		}
	}
	return 0xFF;
}

BYTE CItemManager::MixExpansionAddItem(int aIndex, CItem item, int slot)
{
	if (MIX_INVENTORY_RANGE(slot) == 0)
	{
		return 0xFF;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->InventoryMix[slot].IsItem() != 0 || item.IsItem() == 0)
	{
		return 0xFF;
	}

	ITEM_INFO ItemInfo;

	if (this->GetInfo(item.m_Index, &ItemInfo) == 0)
	{
		return 0xFF;
	}

	int x = slot % 8;
	int y = slot / 8;

	if (this->MixExpansionRectCheck(aIndex, x, y, 1, 1) == 0xFF)
	{
		return 0xFF;
	}

	lpObj->InventoryMix[slot] = item;

	this->MixExpansionItemSet(aIndex, slot, 1);

	return slot;
}

void CItemManager::MixExpansionDelItem(int aIndex, int slot)
{
	this->MixExpansionItemSet(aIndex, slot, 0xFF);

	gObj[aIndex].InventoryMix[slot].Clear();
}
#endif // CHAOS_MACHINE_EXTENSION

void CItemManager::ItemByteConvert(BYTE* lpMsg, CItem item) // OK
{
	lpMsg[0] = item.m_Index & 0xFF;

	lpMsg[1] = 0;
	lpMsg[1] |= item.m_Level * 8;
	lpMsg[1] |= item.m_Option1 * 128;
	lpMsg[1] |= item.m_Option2 * 4;
	lpMsg[1] |= item.m_Option3 & 3;

	lpMsg[2] = (BYTE)item.m_Durability;

	lpMsg[3] = 0;
	lpMsg[3] |= (item.m_Index & 256) >> 1;
	lpMsg[3] |= ((item.m_Option3 > 3) ? 64 : 0);
	lpMsg[3] |= item.m_NewOption;

	lpMsg[4] = item.m_SetOption;

	lpMsg[5] = 0;
	lpMsg[5] |= (item.m_Index & GET_ITEM(15, 0)) >> 5;
	lpMsg[5] |= ((item.m_ItemOptionEx & 128) >> 4);
	lpMsg[5] |= ((item.m_IsPeriodicItem & 1) << 1);
	lpMsg[5] |= ((item.m_IsExpiredItem & 1) << 2);

	if (item.IsSocketItem() == 0 && item.IsPentagramItem() == 0 && item.IsPentagramJewel() == 0 && item.IsPentagramMithril() == 0 && item.IsMuunItem() == 0)
	{
		lpMsg[6] = item.m_JewelOfHarmonyOption;
	}
	else
	{
		lpMsg[6] = item.m_SocketOptionBonus;
	}

	memcpy(&lpMsg[7], item.m_SocketOption, MAX_SOCKET_OPTION);
}

void CItemManager::DBItemByteConvert(BYTE* lpMsg, CItem* lpItem) // OK
{
	if (lpItem->IsItem() == 0)
	{
		memset(lpMsg, 0xFF, 16);
		return;
	}

	if (lpItem->m_Index == GET_ITEM(13, 19) || lpItem->m_Index == GET_ITEM(14, 64))
	{
		memset(lpMsg, 0xFF, 16);
		return;
	}

	lpMsg[0] = lpItem->m_Index & 0xFF;

	lpMsg[1] = 0;
	lpMsg[1] |= lpItem->m_Level * 8;
	lpMsg[1] |= lpItem->m_Option1 * 128;
	lpMsg[1] |= lpItem->m_Option2 * 4;
	lpMsg[1] |= lpItem->m_Option3 & 3;

	lpMsg[2] = (BYTE)lpItem->m_Durability;

	lpMsg[3] = SET_NUMBERHB(SET_NUMBERHW(lpItem->m_Serial));
	lpMsg[4] = SET_NUMBERLB(SET_NUMBERHW(lpItem->m_Serial));
	lpMsg[5] = SET_NUMBERHB(SET_NUMBERLW(lpItem->m_Serial));
	lpMsg[6] = SET_NUMBERLB(SET_NUMBERLW(lpItem->m_Serial));

	lpMsg[7] = 0;
	lpMsg[7] |= (lpItem->m_Index & 256) >> 1;
	lpMsg[7] |= ((lpItem->m_Option3 > 3) ? 64 : 0);
	lpMsg[7] |= lpItem->m_NewOption;

	lpMsg[8] = 0;
	lpMsg[8] |= lpItem->m_SetOption & 15;

	lpMsg[9] = 0;
	lpMsg[9] |= (lpItem->m_Index & GET_ITEM(15, 0)) >> 5;
	lpMsg[9] |= ((lpItem->m_ItemOptionEx & 128) >> 4);
	lpMsg[9] |= ((lpItem->m_IsPeriodicItem & 1) << 1);
	lpMsg[9] |= ((lpItem->m_IsExpiredItem & 1) << 2);

	if (lpItem->IsSocketItem() == 0 && lpItem->IsPentagramItem() == 0 && lpItem->IsPentagramJewel() == 0 && lpItem->IsPentagramMithril() == 0 && lpItem->IsMuunItem() == 0)
	{
		lpMsg[10] = lpItem->m_JewelOfHarmonyOption;
	}
	else
	{
		lpMsg[10] = lpItem->m_SocketOptionBonus;
	}

	memcpy(&lpMsg[11], lpItem->m_SocketOption, MAX_SOCKET_OPTION);
}

bool CItemManager::ConvertItemByte(CItem* lpItem, BYTE* lpMsg) // OK
{
	if (lpMsg[0] == 0xFF && (lpMsg[7] & 0x80) == 0x80 && (lpMsg[9] & 0xF0) == 0xF0)
	{
		return 0;
	}

	lpItem->Clear();

	if (isArrayZeroed(lpMsg) == true)
	{
		return 0;
	}

	lpItem->m_Index = lpMsg[0] | ((lpMsg[9] & 0xF0) * 32) | ((lpMsg[7] & 0x80) * 2);

	lpItem->m_Option1 = (lpMsg[1] / 128) & 1;

	lpItem->m_Option2 = (lpMsg[1] / 4) & 1;

	lpItem->m_Option3 = (lpMsg[1] & 3) + ((lpMsg[7] & 64) / 16);

	lpItem->m_Level = (lpMsg[1] / 8) & 15;

	lpItem->m_Durability = lpMsg[2];

	lpItem->m_Durability = ((lpItem->m_Index == GET_ITEM(13, 70)) ? 2 : lpItem->m_Durability);

	int MaxStack = gItemStack.GetItemMaxStack(lpItem->m_Index, lpItem->m_Level);

	lpItem->m_Durability = ((MaxStack == 0) ? lpItem->m_Durability : ((lpItem->m_Durability > MaxStack) ? MaxStack : lpItem->m_Durability));

	lpItem->m_Serial = MAKE_NUMBERDW(MAKE_NUMBERW(lpMsg[3], lpMsg[4]), MAKE_NUMBERW(lpMsg[5], lpMsg[6]));

	lpItem->m_NewOption = lpMsg[7] & 63;

	lpItem->m_SetOption = lpMsg[8] & 15;

	lpItem->m_ItemOptionEx = (lpMsg[9] & 8) * 16;

	lpItem->m_IsPeriodicItem = ((lpMsg[9] & 2) / 2) & 1;

	lpItem->m_IsExpiredItem = ((lpMsg[9] & 4) / 4) & 1;

	if (lpMsg[11] == 0 && lpMsg[12] == 0 && lpMsg[13] == 0 && lpMsg[14] == 0 && lpMsg[15] == 0)
	{
		memset(lpItem->m_SocketOption, 0xFF, MAX_SOCKET_OPTION);
	}
	else
	{
		memcpy(lpItem->m_SocketOption, &lpMsg[11], MAX_SOCKET_OPTION);
	}

	if (lpItem->IsSocketItem() == 0 && lpItem->IsPentagramItem() == 0 && lpItem->IsPentagramJewel() == 0 && lpItem->IsPentagramMithril() == 0 && lpItem->IsMuunItem() == 0)
	{
		lpItem->m_JewelOfHarmonyOption = lpMsg[10];

		lpItem->m_SocketOptionBonus = 0xFF;
	}
	else
	{
		lpItem->m_JewelOfHarmonyOption = 0;

		lpItem->m_SocketOptionBonus = lpMsg[10];
	}

	lpItem->Convert(lpItem->m_Index, lpItem->m_Option1, lpItem->m_Option2, lpItem->m_Option3, lpItem->m_NewOption, lpItem->m_SetOption, lpItem->m_JewelOfHarmonyOption, lpItem->m_ItemOptionEx, lpItem->m_SocketOption, lpItem->m_SocketOptionBonus);

	return 1;
}

bool CItemManager::IsValidItem(LPOBJ lpObj, CItem* lpItem) // OK
{
	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if (lpItem->IsPentagramItem() == 0 && lpItem->m_Durability == 0)
	{
		return 0;
	}

	if (this->CheckItemRequireLevel(lpObj, lpItem) == 0)
	{
		return 0;
	}

	if (this->CheckItemRequireStrength(lpObj, lpItem) == 0)
	{
		return 0;
	}

	if (this->CheckItemRequireDexterity(lpObj, lpItem) == 0)
	{
		return 0;
	}

	if (this->CheckItemRequireVitality(lpObj, lpItem) == 0)
	{
		return 0;
	}

	if (this->CheckItemRequireEnergy(lpObj, lpItem) == 0)
	{
		return 0;
	}

	if (this->CheckItemRequireLeadership(lpObj, lpItem) == 0)
	{
		return 0;
	}

	if (this->CheckItemRequireClass(lpObj, lpItem->m_Index) == 0)
	{
		return 0;
	}

	return 1;
}

void CItemManager::UpdateInventoryViewport(int aIndex, int slot) // OK
{
	if (INVENTORY_WEAR_RANGE(slot) == 0)
	{
		return;
	}

	if (slot == 10 || slot == 11)
	{
		if (gSkillManager.SkillChangeUse(aIndex) != 0)
		{
			gObjViewportListProtocolCreate(&gObj[aIndex]);
			gObjectManager.CharacterUpdateMapEffect(&gObj[aIndex]);
		}
	}

	gObjectManager.CharacterMakePreviewCharSet(aIndex);

	this->GCItemChangeSend(aIndex, slot);
}

void CItemManager::DeleteInventoryItemCount(LPOBJ lpObj, int index, int level, int count) // OK
{
	int MaxValue = this->GetInventoryMaxValue(lpObj);

	for (int n = 0; (n < MaxValue && count > 0); n++)
	{
		if (lpObj->Inventory[n].IsItem() != 0)
		{
			if (lpObj->Inventory[n].m_Index == index && (level == -1 || lpObj->Inventory[n].m_Level == level))
			{
				if (gItemStack.GetItemMaxStack(lpObj->Inventory[n].m_Index, lpObj->Inventory[n].m_Level) <= 0)
				{
					this->InventoryDelItem(lpObj->Index, n);
					this->GCItemDeleteSend(lpObj->Index, n, 1);
					count--;
				}
				else
				{
					if (lpObj->Inventory[n].m_Durability > count)
					{
						lpObj->Inventory[n].m_Durability -= count;
						this->GCItemDurSend(lpObj->Index, n, (BYTE)lpObj->Inventory[n].m_Durability, 0);
						count = 0;
					}
					else
					{
						count -= (int)lpObj->Inventory[n].m_Durability;
						this->InventoryDelItem(lpObj->Index, n);
						this->GCItemDeleteSend(lpObj->Index, n, 1);
					}
				}
			}
		}
	}
}

void CItemManager::DecreaseItemDur(LPOBJ lpObj, int slot, int dur) // OK
{
	if (INVENTORY_FULL_RANGE(slot) == 0)
	{
		return;
	}

	if (lpObj->Inventory[slot].IsItem() == 0)
	{
		return;
	}

	if (lpObj->Inventory[slot].m_Durability > dur)
	{
		lpObj->Inventory[slot].m_Durability -= dur;
		this->GCItemDurSend(lpObj->Index, slot, (BYTE)lpObj->Inventory[slot].m_Durability, 0);
	}
	else
	{
		this->InventoryDelItem(lpObj->Index, slot);
		this->GCItemDeleteSend(lpObj->Index, slot, 1);
	}
}

int CItemManager::RepairItem(LPOBJ lpObj, CItem* lpItem, int slot, int type) // OK
{
	if (lpItem->m_Slot == 0xFF || INVENTORY_WEAR_RANGE(lpItem->m_Slot) == 0)
	{
		return 0;
	}

	if (lpItem->m_Index == GET_ITEM(0, 41)) // Minning Axe
	{
		return 0;
	}

	if (gItemStack.GetItemMaxStack(lpItem->m_Index, lpItem->m_Level) > 0)
	{
		return 0;
	}

	if (lpItem->m_Slot == 8 && lpItem->m_Index != GET_ITEM(13, 4))
	{
		return 0;
	}

	if (lpItem->IsLuckyItem() != 0 || lpItem->m_IsPeriodicItem != 0 || lpItem->IsPentagramItem() != 0)
	{
		return 0;
	}

	//if(lpItem->m_Index == GET_ITEM(4,7) || lpItem->m_Index == GET_ITEM(4,15)) // Bolt,Arrow
	//{
	//	return 0;
	//}

	if (lpItem->m_Index >= GET_ITEM(12, 130) && lpItem->m_Index <= GET_ITEM(12, 135)) // Mini Wings
	{
		return 0;
	}

	if (lpObj->Interface.type != INTERFACE_TRAINER && (lpItem->m_Index == GET_ITEM(13, 4) || lpItem->m_Index == GET_ITEM(13, 5)))
	{
		return 0;
	}

	if (lpItem->m_Index == GET_ITEM(13, 20) && (lpItem->m_Level >= 0 && lpItem->m_Level <= 2)) // Wizard's Ring,Starter Ring
	{
		return 0;
	}

	if (lpItem->m_Index == GET_ITEM(13, 10) || (lpItem->m_Index >= GET_ITEM(13, 38) && lpItem->m_Index <= GET_ITEM(13, 42)) || lpItem->m_Index == GET_ITEM(13, 68) || lpItem->m_Index == GET_ITEM(13, 76) || lpItem->m_Index == GET_ITEM(13, 122)) // Rings
	{
		return 0;
	}

	int money = this->GetItemRepairMoney(lpItem, type);

	money = ((money < 0) ? 0 : money);

	if (lpObj->Money < ((DWORD)money))
	{
		return 0;
	}

	lpObj->Money -= money;

	lpItem->Convert(lpItem->m_Index, lpItem->m_Option1, lpItem->m_Option2, lpItem->m_Option3, lpItem->m_NewOption, lpItem->m_SetOption, lpItem->m_JewelOfHarmonyOption, lpItem->m_ItemOptionEx, lpItem->m_SocketOption, lpItem->m_SocketOptionBonus);

	lpItem->m_Durability = lpItem->m_BaseDurability;

	if (INVENTORY_WEAR_RANGE(slot) != 0 && lpItem->CheckDurabilityState() != 0)
	{
		gObjectManager.CharacterCalcAttribute(lpObj->Index);
	}

	this->GCItemDurSend(lpObj->Index, slot, (BYTE)lpItem->m_Durability, 0);

	if (lpItem->m_Index == GET_ITEM(13, 4))
	{
		GCPetItemInfoSend(lpObj->Index, 1, 0, slot, lpItem->m_PetItemLevel, lpItem->m_PetItemExp, (BYTE)lpItem->m_Durability);
	}
	else if (lpItem->m_Index == GET_ITEM(13, 5))
	{
		GCPetItemInfoSend(lpObj->Index, 0, 0, slot, lpItem->m_PetItemLevel, lpItem->m_PetItemExp, (BYTE)lpItem->m_Durability);
	}
#ifdef PET_SYSTEM_GAIN_EXP
	else if (lpItem->Kind3 == ItemKind3::PET_GAIN_EXPERIENCE)
	{
		GCPetItemInfoSend(lpObj->Index, 1, 0, slot, lpItem->m_PetItemLevel, lpItem->m_PetItemExp, (BYTE)lpItem->m_Durability);
	}
#endif // PET_SYSTEM_GAIN_EXP

	return lpObj->Money;
}

BYTE CItemManager::MoveItemToInventoryFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (SourceSlot == TargetSlot)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToInventory(lpObj, &lpObj->Inventory[SourceSlot], TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->InventoryAddItemStack(lpObj, SourceSlot, TargetSlot) != 0)
	{
		return 0xFF;
	}

	BYTE map[INVENTORY_SIZE];

	memcpy(map, lpObj->InventoryMap, INVENTORY_SIZE);

	this->InventoryItemSet(lpObj->Index, SourceSlot, 0xFF);

	if (this->InventoryAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		memcpy(lpObj->InventoryMap, map, INVENTORY_SIZE);
		return 0xFF;
	}

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	this->InventoryItemSet(lpObj->Index, TargetSlot, 1);

	this->UpdateInventoryViewport(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, TargetSlot);

	return TargetFlag;
}

BYTE CItemManager::MoveItemToInventoryFromTrade(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (TRADE_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Trade[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Trade[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToInventory(lpObj, &lpObj->Trade[SourceSlot], TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->InventoryAddItem(lpObj->Index, lpObj->Trade[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->TradeDelItem(lpObj->Index, SourceSlot);

	if (OBJECT_RANGE(lpObj->TargetNumber) != 0)
	{
		lpObj->TradeOk = 0;
		lpObj->TradeOkTime = GetTickCount();
		gTrade.GCTradeOkButtonSend(lpObj->Index, 0);
		gObj[lpObj->TargetNumber].TradeOk = 0;
		gObj[lpObj->TargetNumber].TradeOkTime = GetTickCount();
		gTrade.GCTradeOkButtonSend(lpObj->TargetNumber, 2);
		gTrade.GCTradeItemDelSend(lpObj->TargetNumber, SourceSlot);
	}

	this->UpdateInventoryViewport(lpObj->Index, TargetSlot);

	return TargetFlag;
}

BYTE CItemManager::MoveItemToInventoryFromWarehouse(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (WAREHOUSE_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (WAREHOUSE_EXT1_RANGE(SourceSlot) != 0 && lpObj->ExtWarehouse < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	int tax = gWarehouse.GetWarehouseTaxMoney(lpObj->Level, lpObj->WarehousePW);

	if (lpObj->Money < ((DWORD)tax) && lpObj->WarehouseMoney < tax)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Warehouse[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Warehouse[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToInventory(lpObj, &lpObj->Warehouse[SourceSlot], TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->InventoryAddItem(lpObj->Index, lpObj->Warehouse[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->WarehouseDelItem(lpObj->Index, SourceSlot);

	if (lpObj->Money >= ((DWORD)tax))
	{
		lpObj->Money -= tax;
		gWarehouse.GCWarehouseMoneySend(lpObj->Index, 1, lpObj->Money, lpObj->WarehouseMoney);
	}
	else if (lpObj->WarehouseMoney >= tax)
	{
		lpObj->WarehouseMoney -= tax;
		gWarehouse.GCWarehouseMoneySend(lpObj->Index, 1, lpObj->Money, lpObj->WarehouseMoney);
	}

	this->UpdateInventoryViewport(lpObj->Index, TargetSlot);

#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.MovePentagramToInventoryFromWarehouse(lpObj, &lpObj->Inventory[TargetSlot]);

#endif

	return TargetFlag;
}

BYTE CItemManager::MoveItemToInventoryFromChaosBox(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (CHAOS_BOX_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->ChaosBox[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->ChaosBox[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToInventory(lpObj, &lpObj->ChaosBox[SourceSlot], TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->InventoryAddItem(lpObj->Index, lpObj->ChaosBox[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->ChaosBoxDelItem(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, TargetSlot);

	return TargetFlag;
}

BYTE CItemManager::MoveItemToInventoryFromPersonalShop(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (INVENTORY_SHOP_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	//itemLock
	if (lpObj->Lock > 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		return 0xFF;
	}

	if (lpObj->TradeDuel != 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToInventory(lpObj, &lpObj->Inventory[SourceSlot], TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->InventoryAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, TargetSlot);

#if(GAMESERVER_UPDATE>=802)

	gPersonalShop.GDPShopItemValueDeleteSaveSend(lpObj->Index, SourceSlot);

#endif

	return TargetFlag;
}

BYTE CItemManager::MoveItemToTradeFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (TRADE_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToTrade(lpObj, &lpObj->Inventory[SourceSlot], TargetFlag) == 0)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->TradeAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	BYTE ItemInfo[MAX_ITEM_INFO];

	this->ItemByteConvert(ItemInfo, lpObj->Trade[TargetSlot]);

	gTrade.GCTradeItemAddSend(lpObj->TargetNumber, TargetSlot, ItemInfo);

	this->UpdateInventoryViewport(lpObj->Index, SourceSlot);

#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.GCPentagramJewelTradeInfoSend(lpObj->Index, lpObj->TargetNumber, &lpObj->Trade[TargetSlot]);

#endif

	return 1;
}

BYTE CItemManager::MoveItemToTradeFromTrade(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (TRADE_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (TRADE_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (SourceSlot == TargetSlot)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Trade[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Trade[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	BYTE map[TRADE_SIZE];

	memcpy(map, lpObj->TradeMap, TRADE_SIZE);

	this->TradeItemSet(lpObj->Index, SourceSlot, 0xFF);

	if (this->TradeAddItem(lpObj->Index, lpObj->Trade[SourceSlot], TargetSlot) == 0xFF)
	{
		memcpy(lpObj->TradeMap, map, TRADE_SIZE);
		return 0xFF;
	}

	this->TradeDelItem(lpObj->Index, SourceSlot);

	this->TradeItemSet(lpObj->Index, TargetSlot, 1);

	gTrade.GCTradeItemDelSend(lpObj->TargetNumber, SourceSlot);

	BYTE ItemInfo[MAX_ITEM_INFO];

	this->ItemByteConvert(ItemInfo, lpObj->Trade[TargetSlot]);

	gTrade.GCTradeItemAddSend(lpObj->TargetNumber, TargetSlot, ItemInfo);

	return 1;
}

BYTE CItemManager::MoveItemToTradeFromEventInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
#if(GAMESERVER_UPDATE>=802)

	if (EVENT_INVENTORY_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (TRADE_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToTrade(lpObj, &lpObj->EventInventory[SourceSlot], TargetFlag) == 0)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->EventInventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->EventInventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->TradeAddItem(lpObj->Index, lpObj->EventInventory[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	gEventInventory.EventInventoryDelItem(lpObj->Index, SourceSlot);

	BYTE ItemInfo[MAX_ITEM_INFO];

	this->ItemByteConvert(ItemInfo, lpObj->Trade[TargetSlot]);

	gTrade.GCTradeItemAddSend(lpObj->TargetNumber, TargetSlot, ItemInfo);

	return 1;

#else

	return 0xFF;

#endif
}

BYTE CItemManager::MoveItemToWarehouseFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (WAREHOUSE_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (WAREHOUSE_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtWarehouse < 1)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToVault(lpObj, &lpObj->Inventory[SourceSlot], TargetFlag) == 0)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->WarehouseAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, SourceSlot);

#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.MovePentagramToWarehouseFromInventory(lpObj, &lpObj->Warehouse[TargetSlot]);

#endif

	return 2;
}

BYTE CItemManager::MoveItemToWarehouseFromWarehouse(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (WAREHOUSE_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (WAREHOUSE_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (WAREHOUSE_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtWarehouse < 1)
	{
		return 0xFF;
	}

	if (SourceSlot == TargetSlot)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Warehouse[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Warehouse[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	BYTE map[WAREHOUSE_SIZE];

	memcpy(map, lpObj->WarehouseMap, WAREHOUSE_SIZE);

	this->WarehouseItemSet(lpObj->Index, SourceSlot, 0xFF);

	if (this->WarehouseAddItem(lpObj->Index, lpObj->Warehouse[SourceSlot], TargetSlot) == 0xFF)
	{
		memcpy(lpObj->WarehouseMap, map, WAREHOUSE_SIZE);
		return 0xFF;
	}

	this->WarehouseDelItem(lpObj->Index, SourceSlot);

	this->WarehouseItemSet(lpObj->Index, TargetSlot, 1);

	return 2;
}

BYTE CItemManager::MoveItemToChaosBoxFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (CHAOS_BOX_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToChaos(lpObj, &lpObj->Inventory[SourceSlot], TargetFlag) == 0)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->ChaosBoxAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, SourceSlot);

	return TargetFlag;
}

BYTE CItemManager::MoveItemToChaosBoxFromChaosBox(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (CHAOS_BOX_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (CHAOS_BOX_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (SourceSlot == TargetSlot)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->ChaosBox[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->ChaosBox[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	BYTE map[CHAOS_BOX_SIZE];

	memcpy(map, lpObj->ChaosBoxMap, CHAOS_BOX_SIZE);

	this->ChaosBoxItemSet(lpObj->Index, SourceSlot, 0xFF);

	if (this->ChaosBoxAddItem(lpObj->Index, lpObj->ChaosBox[SourceSlot], TargetSlot) == 0xFF)
	{
		memcpy(lpObj->ChaosBoxMap, map, CHAOS_BOX_SIZE);
		return 0xFF;
	}

	this->ChaosBoxDelItem(lpObj->Index, SourceSlot);

	this->ChaosBoxItemSet(lpObj->Index, TargetSlot, 1);

	return TargetFlag;
}

BYTE CItemManager::MoveItemToPersonalShopFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (INVENTORY_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_SHOP_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToTrade(lpObj, &lpObj->Inventory[SourceSlot], TargetFlag) == 0)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->InventoryAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, SourceSlot);

#if(GAMESERVER_UPDATE>=802)

	gPersonalShop.GDPShopItemValueInsertSaveSend(lpObj->Index, TargetSlot, &lpObj->Inventory[TargetSlot]);

#endif

	return 0;
}

BYTE CItemManager::MoveItemToPersonalShopFromPersonalShop(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (INVENTORY_SHOP_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_SHOP_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (SourceSlot == TargetSlot)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	BYTE map[INVENTORY_SIZE];

	memcpy(map, lpObj->InventoryMap, INVENTORY_SIZE);

	this->InventoryItemSet(lpObj->Index, SourceSlot, 0xFF);

	if (this->InventoryAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		memcpy(lpObj->InventoryMap, map, INVENTORY_SIZE);
		return 0xFF;
	}

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	this->InventoryItemSet(lpObj->Index, TargetSlot, 1);

#if(GAMESERVER_UPDATE>=802)

	gPersonalShop.GDPShopItemValueDeleteSaveSend(lpObj->Index, SourceSlot);

	gPersonalShop.GDPShopItemValueInsertSaveSend(lpObj->Index, TargetSlot, &lpObj->Inventory[TargetSlot]);

#endif

	return 0;
}

BYTE CItemManager::MoveItemToEventInventoryFromTrade(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
	if (TRADE_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (EVENT_INVENTORY_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Trade[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Trade[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gEventInventory.EventInventoryAddItem(lpObj->Index, lpObj->Trade[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->TradeDelItem(lpObj->Index, SourceSlot);

	if (OBJECT_RANGE(lpObj->TargetNumber) != 0)
	{
		lpObj->TradeOk = 0;
		lpObj->TradeOkTime = GetTickCount();
		gTrade.GCTradeOkButtonSend(lpObj->Index, 0);
		gObj[lpObj->TargetNumber].TradeOk = 0;
		gObj[lpObj->TargetNumber].TradeOkTime = GetTickCount();
		gTrade.GCTradeOkButtonSend(lpObj->TargetNumber, 2);
		gTrade.GCTradeItemDelSend(lpObj->TargetNumber, SourceSlot);
	}

	return TargetFlag;
}

BYTE CItemManager::MoveItemToEventInventoryFromEventInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
#if(GAMESERVER_UPDATE>=802)

	if (EVENT_INVENTORY_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (EVENT_INVENTORY_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (SourceSlot == TargetSlot)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->EventInventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->EventInventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gEventInventory.EventInventoryAddItemStack(lpObj, SourceSlot, TargetSlot) != 0)
	{
		return 0xFF;
	}

	BYTE map[EVENT_INVENTORY_SIZE];

	memcpy(map, lpObj->EventInventoryMap, EVENT_INVENTORY_SIZE);

	gEventInventory.EventInventoryItemSet(lpObj->Index, SourceSlot, 0xFF);

	if (gEventInventory.EventInventoryAddItem(lpObj->Index, lpObj->EventInventory[SourceSlot], TargetSlot) == 0xFF)
	{
		memcpy(lpObj->EventInventoryMap, map, EVENT_INVENTORY_SIZE);
		return 0xFF;
	}

	gEventInventory.EventInventoryDelItem(lpObj->Index, SourceSlot);

	gEventInventory.EventInventoryItemSet(lpObj->Index, TargetSlot, 1);

	return TargetFlag;

#else

	return 0xFF;

#endif
}

BYTE CItemManager::MoveItemToMuunInventoryFromMuunInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag) // OK
{
#if(GAMESERVER_UPDATE>=803)

	if (MUUN_INVENTORY_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (MUUN_INVENTORY_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (SourceSlot == TargetSlot)
	{
		return 0xFF;
	}

	if (gMuunSystem.CheckItemMoveToMuunInventory(lpObj, &lpObj->MuunInventory[SourceSlot], TargetSlot) == 0)
	{
		return 0xFF;
	}

	BYTE map[MUUN_INVENTORY_SIZE];

	memcpy(map, lpObj->MuunInventoryMap, MUUN_INVENTORY_SIZE);

	gMuunSystem.MuunInventoryItemSet(lpObj->Index, SourceSlot, 0xFF);

	if (gMuunSystem.MuunInventoryAddItem(lpObj->Index, lpObj->MuunInventory[SourceSlot], TargetSlot) == 0xFF)
	{
		memcpy(lpObj->MuunInventoryMap, map, MUUN_INVENTORY_SIZE);
		return 0xFF;
	}

	gMuunSystem.MuunInventoryDelItem(lpObj->Index, SourceSlot);

	gMuunSystem.MuunInventoryItemSet(lpObj->Index, TargetSlot, 1);

	if (MUUN_INVENTORY_WEAR_RANGE(SourceSlot) != 0)
	{
		gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
		gMuunSystem.GCMuunItemChangeSend(lpObj->Index, SourceSlot);
		gMuunSystem.GCMuunItemStatusSend(lpObj->Index, SourceSlot, 0);
		lpObj->MuunItemStatus[SourceSlot] = 0;
	}

	if (MUUN_INVENTORY_WEAR_RANGE(TargetSlot) != 0)
	{
		gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
		gMuunSystem.GCMuunItemChangeSend(lpObj->Index, TargetSlot);
	}

	return TargetFlag;

#else

	return 0xFF;

#endif
}

#ifdef CHAOS_MACHINE_EXTENSION
BYTE CItemManager::MoveItemToMixExpansionFromInventory(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag)
{
	if (lpObj->m_iMixIndex == 0xFF)
	{
		return 0xFF;
	}

	if (INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (CHAOS_BOX_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(SourceSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	//gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "revisando CheckItemMoveToMixExpansion: [SourceSlot: %d][TargetSlot: %d]", SourceSlot, TargetSlot);
	
	if (this->CheckItemMoveToMixExpansion(lpObj, &lpObj->Inventory[SourceSlot], TargetFlag) == 0)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->Inventory[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	//gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "revisando MixExpansionAddItem: [SourceSlot: %d][TargetSlot: %d]", SourceSlot, TargetSlot);

	if (this->MixExpansionAddItem(lpObj->Index, lpObj->Inventory[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	//gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, "si paso MixExpansionAddItem: [SourceSlot: %d][TargetSlot: %d]", SourceSlot, TargetSlot);

	this->InventoryDelItem(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, SourceSlot);

	return TargetFlag;
}

BYTE CItemManager::MoveItemToInventoryFromMixExpansion(LPOBJ lpObj, BYTE SourceSlot, BYTE TargetSlot, BYTE TargetFlag)
{
	if (lpObj->m_iMixIndex == 0xFF)
	{
		return 0xFF;
	}

	if (MIX_INVENTORY_RANGE(SourceSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT1_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 1)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT2_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 2)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT3_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 3)
	{
		return 0xFF;
	}

	if (INVENTORY_EXT4_RANGE(TargetSlot) != 0 && lpObj->ExtInventory < 4)
	{
		return 0xFF;
	}

	if (gObjInventorySearchSerialNumber(lpObj, lpObj->InventoryMix[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (gObjWarehouseSearchSerialNumber(lpObj, lpObj->InventoryMix[SourceSlot].m_Serial) == 0)
	{
		return 0xFF;
	}

	if (this->CheckItemMoveToInventory(lpObj, &lpObj->InventoryMix[SourceSlot], TargetSlot) == 0)
	{
		return 0xFF;
	}

	if (this->InventoryAddItem(lpObj->Index, lpObj->InventoryMix[SourceSlot], TargetSlot) == 0xFF)
	{
		return 0xFF;
	}

	this->MixExpansionDelItem(lpObj->Index, SourceSlot);

	this->UpdateInventoryViewport(lpObj->Index, TargetSlot);

	return TargetFlag;
}
#endif // CHAOS_MACHINE_EXTENSION

#ifdef INVENTORY_EXT_BUY
void CItemManager::CGInventoryBuyRecv(PMSG_ITEM_BUY_RECV* lpMsg, int aIndex)
{
	if (gServerInfo.m_SwitchBuyInventoryExt == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (lpObj->ExtInventory >= 4)
	{
		return;
	}

	int Inven = lpObj->ExtInventory;

	if (lpObj->DieRegen != 0)
	{
		if (gServerInfo.m_InventoryExtMsg[1] != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(gServerInfo.m_InventoryExtMsg[1]));
		}
		return;
	}

	if (lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		if (gServerInfo.m_InventoryExtMsg[1] != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(gServerInfo.m_InventoryExtMsg[1]));
		}
		return;
	}

	if (lpObj->Transaction == 1 && lpObj->TradeDuel == 0)
	{
		if (gServerInfo.m_InventoryExtMsg[1] != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(gServerInfo.m_InventoryExtMsg[1]));
		}
		return;
	}

	if (Inven != lpMsg->slot)
	{
		if (gServerInfo.m_InventoryExtMsg[1] != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(gServerInfo.m_InventoryExtMsg[1]));
		}
		return;
	}

	if (gServerInfo.m_PriceInventoryExt[Inven] > (DWORD)lpObj->Coin1)
	{
		if (gServerInfo.m_InventoryExtMsg[2] != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(gServerInfo.m_InventoryExtMsg[2]));
		}
		return;
	}

	if (gObjectManager.CharacterUseInventoryExpansion(lpObj, 0))
	{
		GDSetCoinSend(lpObj->Index, -((int)gServerInfo.m_PriceInventoryExt[Inven]), 0, 0, "Buy Inventory Ext");

		if (gServerInfo.m_InventoryExtMsg[0] != 0)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(gServerInfo.m_InventoryExtMsg[0]));
		}

		GCFireworksSend(lpObj, lpObj->X, lpObj->Y);
	}
}
#endif // INVENTORY_EXT_BUY

bool CItemManager::runtime_action_item(LPOBJ lpObj, CItem* lpItem, BYTE iSourceIndex, BYTE iTargetIndex)
{
	bool success_delete = true;

	if (gItemStack.GetItemMaxStack(lpItem->m_Index, lpItem->m_Level) != 0)
	{
		if (lpItem->m_Durability >= 1)
		{
			success_delete = false;
			this->DecreaseItemDur(lpObj, iSourceIndex, 1);
		}
	}

	if (success_delete)
	{
		this->InventoryDelItem(lpObj->Index, iSourceIndex);

		if (iTargetIndex != 0xFF)
		{
			this->GCItemDeleteSend(lpObj->Index, iSourceIndex, 1);
		}
	}

	if (iTargetIndex != 0xFF)
	{
		this->GCItemModifySend(lpObj->Index, iTargetIndex);
	}

	return success_delete;
}

void CItemManager::CGItemGetRecv(PMSG_ITEM_GET_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_ITEM_GET_SEND pMsg;

	pMsg.header.setE(0x22, sizeof(pMsg));

	pMsg.result = 0xFF;

	memset(pMsg.ItemInfo, 0, sizeof(pMsg.ItemInfo));

	if (lpObj->DieRegen != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

#if(GAMESERVER_UPDATE>=402)

	if (gDuel.GetDuelArenaBySpectator(aIndex) != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

#endif

	gObjFixInventoryPointer(aIndex);

	if (lpObj->Transaction == 1)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	int index = MAKE_NUMBERW(lpMsg->index[0], lpMsg->index[1]);

	if (MAP_ITEM_RANGE(index) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (MAP_RANGE(lpObj->Map) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (gMap[lpObj->Map].CheckItemGive(aIndex, index) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	CMapItem* lpItem = &gMap[lpObj->Map].m_Item[index];

	if (lpItem->IsEventItem() != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpItem->IsMuunItem() != 0 || lpItem->IsMuunUtil() != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (gQuestObjective.CheckQuestObjectiveItemCount(lpObj, lpItem->m_Index, lpItem->m_Level) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if ((lpItem->m_Index == GET_ITEM(13, 20) && lpItem->m_Level == 0) || lpItem->m_Index == GET_ITEM(13, 38)) // Rings
	{
		if (this->GetInventoryItemCount(lpObj, lpItem->m_Index, lpItem->m_Level) > 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}
	}

	if (lpItem->m_Index == GET_ITEM(14, 15)) // Money
	{
		gMap[lpObj->Map].ItemGive(aIndex, index);

		if (gObjCheckMaxMoney(aIndex, lpItem->m_BuyMoney) == 0)
		{
			lpObj->Money = MAX_MONEY;
		}
		else
		{
			lpObj->Money += lpItem->m_BuyMoney;
		}

		pMsg.result = 0xFE;

		pMsg.ItemInfo[0] = SET_NUMBERHB(SET_NUMBERHW(lpObj->Money));
		pMsg.ItemInfo[1] = SET_NUMBERLB(SET_NUMBERHW(lpObj->Money));
		pMsg.ItemInfo[2] = SET_NUMBERHB(SET_NUMBERLW(lpObj->Money));
		pMsg.ItemInfo[3] = SET_NUMBERLB(SET_NUMBERLW(lpObj->Money));

		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	CItem item = (*lpItem);

	if (this->InventoryInsertItemStack(lpObj, &item) != 0)
	{
		gMap[lpObj->Map].ItemGive(aIndex, index);

		pMsg.result = 0xFD;
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	pMsg.result = this->InventoryInsertItem(aIndex, item);

	if (pMsg.result == 0xFF)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	gMap[lpObj->Map].ItemGive(aIndex, index);

	this->ItemByteConvert(pMsg.ItemInfo, item);

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

	GCPartyItemInfoSend(aIndex, &item);

	if (item.m_IsPeriodicItem != 0)
	{
		gCashShop.GCCashShopPeriodicItemSend(aIndex, item.m_Index, pMsg.result, item.m_PeriodicItemTime);
	}

	if (BC_MAP_RANGE(lpObj->Map) != 0)
	{
		if (gBloodCastle.CheckEventItemSerial(lpObj->Map, lpItem) != 0)
		{
			gBloodCastle.GetEventItem(lpObj->Map, aIndex, lpItem);
		}
	}

	if (IT_MAP_RANGE(lpObj->Map) != 0)
	{
		if (gIllusionTemple.CheckEventItemSerial(lpObj->Map, lpItem) != 0)
		{
			gIllusionTemple.GetEventItem(lpObj->Map, aIndex, lpItem);
		}
	}
}

void CItemManager::CGItemDropRecv(PMSG_ITEM_DROP_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_ITEM_DROP_SEND pMsg;

	pMsg.header.set(0x23, sizeof(pMsg));

	pMsg.result = 0;

	pMsg.slot = lpMsg->slot;

	if (lpObj->DieRegen != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

#if(GAMESERVER_UPDATE>=402)

	if (gDuel.GetDuelArenaBySpectator(aIndex) != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

#endif

	gObjFixInventoryPointer(aIndex);

	if (lpObj->Transaction == 1)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	CItem* lpItem = &lpObj->Inventory[lpMsg->slot];

	if (lpItem->IsItem() == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpItem->IsLuckyItem() != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpItem->m_IsPeriodicItem != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	//Item Lock
	if (lpObj->Lock > 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (gItemMove.CheckItemMoveAllowDrop(lpItem->m_Index) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if ((lpItem->m_Index < GET_ITEM(12, 0) && lpItem->m_Level > 4) || lpItem->IsExcItem() != 0 || lpItem->IsSetItem() != 0 || lpItem->IsJewelOfHarmonyItem() != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (gItemBagManager.DropItemByItemIndex(lpItem->m_Index, lpItem->m_Level, lpObj, lpObj->Map, lpMsg->x, lpMsg->y) != 0)
	{
		if (this->runtime_action_item(lpObj, lpItem, lpMsg->slot, 0xFF) == false)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}
		//this->InventoryDelItem(aIndex, lpMsg->slot);
	}
	else if (lpItem->m_Index == GET_ITEM(13, 7) && (lpItem->m_Level == 0 || lpItem->m_Level == 1)) // Siege Summon
	{
#if(GAMESERVER_TYPE==1)
		if (gMercenary.CreateMercenary(aIndex, 286 + lpItem->m_Level, lpMsg->x, lpMsg->y) == 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);
#endif
	}
	else if (lpItem->m_Index == GET_ITEM(13, 11) && lpItem->m_Level == 1) // Life Stone
	{
#if(GAMESERVER_TYPE==1)

		if (gLifeStone.CreateLifeStone(aIndex) == 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);

#endif
	}
	else if (lpItem->m_Index == GET_ITEM(13, 20) && lpItem->m_Level == 1) // Starter Ring 40
	{
		if (lpObj->Level < 40)
		{
			gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(257));
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);
		gItemBagManager.DropItemBySpecialValue(ITEM_BAG_STARTER_RING1, lpObj, lpObj->Map, lpMsg->x, lpMsg->y);
	}
	else if (lpItem->m_Index == GET_ITEM(13, 20) && lpItem->m_Level == 2) // Starter Ring 80
	{
		if (lpObj->Level < 80)
		{
			gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(258));
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);
		gItemBagManager.DropItemBySpecialValue(ITEM_BAG_STARTER_RING2, lpObj, lpObj->Map, lpMsg->x, lpMsg->y);
	}
	else if (lpItem->m_Index == GET_ITEM(14, 28)) // Lost Map
	{
		if (gKalima.CreateKalimaGate(aIndex, lpItem->m_Level, lpMsg->x, lpMsg->y) == 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);
	}
	else if (lpItem->m_Index == GET_ITEM(14, 63)) // Width Gruel
	{
		GCFireworksSend(lpObj, lpMsg->x, lpMsg->y);
		this->InventoryDelItem(aIndex, lpMsg->slot);
	}
	else if (lpItem->m_Index == GET_ITEM(14, 99)) // Christmas Firecracker
	{
		GCFireworksSend(lpObj, lpMsg->x, lpMsg->y);
		this->InventoryDelItem(aIndex, lpMsg->slot);
	}
	else if (gSummonScroll.CheckSummonScroll(lpItem->m_Index) != 0)
	{
		if (gSummonScroll.CreateSummonScrollMonster(aIndex, lpItem->m_Index, lpObj->Map, lpMsg->x, lpMsg->y) == 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);
	}
	else
	{
		if (gMap[lpObj->Map].ItemDrop(lpItem->m_Index, lpItem->m_Level, lpItem->m_Durability, lpMsg->x, lpMsg->y, lpItem->m_Option1, lpItem->m_Option2, lpItem->m_Option3, lpItem->m_NewOption, lpItem->m_SetOption, lpItem->m_Serial, aIndex, lpItem->m_PetItemLevel, lpItem->m_PetItemExp, lpItem->m_JewelOfHarmonyOption, lpItem->m_ItemOptionEx, lpItem->m_SocketOption, lpItem->m_SocketOptionBonus, lpItem->m_PeriodicItemTime) == 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);
	}

	pMsg.result = 1;

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

	this->UpdateInventoryViewport(aIndex, lpMsg->slot);
}

bool CItemManager::CGPkDrop(PMSG_ITEM_DROP_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return 0;
	}

	if (lpObj->DieRegen != 0)
	{
		return 0;
	}

	if (lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		return 0;
	}

#if(GAMESERVER_UPDATE>=402)

	if (gDuel.GetDuelArenaBySpectator(aIndex) != 0)
	{
		return 0;
	}

#endif

	gObjFixInventoryPointer(aIndex);

	if (lpObj->Transaction == 1)
	{
		return 0;
	}

	if (INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		return 0;
	}

	CItem* lpItem = &lpObj->Inventory[lpMsg->slot];

	if (lpItem->IsItem() == 0)
	{
		return 0;
	}

	if (lpItem->IsLuckyItem() != 0)
	{
		return 0;
	}

	if (lpItem->m_IsPeriodicItem != 0)
	{
		return 0;
	}

	if (gItemMove.CheckItemMoveAllowDrop(lpItem->m_Index) == 0)
	{
		return 0;
	}

	if (lpItem->m_Level > gServerInfo.m_PkItemDropMaxLevel)
	{
		return 0;
	}

	if (lpItem->m_Level < gServerInfo.m_PkItemDropMinLevel)
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropExecellent == 0 && lpItem->IsExcItem() != 0)
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropSocketItem == 0 && lpItem->IsSocketItem() != 0)
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropSetItem == 0 && lpItem->IsSetItem() != 0)
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropHarmony == 0 && lpItem->IsJewelOfHarmonyItem() != 0)
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropRaven == 0 && lpItem->m_Index == GET_ITEM(13, 5))
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropHorse == 0 && lpItem->m_Index == GET_ITEM(13, 4))
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropFenrir == 0 && lpItem->m_Index == GET_ITEM(13, 37))
	{
		return 0;
	}

	if (gServerInfo.m_PkItemDropSetItem == 1 && lpItem->IsSetItem() != 0 && (lpItem->Kind1 == ItemKind1::RING || lpItem->Kind1 == ItemKind1::PENDANT))
	{
		return 0;
	}

	if (lpItem->m_Index == GET_ITEM(13, 7) && (lpItem->m_Level == 0 || lpItem->m_Level == 1)) // Siege Summon
	{
		return  0;
	}
	else if (lpItem->m_Index == GET_ITEM(13, 11) && lpItem->m_Level == 1) // Life Stone
	{
		return 0;
	}
	else if (lpItem->m_Index == GET_ITEM(13, 20) && lpItem->m_Level == 1) // Starter Ring 40
	{
		return 0;
	}
	else if (lpItem->m_Index == GET_ITEM(13, 20) && lpItem->m_Level == 2) // Starter Ring 80
	{
		return 0;
	}
	else
	{
		if (gMap[lpObj->Map].ItemDrop(lpItem->m_Index, lpItem->m_Level, lpItem->m_Durability, lpMsg->x, lpMsg->y, lpItem->m_Option1, lpItem->m_Option2, lpItem->m_Option3, lpItem->m_NewOption, lpItem->m_SetOption, lpItem->m_Serial, aIndex, lpItem->m_PetItemLevel, lpItem->m_PetItemExp, lpItem->m_JewelOfHarmonyOption, lpItem->m_ItemOptionEx, lpItem->m_SocketOption, lpItem->m_SocketOptionBonus, lpItem->m_PeriodicItemTime) == 0)
		{
			return 0;
		}

		this->InventoryDelItem(aIndex, lpMsg->slot);
	}

	PMSG_ITEM_DROP_SEND pMsg;

	pMsg.header.set(0x23, sizeof(pMsg));

	pMsg.slot = lpMsg->slot;

	pMsg.result = 1;

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

	this->UpdateInventoryViewport(aIndex, lpMsg->slot);

	return 1;
}

void CItemManager::CGItemMoveRecv(PMSG_ITEM_MOVE_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_ITEM_MOVE_SEND pMsg;

	pMsg.header.setE(0x24, sizeof(pMsg));

	pMsg.SubCode = 0xFF;

	pMsg.Index = lpMsg->TargetSlot;

	memset(pMsg.ItemInfo, 0xFF, sizeof(pMsg.ItemInfo));

	if (lpObj->DieRegen != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpMsg->SourceFlag == REQUEST_EQUIPMENT_TRADE || lpMsg->TargetFlag == REQUEST_EQUIPMENT_TRADE) // Trade
	{
		if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state == 0 || OBJECT_RANGE(lpObj->TargetNumber) == 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}
	}

	if (lpMsg->SourceFlag == REQUEST_EQUIPMENT_STORAGE || lpMsg->TargetFlag == REQUEST_EQUIPMENT_STORAGE) // Warehouse
	{
		if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_WAREHOUSE || lpObj->Interface.state == 0 || lpObj->LoadWarehouse == 0 || lpObj->WarehouseLock == 1)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}
	}

	if (lpMsg->SourceFlag == REQUEST_EQUIPMENT_CHAOS_MIX || (lpMsg->SourceFlag >= REQUEST_EQUIPMENT_ELPIS_MIX && lpMsg->SourceFlag <= 20) || lpMsg->TargetFlag == REQUEST_EQUIPMENT_CHAOS_MIX || (lpMsg->TargetFlag >= REQUEST_EQUIPMENT_ELPIS_MIX && lpMsg->TargetFlag <= 20)) // Chaos Box
	{
		if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_CHAOS_BOX || lpObj->ChaosLock != 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}
	}

	if (lpMsg->SourceFlag == REQUEST_EQUIPMENT_MYSHOP || lpMsg->TargetFlag == REQUEST_EQUIPMENT_MYSHOP) // Personal Shop
	{
		if (lpObj->PShopOpen != 0 || lpObj->PShopTransaction != 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}
	}

	if (lpMsg->SourceFlag == REQUEST_EQUIPMENT_TRAINER_MIX || lpMsg->TargetFlag == REQUEST_EQUIPMENT_TRAINER_MIX) // Trainer
	{
		if (lpObj->Interface.use == INTERFACE_NONE || lpObj->Interface.type != INTERFACE_TRAINER || lpObj->ChaosLock != 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}
	}

	switch (lpMsg->SourceFlag)
	{
	case REQUEST_EQUIPMENT_INVENTORY:
		{
			if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY) // -> Inventory
			{
				if ((pMsg.SubCode = this->MoveItemToInventoryFromInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_TRADE) // -> Trade
			{
				if ((pMsg.SubCode = this->MoveItemToTradeFromInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Trade[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_STORAGE) // -> Warehouse
			{
				if ((pMsg.SubCode = this->MoveItemToWarehouseFromInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Warehouse[lpMsg->TargetSlot]);
				}
			}
			if ((lpMsg->TargetFlag == REQUEST_EQUIPMENT_CHAOS_MIX || (lpMsg->TargetFlag >= REQUEST_EQUIPMENT_TRAINER_MIX && lpMsg->TargetFlag <= 20))) // -> Chaos Box
			{
				if ((pMsg.SubCode = this->MoveItemToChaosBoxFromInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->ChaosBox[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_MYSHOP) // -> Personal Shop
			{
				if ((pMsg.SubCode = this->MoveItemToPersonalShopFromInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == 23) // Inventory -> MixExpansion Inventory
			{
#ifdef CHAOS_MACHINE_EXTENSION
				if ((pMsg.SubCode = this->MoveItemToMixExpansionFromInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->InventoryMix[lpMsg->TargetSlot]);
				}
#endif // CHAOS_MACHINE_EXTENSION
			}
		}
		break;
	case REQUEST_EQUIPMENT_TRADE:
		{
			if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY) // Trade -> Inventory
			{
				if ((pMsg.SubCode = this->MoveItemToInventoryFromTrade(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_TRADE) // Trade -> Trade
			{
				if ((pMsg.SubCode = this->MoveItemToTradeFromTrade(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Trade[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == 21) // Trade -> Event Inventory
			{
				if ((pMsg.SubCode = this->MoveItemToEventInventoryFromTrade(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
	#if(GAMESERVER_UPDATE>=802)
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->EventInventory[lpMsg->TargetSlot]);
	#endif
				}
			}
		}
		break;
	case REQUEST_EQUIPMENT_STORAGE:
		{
			if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY) // Warehouse -> Inventory
			{
				if ((pMsg.SubCode = this->MoveItemToInventoryFromWarehouse(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_STORAGE) // Warehouse -> Warehouse
			{
				if ((pMsg.SubCode = this->MoveItemToWarehouseFromWarehouse(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Warehouse[lpMsg->TargetSlot]);
				}
			}
		}
		break;
	case REQUEST_EQUIPMENT_MYSHOP:
		{
			if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY) // Personal Shop -> Inventory
			{
				if ((pMsg.SubCode = this->MoveItemToInventoryFromPersonalShop(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
				}
			}
			else if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_MYSHOP) // Personal Shop -> Personal Shop
			{
				if ((pMsg.SubCode = this->MoveItemToPersonalShopFromPersonalShop(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
				}
			}
		}
		break;
	case 21:
		{
			if (lpMsg->TargetFlag == 1) // Event Inventory -> Trade
			{
				if ((pMsg.SubCode = this->MoveItemToTradeFromEventInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
	#if(GAMESERVER_UPDATE>=802)
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Trade[lpMsg->TargetSlot]);
	#endif
				}
			}
			else if (lpMsg->TargetFlag == 21) // Event Inventory -> Event Inventory
			{
				if ((pMsg.SubCode = this->MoveItemToEventInventoryFromEventInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
	#if(GAMESERVER_UPDATE>=802)
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->EventInventory[lpMsg->TargetSlot]);
	#endif
				}
			}
		}
		break;
	case 22:
		if (lpMsg->TargetFlag == 22) // Muun Inventory -> Muun Inventory
		{
			if ((pMsg.SubCode = this->MoveItemToMuunInventoryFromMuunInventory(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
			{
#if(GAMESERVER_UPDATE>=802)
				this->ItemByteConvert(pMsg.ItemInfo, lpObj->MuunInventory[lpMsg->TargetSlot]);
#endif
			}
		}
		break;
	case 23:
		{
	#ifdef CHAOS_MACHINE_EXTENSION
			if (lpMsg->SourceFlag == 23 && lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY) // Inventory -> MixExpansion Inventory
			{
				if ((pMsg.SubCode = this->MoveItemToInventoryFromMixExpansion(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
				{
					this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
				}
			}
	#endif // CHAOS_MACHINE_EXTENSION
		}
		break;
	default:
		{
			if ((lpMsg->SourceFlag == REQUEST_EQUIPMENT_CHAOS_MIX || (lpMsg->SourceFlag >= REQUEST_EQUIPMENT_TRAINER_MIX && lpMsg->SourceFlag <= 20))) //-- Chaos Box ->
			{
				if (lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY) // Chaos Box -> Inventory
				{
					if ((pMsg.SubCode = this->MoveItemToInventoryFromChaosBox(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
					{
						this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[lpMsg->TargetSlot]);
					}
				}
				else if ((lpMsg->TargetFlag == REQUEST_EQUIPMENT_CHAOS_MIX || (lpMsg->TargetFlag >= 5 && lpMsg->TargetFlag <= 20))) // Chaos Box -> Chaos Box
				{
					if ((pMsg.SubCode = this->MoveItemToChaosBoxFromChaosBox(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot, lpMsg->TargetFlag)) != 0xFF)
					{
						this->ItemByteConvert(pMsg.ItemInfo, lpObj->ChaosBox[lpMsg->TargetSlot]);
					}
				}
			}
		}
		break;
	}

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

	if (pMsg.SubCode == 0xFF && lpMsg->SourceFlag == REQUEST_EQUIPMENT_INVENTORY && lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY)
	{
		if (lpObj->Inventory[lpMsg->SourceSlot].IsItem() != 0)
		{
			this->GCItemDurSend(lpObj->Index, lpMsg->SourceSlot, (BYTE)lpObj->Inventory[lpMsg->SourceSlot].m_Durability, 0);
		}
	}

	if (pMsg.SubCode != 0xFF && lpMsg->TargetFlag == REQUEST_EQUIPMENT_INVENTORY)
	{
		if (lpObj->Inventory[lpMsg->TargetSlot].m_IsPeriodicItem != 0)
		{
			gCashShop.GCCashShopPeriodicItemSend(aIndex, lpObj->Inventory[lpMsg->TargetSlot].m_Index, lpMsg->TargetSlot, lpObj->Inventory[lpMsg->TargetSlot].m_PeriodicItemTime);
		}
	}
}

void CItemManager::CGItemUseRecv(PMSG_ITEM_USE_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	GCLifeSend(aIndex, 0xFD, (BYTE)lpObj->Life, lpObj->Shield);

	if (lpObj->DieRegen != 0)
	{
		return;
	}

	if (lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
	{
		return;
	}

	gObjFixInventoryPointer(aIndex);

	if (lpObj->Transaction == 1 && lpObj->TradeDuel == 0)
	{
		return;
	}

	if (INVENTORY_FULL_RANGE(lpMsg->SourceSlot) == 0)
	{
		return;
	}

	if (lpMsg->SourceSlot == lpMsg->TargetSlot)
	{
		return;
	}

	CItem* lpItem = &lpObj->Inventory[lpMsg->SourceSlot];

	if (lpItem->IsItem() == 0)
	{
		return;
	}

	if ((lpItem->m_Index >= GET_ITEM(12, 7) && lpItem->m_Index <= GET_ITEM(12, 24)) || lpItem->m_Index == GET_ITEM(12, 35) || (lpItem->m_Index >= GET_ITEM(12, 44) && lpItem->m_Index <= GET_ITEM(12, 48)) || lpItem->m_Index >= GET_ITEM(15, 0))
	{
		if (gObjectManager.CharacterUseScroll(lpObj, lpItem) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(12, 30)) // Bundle of Jewel of Bless
	{
		if (gObjectManager.CharacterUseJewelOfBles(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
			this->GCItemModifySend(aIndex, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(13, 15)) // Fruit
	{
		if (lpMsg->type == 0)
		{
			if (gFruit.UseFruitAddPoint(lpObj, ((lpItem->m_Level < 4) ? (3 - lpItem->m_Level) : lpItem->m_Level)) != 0)
			{
				this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
				this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
			}
		}
		else
		{
			if (gFruit.UseFruitSubPoint(lpObj, ((lpItem->m_Level < 4) ? (3 - lpItem->m_Level) : lpItem->m_Level)) != 0)
			{
				this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
				this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
			}
		}
	}
	else if (lpItem->m_Index == GET_ITEM(13, 48)) // Kalima Ticket
	{
		if (gKalima.CreateKalimaGate(aIndex, gKalima.GetKalimaGateLevel(lpObj), lpObj->X, lpObj->Y) != 0)
		{
			this->DecreaseItemDur(lpObj, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index >= GET_ITEM(13, 54) && lpItem->m_Index <= GET_ITEM(13, 58)) // Fruit Reset
	{
		if (gFruit.UseFruitResetPoint(lpObj, (lpItem->m_Index - GET_ITEM(13, 54)), (int)(lpItem->m_Durability * 10)) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(13, 66)) // Santa Town Invitation
	{
		gObjTeleport(aIndex, 62, 223, 23);
		this->DecreaseItemDur(lpObj, lpMsg->SourceSlot, 1);
	}
	else if (lpItem->m_Index == GET_ITEM(13, 69)) // Talisman of Resurrection
	{
		if (gObjectManager.CharacterUseTalismanOfResurrection(lpObj, lpMsg->SourceSlot) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(13, 70)) // Talisman of Mobility
	{
		if (gObjectManager.CharacterUseTalismanOfMobility(lpObj, lpMsg->SourceSlot) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if ((lpItem->m_Index >= GET_ITEM(14, 0) && lpItem->m_Index <= GET_ITEM(14, 6)) || (lpItem->m_Index >= GET_ITEM(14, 35) && lpItem->m_Index <= GET_ITEM(14, 40)) || lpItem->m_Index == GET_ITEM(14, 70) || lpItem->m_Index == GET_ITEM(14, 71) || lpItem->m_Index == GET_ITEM(14, 133))
	{
		if (gObjectManager.CharacterUsePotion(lpObj, lpItem) != 0)
		{
			this->DecreaseItemDur(lpObj, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 7)) // Siege Potion
	{
		if (lpItem->m_Level == 0)
		{
			gEffectManager.AddEffect(lpObj, 0, EFFECT_BLESS_POTION, 120, 0, 0, 0, 0);
		}
		else
		{
			gEffectManager.AddEffect(lpObj, 0, EFFECT_SOUL_POTION, 60, 20, 8, 0, 0);
		}

		this->DecreaseItemDur(lpObj, lpMsg->SourceSlot, 1);
	}
	else if (lpItem->m_Index == GET_ITEM(14, 8)) // Antidote
	{
		gEffectManager.DelEffect(lpObj, EFFECT_POISON);
		gEffectManager.DelEffect(lpObj, EFFECT_ICE);
		this->DecreaseItemDur(lpObj, lpMsg->SourceSlot, 1);
	}
	else if (lpItem->m_Index == GET_ITEM(14, 9)) // Ale
	{
		gObjUseDrink(lpObj, lpItem->m_Level);
		this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
		this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
	}
	else if (lpItem->m_Index == GET_ITEM(14, 10)) // Town Portal Scroll
	{
		if (gObjectManager.CharacterUsePortal(lpObj, lpItem) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 13)) // Jewel of Bless
	{
		if (gObjectManager.CharacterUseJewelOfBles(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 14)) // Jewel of Soul
	{
		if (gObjectManager.CharacterUseJewelOfSoul(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 16)) // Jewel of Life
	{
		if (gObjectManager.CharacterUseJewelOfLife(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 20)) // Remedy of Love
	{
		if (lpItem->m_Level == 0)
		{
			gObjUseDrink(lpObj, 2);
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 42)) // Jewel of Harmony
	{
		if (gJewelOfHarmonyOption.AddJewelOfHarmonyOption(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 43) || lpItem->m_Index == GET_ITEM(14, 44)) // Smelting Stone
	{
		if (gJewelOfHarmonyOption.AddSmeltStoneOption(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 91)) // SU Creation Card
	{
		if (gObjectManager.CharacterUseCreationCard(lpObj, lpItem) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 160)) // Jewel of Extension
	{
		if (gLuckyItem.CharacterUseJewelOfExtension(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 161)) // Jewel of Elevation
	{
		if (gLuckyItem.CharacterUseJewelOfElevation(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 162)) // Inventory Expansion
	{
		if (gObjectManager.CharacterUseInventoryExpansion(lpObj, lpItem) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 163)) // Warehouse Expansion
	{
		if (gObjectManager.CharacterUseWarehouseExpansion(lpObj, lpItem) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 169)) // RF Creation Card
	{
		if (gObjectManager.CharacterUseCreationCard(lpObj, lpItem) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
		}
	}
	else if (lpItem->m_Index == GET_ITEM(14, 209)) // Tradeable Seal
	{
		if (gObjectManager.CharacterUseTradeableSeal(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->InventoryDelItem(aIndex, lpMsg->SourceSlot);
			this->GCItemDeleteSend(aIndex, lpMsg->SourceSlot, 1);
			this->GCItemModifySend(aIndex, lpMsg->TargetSlot);
		}
	}
	else if (gCustomJewel.CheckCustomJewelByItem(lpItem->m_Index) != 0)
	{
		if (gObjectManager.CharacterUseCustomJewel(lpObj, lpMsg->SourceSlot, lpMsg->TargetSlot) != 0)
		{
			this->runtime_action_item(lpObj, lpItem, lpMsg->SourceSlot, lpMsg->TargetSlot);
		}
	}
	else
	{
		EFFECT_INFO* lpInfo = gEffectManager.GetInfoByItem(lpItem->m_Index);

		if (lpInfo != 0)
		{
			if (gEffectManager.AddEffect(lpObj, 0, lpInfo->Index, 0, 0, 0, 0, 0) != 0)
			{
				this->DecreaseItemDur(lpObj, lpMsg->SourceSlot, 1);
			}
		}
	}
}

void CItemManager::CGItemBuyRecv(PMSG_ITEM_BUY_RECV* lpMsg, int aIndex, int ok) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_ITEM_BUY_SEND pMsg;

	pMsg.header.set(0x32, sizeof(pMsg));

	pMsg.result = 0xFF;

	if ((GetTickCount() - lpObj->ShopDelay) < 700)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_SHOP)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (SHOP_RANGE(lpObj->TargetShopNumber) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	gObjFixInventoryPointer(aIndex);

	if (lpObj->Transaction == 1)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	CItem item;

	if (gShopManager.GetItemByIndex(lpObj->TargetShopNumber, &item, lpMsg->slot) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (ok == 0 && gServerInfo.m_CustomShopMessageBox > 0)
	{

		if (gServerInfo.m_CustomShopMessageBox == 1 && (item.Coin1 > 0 || item.Coin2 > 0 || item.Coin3 > 0))
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

			PMSG_ITEM_BUY_NEW pMsg;

			pMsg.header.set(0xF3, 0xED, sizeof(pMsg));

			pMsg.slot = lpMsg->slot;

			DataSend(aIndex, (LPBYTE)&pMsg, pMsg.header.size);
			return;
		}
		if (gServerInfo.m_CustomShopMessageBox == 2)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

			PMSG_ITEM_BUY_NEW pMsg;

			pMsg.header.set(0xF3, 0xED, sizeof(pMsg));

			pMsg.slot = lpMsg->slot;

			DataSend(aIndex, (LPBYTE)&pMsg, pMsg.header.size);
			return;
		}
	}

	int tax = (item.m_BuyMoney * gCastleSiegeSync.GetTaxRateStore(lpObj->Index)) / 100;

	DWORD value;

	int valuex;

	if (gItemStack.GetItemMaxStack(item.m_Index, item.m_Level) == 0
		|| item.Kind2 == ItemKind2::ARROW
		|| item.Kind2 == ItemKind2::BOLT
		|| item.Kind2 == ItemKind2::QUIVER)
	{
		value = (DWORD)item.m_BuyMoney;
		valuex = 1;
	}
	else
	{
		value = (DWORD)(item.m_BuyMoney * (DWORD)item.m_Durability);
		valuex = (int)item.m_Durability;
	}

	if (((DWORD)lpObj->Money) < ((item.m_BuyMoney + tax) * valuex))
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(834));
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if ((item.Coin1) > 0 && (lpObj->Coin1 < (item.Coin1 * valuex)))
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(834));
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if ((item.Coin2) > 0 && (lpObj->Coin2 < (item.Coin2 * valuex)))
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(834));
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (item.Coin3 > 0 && (lpObj->Coin3 < (item.Coin3 * valuex)))
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(834));
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (item.m_Index == GET_ITEM(13, 4) || item.m_Index == GET_ITEM(13, 5)
#ifdef PET_SYSTEM_GAIN_EXP
		|| item.Kind3 == ItemKind3::PET_GAIN_EXPERIENCE
#endif // PET_SYSTEM_GAIN_EXP
		) //Dark Horse,Dark Reaven
	{
		GDCreateItemSend(aIndex, 0xEB, 0, 0, item.m_Index, (BYTE)item.m_Level, (BYTE)item.m_Durability, item.m_Option1, item.m_Option2, item.m_Option3, -1, item.m_NewOption, item.m_SetOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption, item.m_SocketOptionBonus, 0);

		if (item.Coin1 > 0 || item.Coin2 > 0 || item.Coin3 > 0)
		{
			GDSetCoinSend(lpObj->Index, -(item.Coin1), -(item.Coin2), -(item.Coin3), "CommonShop");
		}
		else
		{
			lpObj->Money -= item.m_BuyMoney + tax;

			GCMoneySend(aIndex, lpObj->Money);

			gCastleSiegeSync.AddTributeMoney(tax);
		}

		gShop.GCItemValueSend(aIndex);

		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (item.m_Index >= GET_ITEM(13, 71) && item.m_Index <= GET_ITEM(13, 75)) // Random Item
	{
		if (gMossMerchant.GambleItem(lpObj, (item.m_Index - GET_ITEM(13, 71))) == 0)
		{
			DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			return;
		}


		lpObj->Money -= item.m_BuyMoney + tax;

		GCMoneySend(aIndex, lpObj->Money);

		gCastleSiegeSync.AddTributeMoney(tax);

		gShop.GCItemValueSend(aIndex);

		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}



	if (this->InventoryInsertItemStack(lpObj, &item) != 0)
	{
		if (item.Coin1 > 0 || item.Coin2 > 0 || item.Coin3 > 0)
		{
			GDSetCoinSend(lpObj->Index, -(item.Coin1 * valuex), -(item.Coin2 * valuex), -(item.Coin3 * valuex), "CommonShopBuy");
		}
		else
		{
			lpObj->Money -= value + tax;

			GCMoneySend(aIndex, lpObj->Money);

			gCastleSiegeSync.AddTributeMoney(tax);
		}

		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	pMsg.result = this->InventoryInsertItem(aIndex, item);

	if (pMsg.result == 0xFF)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (item.Coin1 > 0 || item.Coin2 > 0 || item.Coin3 > 0)
	{
		GDSetCoinSend(lpObj->Index, -(item.Coin1 * valuex), -(item.Coin2 * valuex), -(item.Coin3 * valuex), "CommonShopBuy");
	}
	else
	{
		lpObj->Money -= value + tax;

		GCMoneySend(aIndex, lpObj->Money);

		gCastleSiegeSync.AddTributeMoney(tax);
	}

	this->ItemByteConvert(pMsg.ItemInfo, item);

	gShop.GCItemValueSend(aIndex);

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
}

void CItemManager::CGItemSellRecv(PMSG_ITEM_SELL_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_ITEM_SELL_SEND pMsg;

	pMsg.header.set(0x33, sizeof(pMsg));

	pMsg.result = 0;

	pMsg.money = 0;

	if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_SHOP)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (SHOP_RANGE(lpObj->TargetShopNumber) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	gObjFixInventoryPointer(aIndex);

	if (lpObj->Transaction == 1)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	CItem* lpItem = &lpObj->Inventory[lpMsg->slot];

	if (lpItem->IsItem() == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (gItemMove.CheckItemMoveAllowSell(lpItem->m_Index) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	//itemLock
	if (lpObj->Lock > 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (gServerInfo.m_TradeItemBlockSell != 0 && this->CheckItemMoveToBlock(lpObj, lpItem) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpItem->m_Index == GET_ITEM(13, 4) || lpItem->m_Index == GET_ITEM(13, 5)
#ifdef PET_SYSTEM_GAIN_EXP
		|| (lpItem->Kind3 == ItemKind3::PET_GAIN_EXPERIENCE)
#endif // PET_SYSTEM_GAIN_EXP
		) // Dark Horse,Dark Reaven
	{
		lpItem->PetValue();
	}
	else
	{
		lpItem->Value();
	}

	DWORD value;

	if (gItemStack.GetItemMaxStack(lpItem->m_Index, lpItem->m_Level) == 0
		|| lpItem->Kind2 == ItemKind2::ARROW
		|| lpItem->Kind2 == ItemKind2::BOLT
		|| lpItem->Kind2 == ItemKind2::QUIVER
		)
	{
		value = (DWORD)lpItem->m_SellMoney;
	}
	else
	{
		value = (DWORD)(lpItem->m_SellMoney * (DWORD)lpItem->m_Durability);
	}

	if (lpItem->Sell > 0)
	{
		if (lpItem->Coin1 > 0)
		{
			GDSetCoinSend(lpObj->Index, value, 0, 0, "CommonShopSell");
		}
		else if (lpItem->Coin2 > 0)
		{
			GDSetCoinSend(lpObj->Index, 0, value, 0, "CommonShopSell");
		}
		else if (lpItem->Coin3 > 0)
		{
			GDSetCoinSend(lpObj->Index, 0, 0, value, "CommonShopSell");
		}
		else
		{
			if (gObjCheckMaxMoney(aIndex, value) == 0)
			{
				lpObj->Money = MAX_MONEY;
			}
			else
			{
				lpObj->Money += value;
			}
		}
	}
	else
	{
		if (gObjCheckMaxMoney(aIndex, value) == 0)
		{
			lpObj->Money = MAX_MONEY;
		}
		else
		{
			lpObj->Money += value;
		}
	}

#if(GAMESERVER_UPDATE>=701)

	gPentagramSystem.DelAllPentagramJewelInfo(lpObj, lpItem, PENTAGRAM_JEWEL_TYPE_INVENTORY);

#endif

	this->InventoryDelItem(aIndex, lpMsg->slot);

	pMsg.result = 1;

	pMsg.money = lpObj->Money;

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

	this->UpdateInventoryViewport(aIndex, lpMsg->slot);
}

void CItemManager::CGItemBreakRecv(PMSG_ITEM_REPAIR_RECV* lpMsg, int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) != 0)
	{
		if (lpMsg->slot != 0xFF)
		{
			this->InventoryBreakItemStack(lpObj, &lpObj->Inventory[lpMsg->slot], lpMsg->slot);
		}
	}

}

void CItemManager::CGItemRepairRecv(PMSG_ITEM_REPAIR_RECV* lpMsg, int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (lpMsg->slot != 0xFF)
	{
		if (lpObj->Inventory[lpMsg->slot].m_Index != GET_ITEM(13, 4) && lpObj->Inventory[lpMsg->slot].m_Index != GET_ITEM(13, 5))
		{
			if (lpObj->Interface.use == INTERFACE_TRADE)
			{
				return;
			}
		}
	}

	PMSG_ITEM_REPAIR_SEND pMsg;

	pMsg.header.set(0x34, sizeof(pMsg));

	pMsg.money = 0;


	if (lpMsg->type == 1 && lpObj->Level < 50)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	if (lpMsg->slot == 0xFF)
	{
		int MaxValue = this->GetInventoryMaxValue(lpObj);

		for (int n = 0; n < MaxValue; n++)
		{
			if (lpObj->Inventory[n].IsItem() == 0)
			{
				continue;
			}

			pMsg.money = this->RepairItem(lpObj, &lpObj->Inventory[n], n, lpMsg->type);

			if (pMsg.money != 0)
			{
				DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
			}
		}

		gObjectManager.CharacterCalcAttribute(aIndex);
		return;
	}

	if (INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		return;
	}

	pMsg.money = this->RepairItem(lpObj, &lpObj->Inventory[lpMsg->slot], lpMsg->slot, lpMsg->type);

	if (pMsg.money != 0)
	{
		DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
		gObjectManager.CharacterCalcAttribute(aIndex);
	}
}

void CItemManager::GCItemMoveSend(int aIndex, BYTE result, BYTE slot, BYTE* ItemInfo) // OK
{
	PMSG_ITEM_MOVE_SEND pMsg;

	pMsg.header.setE(0x24, sizeof(pMsg));

	pMsg.SubCode = result;

	pMsg.Index = slot;

	memcpy(pMsg.ItemInfo, ItemInfo, MAX_ITEM_INFO);

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
}

void CItemManager::GCItemChangeSend(int aIndex, BYTE slot) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	PMSG_ITEM_CHANGE_SEND pMsg;

	pMsg.header.set(0x25, sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);
	pMsg.index[1] = SET_NUMBERLB(aIndex);

	this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[slot]);

	//pMsg.ItemInfo[1] = slot * 16;
	//pMsg.ItemInfo[1] |= ((lpObj->Inventory[slot].m_Level - 1) / 2) & 0x0F;
	pMsg.slot = slot;

#if(GAMESERVER_UPDATE>=701)
	pMsg.attribute = lpObj->ElementalAttribute;
#endif

	MsgSendV2(lpObj, (BYTE*)&pMsg, pMsg.header.size);
}

void CItemManager::GCItemDeleteSend(int aIndex, BYTE slot, BYTE flag) // OK
{
	PMSG_ITEM_DELETE_SEND pMsg;

	pMsg.header.set(0x28, sizeof(pMsg));

	pMsg.slot = slot;

	pMsg.flag = flag;

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
}

void CItemManager::GCItemDurSend(int aIndex, BYTE slot, BYTE dur, BYTE flag) // OK
{
	PMSG_ITEM_DUR_SEND pMsg;

	pMsg.header.set(0x2A, sizeof(pMsg));

	pMsg.slot = slot;

	pMsg.dur = dur;

	pMsg.flag = flag;

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
}

void CItemManager::GCItemListSend(int aIndex, bool SendTime) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[4096];

	PMSG_ITEM_LIST_SEND pMsg;

	pMsg.header.setE(0xF3, 0x10, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	PMSG_ITEM_LIST info;

	for (int n = 0; n < INVENTORY_SIZE; n++)
	{
		CItem* lpItem = &lpObj->Inventory[n];

		if (lpItem->IsItem() != 0 && lpItem->m_ItemExist != 0)
		{
			info.slot = n;

			this->ItemByteConvert(info.ItemInfo, lpObj->Inventory[n]);

			if ((size + sizeof(info)) <= sizeof(send))
			{
				memcpy(&send[size], &info, sizeof(info));
				size += sizeof(info);
				pMsg.count++;
			}
		}
	}

	//LogAdd(LOG_BLUE, "[GCItemListSend][count: %d][Size: %d][data1: %d][data2: %d]", pMsg.count, size, sizeof(PMSG_ITEM_LIST_SEND), sizeof(PMSG_ITEM_LIST));

	pMsg.header.SetSize(size);
	memcpy(send, &pMsg, sizeof(pMsg));
	DataSend(aIndex, send, size);

	if (SendTime)
	{
		for (int n = 0; n < INVENTORY_SIZE; n++)
		{
			CItem* lpItem = &lpObj->Inventory[n];

			if (lpItem->IsItem() != 0 && lpItem->m_ItemExist != 0 && lpItem->m_IsPeriodicItem != 0)
			{
				gCashShop.GCCashShopPeriodicItemSend(aIndex, lpItem->m_Index, n, lpItem->m_PeriodicItemTime);
			}
		}
	}
}

void CItemManager::GCItemEquipmentSend(int aIndex) // OK
{
	PMSG_ITEM_EQUIPMENT_SEND pMsg;

	pMsg.header.set(0xF3, 0x13, sizeof(pMsg));

	pMsg.index[0] = SET_NUMBERHB(aIndex);

	pMsg.index[1] = SET_NUMBERLB(aIndex);

	memcpy(pMsg.Equipment, gObj[aIndex].CharSet, (sizeof(DWORD) * EQUIPMENT_NEW_LENGTH));

	MsgSendV2(&gObj[aIndex], (BYTE*)&pMsg, pMsg.header.size);
}

void CItemManager::GCItemModifySend(int aIndex, BYTE slot) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Inventory[slot].IsItem() == 0)
	{
		return;
	}

	PMSG_ITEM_MODIFY_SEND pMsg;

	pMsg.header.set(0xF3, 0x14, sizeof(pMsg));

	pMsg.slot = slot;

	this->ItemByteConvert(pMsg.ItemInfo, lpObj->Inventory[slot]);

	DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);
}

void CItemManager::CGItemBuyConfirmRecv(PMSG_ITEM_BUY_NEW* lpMsg, int aIndex) // OK
{
	PMSG_ITEM_BUY_RECV pMsg;

	pMsg.header.set(0x32, sizeof(pMsg));

	pMsg.slot = lpMsg->slot;

	CGItemBuyRecv(&pMsg, aIndex, 1);
}

void CItemManager::CGItemPostChatRecv(PMSG_ITEM_BUY_RECV* lpMsg, int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (lpObj->Inventory[lpMsg->slot].IsItem() == 0)
	{
		return;
	}

	PHEADER_DEFAULT_SUBCODE_CHAT_ITEM pMsg;

	pMsg.header.set(0xF3, 0x15, sizeof(pMsg));

	memcpy(pMsg.ID, lpObj->Name, sizeof(pMsg.ID));

	this->ItemByteConvert(pMsg.Item, lpObj->Inventory[lpMsg->slot]);

	//DataSend(aIndex, (BYTE*)&pMsg, pMsg.header.size);

	DataSendAll((BYTE*)&pMsg, pMsg.header.size);
}
