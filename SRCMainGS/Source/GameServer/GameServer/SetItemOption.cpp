// SetItemOption.cpp: implementation of the CSetItemOption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DefaultClassInfo.h"
#include "SetItemOption.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "SetItemType.h"
#include "Util.h"

CSetItemOption gSetItemOption;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSetItemOption::CSetItemOption() // OK
{
	this->Init();
}

CSetItemOption::~CSetItemOption() // OK
{

}

void CSetItemOption::Init() // OK
{
	for (int n = 0; n < MAX_SET_ITEM_OPTION; n++)
	{
		this->m_SetItemOptionInfo[n].Index = -1;
	}
}

void CSetItemOption::Load(char* path) // OK
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

			if (strcmp("end", lpMemScript->GetString()) == 0)
			{
				break;
			}

			SET_ITEM_OPTION_INFO info;

			memset(&info, 0, sizeof(info));

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Name, lpMemScript->GetAsString());

			for (int n = 0; n < MAX_SET_ITEM_OPTION_TABLE; n++)
			{
				for (int i = 0; i < MAX_SET_ITEM_OPTION_INDEX; i++)
				{
					info.OptionTable[n][i].Index = lpMemScript->GetAsNumber();
					info.OptionTable[n][i].Value = lpMemScript->GetAsNumber();
				}
			}

			for (int o = 0; o < MAX_SET_ITEM_FULL_OPTION_TABLE; o++)
			{
				info.FullOptionTable[o].Index = lpMemScript->GetAsNumber();
				info.FullOptionTable[o].Value = lpMemScript->GetAsNumber();
			}

			this->SetInfo(info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CSetItemOption::SetInfo(SET_ITEM_OPTION_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_SET_ITEM_OPTION)
	{
		return;
	}

	this->m_SetItemOptionInfo[info.Index] = info;
}

void CSetItemOption::ExportXML(std::string filename)
{

}

void CSetItemOption::ExportBMD(std::string filename)
{
	int MaxLine = 256;
	int Size = sizeof(ITEM_SET_OPTION);

	ITEM_SET_OPTION* m_ItemSetOption = new ITEM_SET_OPTION[MaxLine];

	memset(m_ItemSetOption, -1, (sizeof(ITEM_SET_OPTION) * MaxLine));

	for (int i = 0; i < MaxLine; i++)
	{
		SET_ITEM_OPTION_INFO* info = this->GetInfo(i);

		memset(m_ItemSetOption[i].strSetName, 0, (sizeof(char) * 64));

		if (info)
		{
			strcpy_s(m_ItemSetOption[i].strSetName, info->Name);

			m_ItemSetOption[i].byOptionCount = 2;

			for (int n = 0; n < MAX_SET_ITEM_OPTION_TABLE; n++)
			{
				m_ItemSetOption[i].byStandardOption[n][0] = info->OptionTable[n][0].Index;
				m_ItemSetOption[i].byStandardOptionValue[n][0] = info->OptionTable[n][0].Value;

				m_ItemSetOption[i].byStandardOption[n][1] = info->OptionTable[n][1].Index;
				m_ItemSetOption[i].byStandardOptionValue[n][1] = info->OptionTable[n][1].Value;

				if (info->OptionTable[n][0].Index != -1 || info->OptionTable[n][1].Index != -1)
				{
					m_ItemSetOption[i].byOptionCount++;
				}
			}

			for (int n = 0; n < MAX_SET_ITEM_FULL_OPTION_TABLE; n++)
			{
				m_ItemSetOption[i].byFullOption[n] = info->FullOptionTable[n].Index;
				m_ItemSetOption[i].byFullOptionValue[n] = info->FullOptionTable[n].Value;
			}

			m_ItemSetOption[i].byExtOption[0] = -1;
			m_ItemSetOption[i].byExtOption[1] = -1;
			m_ItemSetOption[i].byExtOptionValue[0] = -1;
			m_ItemSetOption[i].byExtOptionValue[1] = -1;

			memset(m_ItemSetOption[i].byRequireClass, 0, (sizeof(BYTE) * MAX_CLASS));
		}
	}

	PackFileEncrypt(filename.c_str(), (BYTE*)m_ItemSetOption, MaxLine, Size, 0xA2F1, false);

	delete[] m_ItemSetOption;
}

SET_ITEM_OPTION_INFO* CSetItemOption::GetInfo(int index) // OK
{
	if (index < 0 || index >= MAX_SET_ITEM_OPTION)
	{
		return 0;
	}

	if (this->m_SetItemOptionInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_SetItemOptionInfo[index];
}

bool CSetItemOption::IsSetItem(CItem* lpItem) // OK
{
	if ((lpItem->m_SetOption & 3) == 0)
	{
		return 0;
	}

	return 1;
}

int CSetItemOption::GetSetItemMaxOptionCount(int index) // OK
{
	SET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(index);

	if (lpInfo == 0)
	{
		return 0;
	}

	int count = 0;

	for (int n = 0; n < MAX_SET_ITEM_OPTION_TABLE; n++)
	{
		for (int i = 0; i < MAX_SET_ITEM_OPTION_INDEX; i++)
		{
			if (lpInfo->OptionTable[n][i].Index != -1)
			{
				count++;
				break;
			}
		}
	}

	return count;
}

int CSetItemOption::GetInventorySetItemOptionCount(LPOBJ lpObj, int index) // OK
{
	int OptionCount = 0;
	int WeaponOptionIndex = 0;
	int RingOptionIndex = 0;

	int _inv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 237, 238, 239, 240, 241, 242, -1 };

	for (int i = 0; _inv[i] != -1; i++)
	{
		int n = _inv[i];

		CItem* pItem = &lpObj->Inventory[n];

		if (pItem->IsItem() == 0 || pItem->IsSetItem() == 0 || pItem->m_IsValidItem == 0)
		{
			continue;
		}

		if (gSetItemType.CheckSetItemType(pItem->m_Index) == 0)
		{
			continue;
		}

		int OptionIndex = gSetItemType.GetSetItemOptionIndex(pItem->m_Index, ((pItem->m_SetOption & 3) - 1));

		if (OptionIndex == 0 || OptionIndex != index)
		{
			continue;
		}

		if (n == EQUIPMENT_WEAPON_R || n == EQUIPMENT_WEAPON_L)
		{
			if (WeaponOptionIndex == 0)
			{
				WeaponOptionIndex = OptionIndex;
			}
			else if (WeaponOptionIndex == OptionIndex && lpObj->Inventory[EQUIPMENT_WEAPON_R].m_Index == lpObj->Inventory[EQUIPMENT_WEAPON_L].m_Index)
			{
				continue;
			}
		}
		else if (n == EQUIPMENT_RING_R || n == EQUIPMENT_RING_L)
		{
			if (RingOptionIndex == 0)
			{
				RingOptionIndex = OptionIndex;
			}
			else if (RingOptionIndex == OptionIndex && lpObj->Inventory[EQUIPMENT_RING_R].m_Index == lpObj->Inventory[EQUIPMENT_RING_L].m_Index)
			{
				continue;
			}
		}
		OptionCount++;
	}

	return OptionCount;
}

void CSetItemOption::CalcSetItemStat(LPOBJ lpObj) // OK
{
	int _inv[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 237, 238, 239, 240, 241, 242, -1 };

	for (int i = 0; _inv[i] != -1; i++)
	{
		int n = _inv[i];

		CItem* pItem = &lpObj->Inventory[n];

		if (pItem->IsItem() == 0 || pItem->IsSetItem() == 0)
		{
			continue;
		}

		if (gSetItemType.CheckSetItemType(pItem->m_Index) == 0)
		{
			continue;
		}

		int ItemOption = gSetItemType.GetSetItemStatType(pItem->m_Index);

		if (ItemOption == -1)
		{
			continue;
		}

		this->InsertOption(lpObj, ItemOption, (((pItem->m_SetOption >> 2) & 3) * 5), 1);
	}
}

void CSetItemOption::CalcSetItemOption(LPOBJ lpObj, bool flag) // OK
{
	lpObj->IsFullSetItem = 0;

	for (int n = 0; n < MAX_SET_ITEM_OPTION; n++)
	{
		SET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(n);

		if (lpInfo == 0)
		{
			continue;
		}

		int ItemOptionCount = this->GetInventorySetItemOptionCount(lpObj, lpInfo->Index);

		if (ItemOptionCount == 0)
		{
			continue;
		}

		for (int i = 0; i < (ItemOptionCount - 1); i++)
		{
			for (int o = 0; o < MAX_SET_ITEM_OPTION_INDEX; o++)
			{
				if (lpInfo->OptionTable[i][o].Index != -1)
				{
					this->InsertOption(lpObj, lpInfo->OptionTable[i][o].Index, lpInfo->OptionTable[i][o].Value, flag);
				}
			}
		}

		if ((ItemOptionCount - 1) == this->GetSetItemMaxOptionCount(lpInfo->Index))
		{
			for (int o = 0; o < MAX_SET_ITEM_FULL_OPTION_TABLE; o++)
			{
				if (lpInfo->FullOptionTable[o].Index != -1)
				{
					this->InsertOption(lpObj, lpInfo->FullOptionTable[o].Index, lpInfo->FullOptionTable[o].Value, flag);
				}
			}
			lpObj->IsFullSetItem = 1;
		}
	}
}

void CSetItemOption::InsertOption(LPOBJ lpObj, int index, int value, bool flag) // OK
{
	if (flag == 0)
	{
		if (index == SET_ITEM_OPTION_ADD_STRENGTH || index == SET_ITEM_OPTION_ADD_DEXTERITY || index == SET_ITEM_OPTION_ADD_VITALITY || index == SET_ITEM_OPTION_ADD_ENERGY || index == SET_ITEM_OPTION_ADD_LEADERSHIP)
		{
			return;
		}
	}
	else
	{
		if (index != SET_ITEM_OPTION_ADD_STRENGTH && index != SET_ITEM_OPTION_ADD_DEXTERITY && index != SET_ITEM_OPTION_ADD_VITALITY && index != SET_ITEM_OPTION_ADD_ENERGY && index != SET_ITEM_OPTION_ADD_LEADERSHIP)
		{
			return;
		}
	}

	switch (index)
	{
	case SET_ITEM_OPTION_ADD_STRENGTH:
		lpObj->AddStrength += value;
		break;
	case SET_ITEM_OPTION_ADD_DEXTERITY:
		lpObj->AddDexterity += value;
		break;
	case SET_ITEM_OPTION_ADD_VITALITY:
		lpObj->AddVitality += value;
		break;
	case SET_ITEM_OPTION_ADD_ENERGY:
		lpObj->AddEnergy += value;
		break;
	case SET_ITEM_OPTION_ADD_LEADERSHIP:
		lpObj->AddLeadership += value;
		break;
	case SET_ITEM_OPTION_ADD_MIN_PHYSI_DAMAGE:
		lpObj->PhysiDamageMinLeft += value;
		lpObj->PhysiDamageMinRight += value;
		break;
	case SET_ITEM_OPTION_ADD_MAX_PHYSI_DAMAGE:
		lpObj->PhysiDamageMaxLeft += value;
		lpObj->PhysiDamageMaxRight += value;
		break;
	case SET_ITEM_OPTION_MUL_MAGIC_DAMAGE:
		lpObj->MagicDamageMin += (lpObj->MagicDamageMin * value) / 100;
		lpObj->MagicDamageMax += (lpObj->MagicDamageMax * value) / 100;
		break;
	case SET_ITEM_OPTION_ADD_PHYSI_DAMAGE:
		lpObj->PhysiDamageMinLeft += value;
		lpObj->PhysiDamageMinRight += value;
		lpObj->PhysiDamageMaxLeft += value;
		lpObj->PhysiDamageMaxRight += value;
		break;
	case SET_ITEM_OPTION_MUL_ATTACK_SUCCESS_RATE:
		lpObj->AttackSuccessRate += (lpObj->AttackSuccessRate * value) / 100;
		break;
	case SET_ITEM_OPTION_ADD_DEFENSE:
		lpObj->Defense += value;
		break;
	case SET_ITEM_OPTION_ADD_MAX_LIFE:
		lpObj->AddLife += value;
		break;
	case SET_ITEM_OPTION_ADD_MAX_MANA:
		lpObj->AddMana += value;
		break;
	case SET_ITEM_OPTION_ADD_MAX_BP:
		lpObj->AddBP += value;
		break;
	case SET_ITEM_OPTION_ADD_BP_RECOVERY:
		lpObj->BPRecovery += value;
		break;
	case SET_ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE:
		lpObj->CriticalDamageRate += value;
		break;
	case SET_ITEM_OPTION_ADD_CRITICAL_DAMAGE:
		lpObj->CriticalDamage += value;
		break;
	case SET_ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE:
		lpObj->ExcellentDamageRate += value;
		break;
	case SET_ITEM_OPTION_ADD_EXCELLENT_DAMAGE:
		lpObj->ExcellentDamage += value;
		break;
	case SET_ITEM_OPTION_ADD_SKILL_DAMAGE:
		lpObj->SkillDamageBonus += value;
		break;
	case SET_ITEM_OPTION_ADD_DOUBLE_DAMAGE_RATE:
		lpObj->DoubleDamageRate += value;
		break;
	case SET_ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE:
		lpObj->IgnoreDefenseRate += value;
		break;
	case SET_ITEM_OPTION_MUL_SHIELD_DEFENSE:
		if (lpObj->Inventory[1].m_Index >= GET_ITEM(6, 0) && lpObj->Inventory[1].m_Index < GET_ITEM(7, 0))
		{
			lpObj->Defense += (lpObj->Defense * value) / 100;
		}
		break;
	case SET_ITEM_OPTION_MUL_TWO_HAND_PHYSI_DAMAGE:
		if ((lpObj->Inventory[0].IsItem() != 0 && lpObj->Inventory[0].m_TwoHand != 0) || (lpObj->Inventory[1].IsItem() != 0 && lpObj->Inventory[1].m_TwoHand != 0))
		{
			lpObj->PhysiDamageMinLeft += (lpObj->PhysiDamageMinLeft * value) / 100;
			lpObj->PhysiDamageMinRight += (lpObj->PhysiDamageMinRight * value) / 100;
			lpObj->PhysiDamageMaxLeft += (lpObj->PhysiDamageMaxLeft * value) / 100;
			lpObj->PhysiDamageMaxRight += (lpObj->PhysiDamageMaxRight * value) / 100;
		}
		break;
	case SET_ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_STRENGTH:
		lpObj->PhysiDamageMinLeft += ((value == 0) ? 0 : (lpObj->Strength / value));
		lpObj->PhysiDamageMinRight += ((value == 0) ? 0 : (lpObj->Strength / value));
		lpObj->PhysiDamageMaxLeft += ((value == 0) ? 0 : (lpObj->Strength / value));
		lpObj->PhysiDamageMaxRight += ((value == 0) ? 0 : (lpObj->Strength / value));
		break;
	case SET_ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_DEXTERITY:
		lpObj->PhysiDamageMinLeft += ((value == 0) ? 0 : (lpObj->Dexterity / value));
		lpObj->PhysiDamageMinRight += ((value == 0) ? 0 : (lpObj->Dexterity / value));
		lpObj->PhysiDamageMaxLeft += ((value == 0) ? 0 : (lpObj->Dexterity / value));
		lpObj->PhysiDamageMaxRight += ((value == 0) ? 0 : (lpObj->Dexterity / value));
		break;
	case SET_ITEM_OPTION_ADD_DEFENSE_BY_DEXTERITY:
		lpObj->Defense += ((value == 0) ? 0 : (lpObj->Dexterity / value));
		break;
	case SET_ITEM_OPTION_ADD_DEFENSE_BY_VITALITY:
		lpObj->Defense += ((value == 0) ? 0 : (lpObj->Vitality / value));
		break;
	case SET_ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_ENERGY:
		lpObj->MagicDamageMin += ((value == 0) ? 0 : (lpObj->Energy / value));
		lpObj->MagicDamageMax += ((value == 0) ? 0 : (lpObj->Energy / value));
		break;
	case SET_ITEM_OPTION_ADD_ICE_DAMAGE:
		lpObj->AddResistance[0] += value;
		break;
	case SET_ITEM_OPTION_ADD_POISON_DAMAGE:
		lpObj->AddResistance[1] += value;
		break;
	case SET_ITEM_OPTION_ADD_LIGHTNING_DAMAGE:
		lpObj->AddResistance[2] += value;
		break;
	case SET_ITEM_OPTION_ADD_FIRE_DAMAGE:
		lpObj->AddResistance[3] += value;
		break;
	case SET_ITEM_OPTION_ADD_EARTH_DAMAGE:
		lpObj->AddResistance[4] += value;
		break;
	case SET_ITEM_OPTION_ADD_WIND_DAMAGE:
		lpObj->AddResistance[5] += value;
		break;
	case SET_ITEM_OPTION_ADD_WATER_DAMAGE:
		lpObj->AddResistance[6] += value;
		break;
	}
}

bool CSetItemOption::IsFullSet(PART_t* Inventory, size_t Element)
{
	bool IsFullSetItem = false;

	for (int n = 0; n < MAX_SET_ITEM_OPTION; n++)
	{
		SET_ITEM_OPTION_INFO* lpInfo = this->GetInfo(n);

		if (lpInfo == 0)
		{
			continue;
		}

		int ItemOptionCount = this->GetInventorySetOptionCount(Inventory, Element, lpInfo->Index);

		if (ItemOptionCount == 0)
		{
			continue;
		}

		if ((ItemOptionCount - 1) == this->GetSetItemMaxOptionCount(lpInfo->Index))
		{
			IsFullSetItem = true;
		}
	}
	return IsFullSetItem;
}

int CSetItemOption::GetInventorySetOptionCount(PART_t* Inventory, size_t size, int index)
{
	int OptionCount = 0;
	int WeaponOptionIndex = 0;
	int RingOptionIndex = 0;

	for (size_t _index = 0; _index < size; _index++)
	{
		PART_t* f = &Inventory[_index];

		if (f->Type == 0x1FFF)
		{
			continue;
		}

		if (gSetItemType.CheckSetItemType(f->Type) == 0)
		{
			continue;
		}

		int OptionIndex = gSetItemType.GetSetItemOptionIndex(f->Type, ((f->ExtOption & 3) - 1));

		if (OptionIndex == 0 || OptionIndex != index)
		{
			continue;
		}

		if (_index == EQUIPMENT_WEAPON_R || _index == EQUIPMENT_WEAPON_L)
		{
			if (WeaponOptionIndex == 0)
			{
				WeaponOptionIndex = OptionIndex;
			}
			else if (WeaponOptionIndex == OptionIndex && Inventory[EQUIPMENT_WEAPON_R].Type == Inventory[EQUIPMENT_WEAPON_L].Type)
			{
				continue;
			}
		}
		else if (_index == EQUIPMENT_RING_R || _index == EQUIPMENT_RING_L)
		{
			if (RingOptionIndex == 0)
			{
				RingOptionIndex = OptionIndex;
			}
			else if (RingOptionIndex == OptionIndex && Inventory[EQUIPMENT_RING_R].Type == Inventory[EQUIPMENT_RING_L].Type)
			{
				continue;
			}
		}

		OptionCount++;
	}

	return OptionCount;
}
