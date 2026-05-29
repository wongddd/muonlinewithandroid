// SocketItemOption.cpp: implementation of the CSocketItemOption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketItemOption.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "RandomManager.h"
#include "Util.h"

CSocketItemOption gSocketItemOption;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketItemOption::CSocketItemOption() // OK
{
	this->Init();
}

CSocketItemOption::~CSocketItemOption() // OK
{

}

void CSocketItemOption::Init() // OK
{
	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION; n++)
	{
		this->m_SocketItemOptionInfo[n].Index = -1;
		this->m_SocketItemBonusOptionInfo[n].Index = -1;
		this->m_SocketItemPackageOptionInfo[n].Index = -1;
	}
}

void CSocketItemOption::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if (section == 0)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					SOCKET_ITEM_OPTION_INFO info;

					memset(&info, 0, sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.Type = lpMemScript->GetAsNumber();

					info.SubIndex = lpMemScript->GetAsNumber();

					strcpy_s(info.Name, lpMemScript->GetAsString());

					info.Rate = lpMemScript->GetAsNumber();

					for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TABLE; n++)
					{
						info.ValueTable[n] = lpMemScript->GetAsNumber();
					}

					this->SetInfo(info);
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					SOCKET_ITEM_BONUS_OPTION_INFO info;

					memset(&info, 0, sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.MinItemSection = lpMemScript->GetAsNumber();

					info.MaxItemSection = lpMemScript->GetAsNumber();

					strcpy_s(info.Name, lpMemScript->GetAsString());

					info.Value = lpMemScript->GetAsNumber();

					for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TABLE; n++)
					{
						info.RequireOptionTypeTable[n] = lpMemScript->GetAsNumber();
					}

					this->SetBonusInfo(info);
				}
				else if (section == 2)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					SOCKET_ITEM_PACKAGE_OPTION_INFO info;

					memset(&info, 0, sizeof(info));

					info.Index = lpMemScript->GetNumber();

					strcpy_s(info.Name, lpMemScript->GetAsString());

					info.Value = lpMemScript->GetAsNumber();

					for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TYPE; n++)
					{
						info.RequireOptionTypeCountTable[n] = lpMemScript->GetAsNumber();
					}

					this->SetPackageInfo(info);
				}
				else
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CSocketItemOption::ExportXML(std::string filename)
{
	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("SeedSphere");

	for (int i = 0; i < MAX_SOCKET_ITEM_OPTION; i++)
	{
		SOCKET_ITEM_OPTION_INFO* info = &this->m_SocketItemOptionInfo[i];

		if (info->Index == -1)
			continue;

		pugi::xml_node leaf = root.append_child("Info");

		leaf.append_attribute("Index").set_value(info->Index);

		leaf.append_attribute("Seed").set_value(info->Type);

		leaf.append_attribute("Level").set_value(info->SubIndex);

		leaf.append_attribute("Rate").set_value(info->Rate);

		leaf.append_attribute("SphereLevel1").set_value(info->ValueTable[0]);

		leaf.append_attribute("SphereLevel2").set_value(info->ValueTable[1]);

		leaf.append_attribute("SphereLevel3").set_value(info->ValueTable[2]);

		leaf.append_attribute("SphereLevel4").set_value(info->ValueTable[3]);

		leaf.append_attribute("SphereLevel5").set_value(info->ValueTable[4]);

		leaf.append_attribute("Name").set_value(info->Name);
	}

	//--------------------------------------------------

	root = doc.append_child("BonusSocket");

	for (int i = 0; i < MAX_SOCKET_ITEM_OPTION; i++)
	{
		SOCKET_ITEM_BONUS_OPTION_INFO* info = &this->m_SocketItemBonusOptionInfo[i];

		if (info->Index == -1)
			continue;

		pugi::xml_node leaf = root.append_child("Info");

		leaf.append_attribute("Index").set_value(info->Index);

		leaf.append_attribute("MinItemSection").set_value(info->MinItemSection);

		leaf.append_attribute("MaxItemSection").set_value(info->MaxItemSection);

		leaf.append_attribute("Value").set_value(info->Value);

		leaf.append_attribute("ReqSeed1").set_value(info->RequireOptionTypeTable[0]);

		leaf.append_attribute("ReqSeed2").set_value(info->RequireOptionTypeTable[1]);

		leaf.append_attribute("ReqSeed3").set_value(info->RequireOptionTypeTable[2]);

		leaf.append_attribute("ReqSeed4").set_value(info->RequireOptionTypeTable[3]);

		leaf.append_attribute("ReqSeed5").set_value(info->RequireOptionTypeTable[4]);

		leaf.append_attribute("Name").set_value(info->Name);
	}

	//--------------------------------------------------

	root = doc.append_child("PackageOption");

	for (int i = 0; i < MAX_SOCKET_ITEM_OPTION; i++)
	{
		SOCKET_ITEM_PACKAGE_OPTION_INFO* info = &this->m_SocketItemPackageOptionInfo[i];

		if (info->Index == -1)
			continue;

		pugi::xml_node leaf = root.append_child("Info");

		leaf.append_attribute("Index").set_value(info->Index);

		leaf.append_attribute("Value").set_value(info->Value);

		leaf.append_attribute("FireLevel").set_value(info->RequireOptionTypeCountTable[0]);

		leaf.append_attribute("WaterLevel").set_value(info->RequireOptionTypeCountTable[1]);

		leaf.append_attribute("IceLevel").set_value(info->RequireOptionTypeCountTable[2]);

		leaf.append_attribute("WindLevel").set_value(info->RequireOptionTypeCountTable[3]);

		leaf.append_attribute("LightningLevel").set_value(info->RequireOptionTypeCountTable[4]);

		leaf.append_attribute("EarthLevel").set_value(info->RequireOptionTypeCountTable[5]);

		leaf.append_attribute("Name").set_value(info->Name);
	}

	doc.save_file(filename.c_str());
}

void CSocketItemOption::ExportBMD(std::string filename)
{
	int MaxLine = 150;

	int Size = sizeof(SOCKET_OPTION_INFO);

	SOCKET_OPTION_INFO m_SocketOptionInfo[3][50];

	memset(m_SocketOptionInfo, 0, sizeof(m_SocketOptionInfo));

	int current = 0;

	for (int i = 0; i < MAX_SOCKET_ITEM_OPTION; i++)
	{
		SOCKET_ITEM_OPTION_INFO* info = &this->m_SocketItemOptionInfo[i];

		int index = info->Index;

		if (index == -1)
			continue;

		m_SocketOptionInfo[current][index].m_iOptionID = info->Index;

		m_SocketOptionInfo[current][index].m_iOptionCategory = info->Type;

		strcpy_s(m_SocketOptionInfo[current][index].m_szOptionName, info->Name);

		m_SocketOptionInfo[current][index].m_bOptionType = this->CalciOptionType(info->Index);

		m_SocketOptionInfo[current][index].m_iOptionValue[0] = info->ValueTable[0];

		m_SocketOptionInfo[current][index].m_iOptionValue[1] = info->ValueTable[1];

		m_SocketOptionInfo[current][index].m_iOptionValue[2] = info->ValueTable[2];

		m_SocketOptionInfo[current][index].m_iOptionValue[3] = info->ValueTable[3];

		m_SocketOptionInfo[current][index].m_iOptionValue[4] = info->ValueTable[4];
	}

	//--------------------------------------------------
	current = 1;

	for (int i = 0; i < MAX_SOCKET_ITEM_OPTION; i++)
	{
		SOCKET_ITEM_BONUS_OPTION_INFO* info = &this->m_SocketItemBonusOptionInfo[i];

		int index = info->Index;

		if (index == -1)
			continue;

		m_SocketOptionInfo[current][index].m_iOptionID = info->Index;

		m_SocketOptionInfo[current][index].m_iOptionCategory = 0;

		strcpy_s(m_SocketOptionInfo[current][index].m_szOptionName, info->Name);

		m_SocketOptionInfo[current][index].m_bOptionType = 1;

		m_SocketOptionInfo[current][index].m_iOptionValue[0] = info->Value;
	}

	//--------------------------------------------------
	current = 2;

	for (int i = 0; i < MAX_SOCKET_ITEM_OPTION; i++)
	{
		SOCKET_ITEM_PACKAGE_OPTION_INFO* info = &this->m_SocketItemPackageOptionInfo[i];

		int index = info->Index;

		if (index == -1)
			continue;

		m_SocketOptionInfo[current][index].m_iOptionID = info->Index;

		m_SocketOptionInfo[current][index].m_iOptionCategory = 0;

		strcpy_s(m_SocketOptionInfo[current][index].m_szOptionName, info->Name);

		m_SocketOptionInfo[current][index].m_bOptionType = 2;

		m_SocketOptionInfo[current][index].m_iOptionValue[0] = info->Value;

		m_SocketOptionInfo[current][index].m_bySocketCheckInfo[0] = info->RequireOptionTypeCountTable[0];

		m_SocketOptionInfo[current][index].m_bySocketCheckInfo[1] = info->RequireOptionTypeCountTable[1];

		m_SocketOptionInfo[current][index].m_bySocketCheckInfo[2] = info->RequireOptionTypeCountTable[2];

		m_SocketOptionInfo[current][index].m_bySocketCheckInfo[3] = info->RequireOptionTypeCountTable[3];

		m_SocketOptionInfo[current][index].m_bySocketCheckInfo[4] = info->RequireOptionTypeCountTable[4];

		m_SocketOptionInfo[current][index].m_bySocketCheckInfo[5] = info->RequireOptionTypeCountTable[5];
	}

	PackFileEncrypt(filename.c_str(), (BYTE*)m_SocketOptionInfo, MaxLine, Size, 0xA2F1, false, false);
}

void CSocketItemOption::SetInfo(SOCKET_ITEM_OPTION_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_SOCKET_ITEM_OPTION)
	{
		return;
	}

	this->m_SocketItemOptionInfo[info.Index] = info;
}

void CSocketItemOption::SetBonusInfo(SOCKET_ITEM_BONUS_OPTION_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_SOCKET_ITEM_OPTION)
	{
		return;
	}

	this->m_SocketItemBonusOptionInfo[info.Index] = info;
}

void CSocketItemOption::SetPackageInfo(SOCKET_ITEM_PACKAGE_OPTION_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_SOCKET_ITEM_OPTION)
	{
		return;
	}

	this->m_SocketItemPackageOptionInfo[info.Index] = info;
}

int CSocketItemOption::CalciOptionType(int index)
{
	int OptionType = 1;

	switch (index)
	{
	case SOCKET_ITEM_OPTION_SUB_BP_CONSUMPTION_RATE:
	case SOCKET_ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE:
	case SOCKET_ITEM_OPTION_ADD_ATTACK_SUCCESS_RATE:
	case SOCKET_ITEM_OPTION_ADD_ITEM_DURABILITY_RATE:
	case SOCKET_ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE:
	case SOCKET_ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE:
		OptionType = 2;
		break;
	case SOCKET_ITEM_OPTION_ADD_DAMAGE_BY_LEVEL:
		OptionType = 3;
		break;
	case SOCKET_ITEM_OPTION_ADD_HUNT_HP:
		OptionType = 4;
		break;
	case SOCKET_ITEM_OPTION_ADD_HUNT_MP:
		OptionType = 5;
		break;
	default:
		OptionType = 1;
		break;
	}
	return OptionType;
}

SOCKET_ITEM_OPTION_INFO* CSocketItemOption::GetInfo(int index) // OK
{
	if (index < 0 || index >= MAX_SOCKET_ITEM_OPTION)
	{
		return 0;
	}

	if (this->m_SocketItemOptionInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_SocketItemOptionInfo[index];
}

SOCKET_ITEM_BONUS_OPTION_INFO* CSocketItemOption::GetBonusInfo(int index) // OK
{
	if (index < 0 || index >= MAX_SOCKET_ITEM_OPTION)
	{
		return 0;
	}

	if (this->m_SocketItemBonusOptionInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_SocketItemBonusOptionInfo[index];
}

SOCKET_ITEM_PACKAGE_OPTION_INFO* CSocketItemOption::GetPackageInfo(int index) // OK
{
	if (index < 0 || index >= MAX_SOCKET_ITEM_OPTION)
	{
		return 0;
	}

	if (this->m_SocketItemPackageOptionInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_SocketItemPackageOptionInfo[index];
}

bool CSocketItemOption::IsSocketItem(CItem* lpItem) // OK
{
	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TABLE; n++)
	{
		if (lpItem->m_SocketOption[n] != SOCKET_ITEM_OPTION_NONE)
		{
			return 1;
		}
	}

	return 0;
}

int CSocketItemOption::GetSocketItemOptionCount(CItem* lpItem) // OK
{
	int count = 0;

	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TABLE; n++)
	{
		if (lpItem->m_SocketOption[n] != SOCKET_ITEM_OPTION_NONE)
		{
			count++;
		}
	}

	return count;
}

int CSocketItemOption::GetSocketItemExtraMoney(CItem* lpItem) // OK
{
	int money = 0;

	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TABLE; n++)
	{
		if (lpItem->m_SocketOption[n] != SOCKET_ITEM_OPTION_NONE && lpItem->m_SocketOption[n] != SOCKET_ITEM_OPTION_EMPTY)
		{
			int ItemOptionType = lpItem->m_SocketOption[n] % MAX_SOCKET_ITEM_OPTION;
			int ItemOptionLevel = lpItem->m_SocketOption[n] / MAX_SOCKET_ITEM_OPTION;

			SOCKET_ITEM_OPTION_INFO* lpInfoA = this->GetInfo(ItemOptionType);

			if (lpInfoA == 0)
			{
				continue;
			}

			int ItemIndex = (GET_ITEM(12, 100) + (ItemOptionLevel * 6) + (lpInfoA->Type - 1));

			ITEM_INFO ItemInfo;

			if (gItemManager.GetInfo(ItemIndex, &ItemInfo) != 0)
			{
				money += ItemInfo.BuyMoney;
			}
		}
	}

	return money;
}

int CSocketItemOption::GetSocketItemSeedOption(int index, int type) // OK
{
	SOCKET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(index);

	if (lpInfo == 0)
	{
		return -1;
	}

	if (lpInfo->Type != type)
	{
		return -1;
	}

	return lpInfo->SubIndex;
}

int CSocketItemOption::GetSocketItemRandomOption(int type) // OK
{
	int SocketItemOption = -1;

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION; n++)
	{
		SOCKET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(n);

		if (lpInfo == 0)
		{
			continue;
		}

		if (lpInfo->Type != type)
		{
			continue;
		}

		RandomManager.AddElement(lpInfo->Index, lpInfo->Rate);
	}

	if (RandomManager.GetRandomElement(&SocketItemOption) != 0)
	{
		return SocketItemOption;
	}
	else
	{
		return -1;
	}
}

BYTE CSocketItemOption::GetSocketItemOption(CItem* lpItem, int type, int option, int level) // OK
{
	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION; n++)
	{
		SOCKET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(n);

		if (lpInfo == 0)
		{
			continue;
		}

		if (lpInfo->Type != type)
		{
			continue;
		}

		if (lpInfo->SubIndex != option)
		{
			continue;
		}

		if (this->CheckSocketItemOption(lpInfo->Index, lpItem) == 0)
		{
			continue;
		}

		return (lpInfo->Index + (level * MAX_SOCKET_ITEM_OPTION));
	}

	return SOCKET_ITEM_OPTION_NONE;
}

BYTE CSocketItemOption::GetSocketItemBonusOption(CItem* lpItem) // OK
{
	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION; n++)
	{
		SOCKET_ITEM_BONUS_OPTION_INFO* lpBonusInfo = this->GetBonusInfo(n);

		if (lpBonusInfo == 0)
		{
			continue;
		}

		if (this->CheckSocketItemBonusOption(lpBonusInfo->Index, lpItem) == 0)
		{
			continue;
		}

		return lpBonusInfo->Index;
	}

	return SOCKET_ITEM_BONUS_OPTION_NONE;
}

bool CSocketItemOption::CheckSocketItemOption(int index, CItem* lpItem) // OK
{
	SOCKET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(index);

	if (lpInfo == 0)
	{
		return 0;
	}

	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TABLE; n++)
	{
		if (lpItem->m_SocketOption[n] != SOCKET_ITEM_OPTION_NONE && lpItem->m_SocketOption[n] != SOCKET_ITEM_OPTION_EMPTY)
		{
			if ((lpItem->m_SocketOption[n] % MAX_SOCKET_ITEM_OPTION) == lpInfo->Index)
			{
				return 0;
			}
		}
	}

	if (lpItem->m_Index >= GET_ITEM(0, 0) && lpItem->m_Index < GET_ITEM(6, 0) && lpItem->m_Index != GET_ITEM(4, 7) && lpItem->m_Index != GET_ITEM(4, 15))
	{
		if (lpInfo->Type == SOCKET_ITEM_OPTION_TYPE_FIRE || lpInfo->Type == SOCKET_ITEM_OPTION_TYPE_ICE || lpInfo->Type == SOCKET_ITEM_OPTION_TYPE_LIGHTNING)
		{
			return 1;
		}
	}

	if (lpItem->m_Index >= GET_ITEM(6, 0) && lpItem->m_Index < GET_ITEM(12, 0))
	{
		if (lpInfo->Type == SOCKET_ITEM_OPTION_TYPE_WATER || lpInfo->Type == SOCKET_ITEM_OPTION_TYPE_WIND || lpInfo->Type == SOCKET_ITEM_OPTION_TYPE_EARTH)
		{
			return 1;
		}
	}

	return 0;
}

bool CSocketItemOption::CheckSocketItemBonusOption(int index, CItem* lpItem) // OK
{
	SOCKET_ITEM_BONUS_OPTION_INFO* lpBonusInfo = this->GetBonusInfo(index);

	if (lpBonusInfo == 0)
	{
		return 0;
	}

	if (lpBonusInfo->MinItemSection > (lpItem->m_Index / MAX_ITEM_TYPE) || lpBonusInfo->MaxItemSection < (lpItem->m_Index / MAX_ITEM_TYPE))
	{
		return 0;
	}

	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TABLE; n++)
	{
		if (lpBonusInfo->RequireOptionTypeTable[n] == SOCKET_ITEM_OPTION_TYPE_NONE)
		{
			continue;
		}

		if (lpItem->m_SocketOption[n] == SOCKET_ITEM_OPTION_NONE || lpItem->m_SocketOption[n] == SOCKET_ITEM_OPTION_EMPTY)
		{
			return 0;
		}

		SOCKET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(lpItem->m_SocketOption[n] % MAX_SOCKET_ITEM_OPTION);

		if (lpInfo == 0)
		{
			return 0;
		}

		if (lpInfo->Type != lpBonusInfo->RequireOptionTypeTable[n])
		{
			return 0;
		}
	}

	return 1;
}

bool CSocketItemOption::CheckSocketItemPackageOption(int index, int* ItemOptionTypeCount) // OK
{
	SOCKET_ITEM_PACKAGE_OPTION_INFO* lpPackageInfo = this->GetPackageInfo(index);

	if (lpPackageInfo == 0)
	{
		return 0;
	}

	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION_TYPE; n++)
	{
		if (lpPackageInfo->RequireOptionTypeCountTable[n] > ItemOptionTypeCount[n])
		{
			return 0;
		}
	}

	return 1;
}

void CSocketItemOption::CalcSocketItemOption(LPOBJ lpObj, bool flag) // OK
{
	int ItemOptionTypeCount[MAX_SOCKET_ITEM_OPTION_TYPE] = { 0,0,0,0,0,0 };

	int _inv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 237, 238, 239, 240, 241, 242, -1 };

	for (int j = 0; _inv[j] != -1; j++)
	{
		int n = _inv[j];

		if (lpObj->Inventory[n].IsItem() == 0 || lpObj->Inventory[n].IsSocketItem() == 0)
		{
			continue;
		}

		for (int i = 0; i < MAX_SOCKET_ITEM_OPTION_TABLE; i++)
		{
			if (lpObj->Inventory[n].m_SocketOption[i] != SOCKET_ITEM_OPTION_NONE && lpObj->Inventory[n].m_SocketOption[i] != SOCKET_ITEM_OPTION_EMPTY)
			{
				int ItemOption = lpObj->Inventory[n].m_SocketOption[i] % MAX_SOCKET_ITEM_OPTION;

				SOCKET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(ItemOption);

				if (lpInfo == 0)
				{
					continue;
				}

				int ItemOptionLevel = lpObj->Inventory[n].m_SocketOption[i] / MAX_SOCKET_ITEM_OPTION;

				if (ItemOptionLevel < 0 || ItemOptionLevel >= MAX_SOCKET_ITEM_OPTION_TABLE)
				{
					continue;
				}

				this->InsertOption(lpObj, lpInfo->Index, lpInfo->ValueTable[ItemOptionLevel], flag);

				ItemOptionTypeCount[(lpInfo->Type - 1)]++;
			}
		}

		if (lpObj->Inventory[n].m_SocketOptionBonus != SOCKET_ITEM_BONUS_OPTION_NONE)
		{
			SOCKET_ITEM_BONUS_OPTION_INFO* lpBonusInfo = this->GetBonusInfo(lpObj->Inventory[n].m_SocketOptionBonus);

			if (lpBonusInfo == 0)
			{
				continue;
			}

			this->InsertBonusOption(lpObj, lpBonusInfo->Index, lpBonusInfo->Value, flag);
		}
	}

	this->CalcSocketItemPackageOption(lpObj, ItemOptionTypeCount, flag);
}

void CSocketItemOption::CalcSocketItemPackageOption(LPOBJ lpObj, int* ItemOptionTypeCount, bool flag) // OK
{
	for (int n = 0; n < MAX_SOCKET_ITEM_OPTION; n++)
	{
		SOCKET_ITEM_PACKAGE_OPTION_INFO* lpPackageInfo = this->GetPackageInfo(n);

		if (lpPackageInfo == 0)
		{
			continue;
		}

		if (this->CheckSocketItemPackageOption(lpPackageInfo->Index, ItemOptionTypeCount) == 0)
		{
			continue;
		}

		this->InsertPackageOption(lpObj, lpPackageInfo->Index, lpPackageInfo->Value, flag);
	}
}

void CSocketItemOption::InsertOption(LPOBJ lpObj, int index, int value, bool flag) // OK
{
	if (flag == 0)
	{
		if (index == SOCKET_ITEM_OPTION_ADD_STRENGTH || index == SOCKET_ITEM_OPTION_ADD_DEXTERITY || index == SOCKET_ITEM_OPTION_ADD_VITALITY || index == SOCKET_ITEM_OPTION_ADD_ENERGY)
		{
			return;
		}
	}
	else
	{
		if (index != SOCKET_ITEM_OPTION_ADD_STRENGTH && index != SOCKET_ITEM_OPTION_ADD_DEXTERITY && index != SOCKET_ITEM_OPTION_ADD_VITALITY && index != SOCKET_ITEM_OPTION_ADD_ENERGY)
		{
			return;
		}
	}

	switch (index)
	{
	case SOCKET_ITEM_OPTION_ADD_DAMAGE_BY_LEVEL:
		lpObj->PhysiDamageMinLeft += (value == 0 ? 0 : (lpObj->Level / value));
		lpObj->PhysiDamageMinRight += (value == 0 ? 0 : (lpObj->Level / value));
		lpObj->PhysiDamageMaxLeft += (value == 0 ? 0 : (lpObj->Level / value));
		lpObj->PhysiDamageMaxRight += (value == 0 ? 0 : (lpObj->Level / value));
		lpObj->MagicDamageMin += (value == 0 ? 0 : (lpObj->Level / value));
		lpObj->MagicDamageMax += (value == 0 ? 0 : (lpObj->Level / value));
		lpObj->CurseDamageMin += (value == 0 ? 0 : (lpObj->Level / value));
		lpObj->CurseDamageMax += (value == 0 ? 0 : (lpObj->Level / value));
		break;
	case SOCKET_ITEM_OPTION_ADD_SPEED:
		lpObj->PhysiSpeed += value;
		lpObj->MagicSpeed += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_MIN_DAMAGE:
		lpObj->PhysiDamageMinLeft += value;
		lpObj->PhysiDamageMinRight += value;
		lpObj->MagicDamageMin += value;
		lpObj->CurseDamageMin += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_MAX_DAMAGE:
		lpObj->PhysiDamageMaxLeft += value;
		lpObj->PhysiDamageMaxRight += value;
		lpObj->MagicDamageMax += value;
		lpObj->CurseDamageMax += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_DAMAGE:
		lpObj->PhysiDamageMinLeft += value;
		lpObj->PhysiDamageMinRight += value;
		lpObj->PhysiDamageMaxLeft += value;
		lpObj->PhysiDamageMaxRight += value;
		lpObj->MagicDamageMin += value;
		lpObj->MagicDamageMax += value;
		lpObj->CurseDamageMin += value;
		lpObj->CurseDamageMax += value;
		break;
	case SOCKET_ITEM_OPTION_SUB_BP_CONSUMPTION_RATE:
		lpObj->BPConsumptionRate -= value;
		break;
	case SOCKET_ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE:
		lpObj->DefenseSuccessRate += (lpObj->DefenseSuccessRate * value) / 100;
		break;
	case SOCKET_ITEM_OPTION_ADD_DEFENSE:
		lpObj->Defense += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_SHIELD_DAMAGE_REDUCTION:
		lpObj->ShieldDamageReduction += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_DAMAGE_REDUCTION:
		lpObj->DamageReduction[DAMAGE_REDUCTION_SOCKET_ITEM] += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_DAMAGE_REFLECT:
		lpObj->DamageReflect += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_HUNT_HP:
		lpObj->HuntHP += (value == 0 ? 0 : (100 / value));
		break;
	case SOCKET_ITEM_OPTION_ADD_HUNT_MP:
		lpObj->HuntMP += (value == 0 ? 0 : (100 / value));
		break;
	case SOCKET_ITEM_OPTION_ADD_SKILL_DAMAGE:
		lpObj->SkillDamageBonus += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_ATTACK_SUCCESS_RATE:
		lpObj->AttackSuccessRate += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_ITEM_DURABILITY_RATE:
		lpObj->WeaponDurabilityRate += value;
		lpObj->ArmorDurabilityRate += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_HP_RECOVERY:
		lpObj->HPRecovery += value;
		break;
	case SOCKET_ITEM_OPTION_MUL_MAX_HP:
		lpObj->AddLife += (int)((lpObj->MaxLife * value) / 100);
		break;
	case SOCKET_ITEM_OPTION_MUL_MAX_MP:
		lpObj->AddMana += (int)((lpObj->MaxMana * value) / 100);
		break;
	case SOCKET_ITEM_OPTION_ADD_MP_RECOVERY:
		lpObj->MPRecovery += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_MAX_BP:
		lpObj->AddBP += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_BP_RECOVERY:
		lpObj->BPRecovery += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_EXCELLENT_DAMAGE:
		lpObj->ExcellentDamage += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE:
		lpObj->ExcellentDamageRate += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_CRITICAL_DAMAGE:
		lpObj->CriticalDamage += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE:
		lpObj->CriticalDamageRate += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_STRENGTH:
		lpObj->AddStrength += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_DEXTERITY:
		lpObj->AddDexterity += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_VITALITY:
		lpObj->AddVitality += value;
		break;
	case SOCKET_ITEM_OPTION_ADD_ENERGY:
		lpObj->AddEnergy += value;
		break;
	}
}

void CSocketItemOption::InsertBonusOption(LPOBJ lpObj, int index, int value, bool flag) // OK
{
	if (flag != 0)
	{
		return;
	}

	switch (index)
	{
	case SOCKET_ITEM_WEAPON_BONUS_OPTION_ADD_DAMAGE:
		lpObj->PhysiDamageMinLeft += value;
		lpObj->PhysiDamageMinRight += value;
		lpObj->PhysiDamageMaxLeft += value;
		lpObj->PhysiDamageMaxRight += value;
		lpObj->MagicDamageMin += value;
		lpObj->MagicDamageMax += value;
		break;
	case SOCKET_ITEM_WEAPON_BONUS_OPTION_ADD_SKILL_DAMAGE:
		lpObj->SkillDamageBonus += value;
		break;
	case SOCKET_ITEM_STAFF_BONUS_OPTION_ADD_DAMAGE:
		lpObj->PhysiDamageMinLeft += value;
		lpObj->PhysiDamageMinRight += value;
		lpObj->PhysiDamageMaxLeft += value;
		lpObj->PhysiDamageMaxRight += value;
		lpObj->MagicDamageMin += value;
		lpObj->MagicDamageMax += value;
		break;
	case SOCKET_ITEM_STAFF_BONUS_OPTION_ADD_SKILL_DAMAGE:
		lpObj->SkillDamageBonus += value;
		break;
	case SOCKET_ITEM_ARMOR_BONUS_OPTION_ADD_DEFENSE:
		lpObj->Defense += value;
		break;
	case SOCKET_ITEM_ARMOR_BONUS_OPTION_ADD_MAX_HP:
		lpObj->AddLife += value;
		break;
	}
}

void CSocketItemOption::InsertPackageOption(LPOBJ lpObj, int index, int value, bool flag) // OK
{
	if (flag != 0)
	{
		return;
	}

	switch (index)
	{
	case SOCKET_ITEM_PACKAGE_OPTION_ADD_DOUBLE_DAMAGE_RATE:
		lpObj->DoubleDamageRate += value;
		break;
	case SOCKET_ITEM_PACKAGE_OPTION_ADD_IGNORE_DEFENSE_RATE:
		lpObj->IgnoreDefenseRate += value;
		break;
	}
}
