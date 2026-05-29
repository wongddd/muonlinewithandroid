#include "StdAfx.h"
#include "User.h"
#include "Util.h"
#include "MemScript.h"
#include "ItemOption.h"
#include "ItemManager.h"
#include "CGMPetManager.h"

CGMPetManager::CGMPetManager()
{
}

CGMPetManager::~CGMPetManager()
{
	this->m_ItemOption.clear();
}

CGMPetManager* CGMPetManager::Instance()
{
	static CGMPetManager s_Instance;
	return &s_Instance;
}

void CGMPetManager::Load(char* filename)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(filename);

	this->m_ItemOption.clear();

	if (res.status != pugi::status_ok)
	{
		ErrorMessageBox("Error loading file: %s\nDescription: %s\nOffset: %lld", filename, res.description(), res.offset);
		return;
	}

	pugi::xml_node root = file.child("ItemOption");

	for (pugi::xml_node child = root.child("Item"); child; child = child.next_sibling())
	{
		CUSTOM_PET_INFO info;

		int x = child.attribute("Type").as_int();

		int y = child.attribute("Index").as_int();

		info.ItemIndex = SafeGetItem(GET_ITEM(x, y));

		info.IncDamageRate = child.attribute("IncDamageRate").as_int();

		info.DecDamageRate = child.attribute("DecDamageRate").as_int();

		char attribute_name[100];

		for (size_t i = 0; i < MAX_ADD_OPTION_INFO; i++)
		{
			sprintf_s(attribute_name, "Opt%d", i + 1);
			info.Opt[i] = child.attribute(attribute_name).as_int();

			sprintf_s(attribute_name, "Val%d", i + 1);
			info.Val[i] = child.attribute(attribute_name).as_int();
		}

		this->m_ItemOption.insert(std::pair<int, CUSTOM_PET_INFO>(info.ItemIndex, info));
	}
}

bool CGMPetManager::CheckPetByItem(int ItemIndex)
{
	std::map<int, CUSTOM_PET_INFO>::iterator it = this->m_ItemOption.find(ItemIndex);

	return (it != this->m_ItemOption.end());
}

bool CGMPetManager::PetIsInmortal(int ItemIndex)
{
	std::map<int, CUSTOM_PET_INFO>::iterator it = this->m_ItemOption.find(ItemIndex);

	if (it != this->m_ItemOption.end())
	{
		return (it->second.Inmortal != 0);
	}

	return false;
}

int CGMPetManager::GetItemPetIncDamageRate(int ItemIndex)
{
	std::map<int, CUSTOM_PET_INFO>::iterator it = this->m_ItemOption.find(ItemIndex);

	if (it != this->m_ItemOption.end())
	{
		return it->second.IncDamageRate;
	}

	return 0;
}

int CGMPetManager::GetItemPetDecDamageRate(int ItemIndex)
{
	std::map<int, CUSTOM_PET_INFO>::iterator it = this->m_ItemOption.find(ItemIndex);

	if (it != this->m_ItemOption.end())
	{
		return it->second.DecDamageRate;
	}

	return 0;
}

void CGMPetManager::CalcItemCommonOption(LPOBJ lpObj, bool flag)
{
	int _inv[] = { EQUIPMENT_HELPER, EQUIPMENT_EAGLE, EQUIPMENT_MUUN, -1 };

	for (int i = 0; _inv[i] != -1; i++)
	{
		int n = _inv[i];

		CItem* pItem = &lpObj->Inventory[n];

		if (pItem->IsItem() != 0 && pItem->m_IsValidItem != 0 && pItem->m_Durability != 0)
		{
			this->InsertOption(lpObj, pItem, flag);
		}
	}
}

void CGMPetManager::InsertOption(LPOBJ lpObj, CItem* lpItem, bool flag)
{
	this->UpdateInfo(lpItem->m_Index);

	if (this->CurrentInfo != this->m_ItemOption.end())
	{
		for (size_t i = 0; i < MAX_ADD_OPTION_INFO; i++)
		{
			int index = this->GetIndex(i);

			int value = this->GetValue(i);

			if (index == 0 || value == 0)
				continue;

			gItemOption.AddOption(lpObj, index, value, lpItem, flag);
		}
	}
}

void CGMPetManager::UpdateInfo(int Index)
{
	this->CurrentInfo = this->m_ItemOption.find(Index);
}

int CGMPetManager::GetIndex(int index)
{
	if (this->CurrentInfo != this->m_ItemOption.end())
	{
		return this->CurrentInfo->second.Opt[index];
	}
	return 0;
}

int CGMPetManager::GetValue(int index)
{
	if (this->CurrentInfo != this->m_ItemOption.end())
	{
		return this->CurrentInfo->second.Val[index];
	}
	return 0;
}


