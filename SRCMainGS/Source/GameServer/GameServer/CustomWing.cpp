#include "stdafx.h"
#include "Util.h"
#include "MemScript.h"
#include "ItemOption.h"
#include "CustomWing.h"

CCustomWing gCustomWing;

CCustomWing::CCustomWing() // OK
{
	this->Init();
}

CCustomWing::~CCustomWing() // OK
{
}

void CCustomWing::Init() // OK
{
	m_nBackByItem.clear();
}

void CCustomWing::Load(char* path) // OK
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

			CUSTOM_WING_INFO info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.ItemIndex = nIndex + (nType << 9);

			info.DefenseConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstB = lpMemScript->GetAsNumber();

			info.DecDamageConstA = lpMemScript->GetAsNumber();

			info.DecDamageConstB = lpMemScript->GetAsNumber();

			for (size_t i = 0; i < 3; i++)
			{
				info.OptionIndex[i] = lpMemScript->GetAsNumber();

				info.OptionValue[i] = lpMemScript->GetAsNumber();
			}

			for (size_t i = 0; i < 4; i++)
			{
				info.NewOptionIndex[i] = lpMemScript->GetAsNumber();

				info.NewOptionValue[i] = lpMemScript->GetAsNumber();
			}

			lpMemScript->GetAsNumber();

			lpMemScript->GetAsString();

			this->Mng__Register(info.ItemIndex, info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CCustomWing::Mng__Register(int Index, CUSTOM_WING_INFO info) // OK
{
	m_nBackByItem.insert(std::pair<int, CUSTOM_WING_INFO>(info.ItemIndex, info));
}

CUSTOM_WING_INFO* CCustomWing::findWings007(int ItemIndex) // OK
{
	std::map<int, CUSTOM_WING_INFO>::iterator it = m_nBackByItem.find(ItemIndex);

	if (it != m_nBackByItem.end())
	{
		return (&it->second);
	}

	return NULL;
}

bool CCustomWing::CheckCustomWingByItem(int ItemIndex) // OK
{
	if (this->findWings007(ItemIndex) != 0)
	{
		return 1;
	}

	return 0;
}

int OpcionExcAdd(int m_nOptionR)
{
	switch (m_nOptionR)
	{
	case WINGS_ITEM_OPTION_ADD_PHYSI_DAMAGE:
		return ITEM_OPTION_ADD_PHYSI_DAMAGE;
	case WINGS_ITEM_OPTION_ADD_MAGIC_DAMAGE:
		return ITEM_OPTION_ADD_MAGIC_DAMAGE;
	case WINGS_ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE:
		return ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE;
	case WINGS_ITEM_OPTION_ADD_DEFENSE:
		return ITEM_OPTION_ADD_DEFENSE;
	case WINGS_ITEM_OPTION_ADD_HP_RECOVERY_RATE:
	case WINGS_ITEM_OPTION_ADD_HP_RECOVERY_RATE_:
		return ITEM_OPTION_ADD_HP_RECOVERY_RATE;
	case WINGS_ITEM_OPTION_ADD_MONEY_AMOUNT_DROP_RATE:
		return ITEM_OPTION_ADD_MONEY_AMOUNT_DROP_RATE;
	case WINGS_ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE:
		return ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE;
	case WINGS_ITEM_OPTION_ADD_DAMAGE_REFLECT:
		return ITEM_OPTION_ADD_DAMAGE_REFLECT;
	case WINGS_ITEM_OPTION_ADD_DAMAGE_REDUCTION:
		return ITEM_OPTION_ADD_DAMAGE_REDUCTION;
	case WINGS_ITEM_OPTION_MUL_MP:
		return ITEM_OPTION_MUL_MP;
	case WINGS_ITEM_OPTION_MUL_HP:
		return ITEM_OPTION_MUL_HP;
	case WINGS_ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE:
		return ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE;
	case WINGS_ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_LEVEL:
		return ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_LEVEL;
	case WINGS_ITEM_OPTION_MUL_PHYSI_DAMAGE:
		return ITEM_OPTION_MUL_PHYSI_DAMAGE;
	case WINGS_ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_LEVEL:
		return ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_LEVEL;
	case WINGS_ITEM_OPTION_MUL_MAGIC_DAMAGE:
		return ITEM_OPTION_MUL_MAGIC_DAMAGE;
	case WINGS_ITEM_OPTION_ADD_SPEED_MAGIC:
		return ITEM_OPTION_ADD_SPEED;
	case WINGS_ITEM_OPTION_ADD_HUNT_HP:
		return ITEM_OPTION_ADD_HUNT_HP;
	case WINGS_ITEM_OPTION_ADD_HUNT_MP:
		return ITEM_OPTION_ADD_HUNT_MP;
	case WINGS_ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE:
	case WINGS_ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE_:
		return ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE;
	case WINGS_ITEM_OPTION_ADD_BP:
		return ITEM_OPTION_ADD_BP;
	case WINGS_ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE:
		return ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE;
	case WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE:
	case WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE_:
		return ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE;
	case WINGS_ITEM_OPTION_ADD_DEFENSIVE_FULL_MP_RESTORE_RATE:
		return ITEM_OPTION_ADD_DEFENSIVE_FULL_MP_RESTORE_RATE;
	case WINGS_ITEM_OPTION_ADD_HP:
		return ITEM_OPTION_ADD_HP;
	case WINGS_ITEM_OPTION_ADD_MP:
		return ITEM_OPTION_ADD_MP;
	case WINGS_ITEM_OPTION_MUL_DEFENSE:
		return ITEM_OPTION_MUL_DEFENSE;
	default:
		break;
	}
	return m_nOptionR;
}