// ItemValue.cpp: implementation of the CItemValue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemValue.h"
#include "ItemManager.h"
#include "ItemStack.h"
#include "MemScript.h"
#include "Util.h"

CItemValue gItemValue;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemValue::CItemValue() // OK
{
	this->m_ItemValueInfo.clear();
}

CItemValue::~CItemValue() // OK
{

}

void CItemValue::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_ItemValueInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			ITEM_VALUE_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.Level = lpMemScript->GetAsNumber();

			info.Grade = lpMemScript->GetAsNumber();

			info.Value = lpMemScript->GetAsNumber();

			info.Coin1 = lpMemScript->GetAsNumber();

			info.Coin2 = lpMemScript->GetAsNumber();

			info.Coin3 = lpMemScript->GetAsNumber();

			info.Sell  = lpMemScript->GetAsNumber();

			this->m_ItemValueInfo.push_back(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CItemValue::ExportXML(std::string filename)
{
	pugi::xml_node leaf;

	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("ItemValue");

	for (int i = 0; i < m_ItemValueInfo.size(); i++)
	{
		ITEM_VALUE_INFO* info = &m_ItemValueInfo[i];

		if (info)
		{
			leaf = root.append_child("Info");

			leaf.append_attribute("Index").set_value(info->Index);

			leaf.append_attribute("Level").set_value(info->Level);

			leaf.append_attribute("Grade").set_value(info->Grade);

			leaf.append_attribute("Value").set_value(info->Value);

			leaf.append_attribute("Coin1").set_value(info->Coin1);

			leaf.append_attribute("Coin2").set_value(info->Coin2);

			leaf.append_attribute("Coin3").set_value(info->Coin3);

			leaf.append_attribute("Sell").set_value(info->Sell);

			if (info->Index != -1)
			{
				leaf.append_attribute("Comment").set_value(gItemManager.GetItemName(info->Index));
			}
			else
			{
				leaf.append_attribute("Comment").set_value(" ");
			}
		}
	}

	doc.save_file(filename.c_str());
}

bool CItemValue::GetItemValue(CItem* lpItem,int* value) // OK
{
	for(std::vector<ITEM_VALUE_INFO>::iterator it=this->m_ItemValueInfo.begin();it != this->m_ItemValueInfo.end();it++)
	{
		if(it->Index == lpItem->m_Index)
		{
			if(it->Level == -1 || it->Level == lpItem->m_Level)
			{
				if(it->Grade == -1 || it->Grade == lpItem->m_NewOption)
				{
					if(gItemStack.GetItemMaxStack(it->Index, lpItem->m_Level) == 0 || it->Index == GET_ITEM(4,7) || it->Index == GET_ITEM(4,15))
					{
						(*value) = it->Value;
						return 1;
					}
					else
					{
						(*value) = (int)(it->Value*lpItem->m_Durability);
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

bool CItemValue::GetItemValueNew(CItem* lpItem,int* value,int* coin1,int* coin2,int* coin3,int* sell) // OK
{
	for(std::vector<ITEM_VALUE_INFO>::iterator it=this->m_ItemValueInfo.begin();it != this->m_ItemValueInfo.end();it++)
	{
		if(it->Index == lpItem->m_Index)
		{
			if(it->Level == lpItem->m_Level && it->Grade == lpItem->m_NewOption)
			{
					(*value) = (int)(it->Value);
					(*coin1) = it->Coin1;
					(*coin2) = it->Coin2;
					(*coin3) = it->Coin3;
					(*sell)  = it->Sell;
					return 1;
			}

			if(it->Level == lpItem->m_Level && it->Grade == -1)
			{
					(*value) = (int)(it->Value);
					(*coin1) = it->Coin1;
					(*coin2) = it->Coin2;
					(*coin3) = it->Coin3;
					(*sell)  = it->Sell;
					return 1;
			}

			if(it->Level == -1 && it->Grade == lpItem->m_NewOption)
			{
				(*value) = (int)(it->Value);
				(*coin1) = it->Coin1;
				(*coin2) = it->Coin2;
				(*coin3) = it->Coin3;
				(*sell)  = it->Sell;
				return 1;
			}
		}
	}

	for(std::vector<ITEM_VALUE_INFO>::iterator it=this->m_ItemValueInfo.begin();it != this->m_ItemValueInfo.end();it++)
	{
		if(it->Index == lpItem->m_Index)
		{
			if(it->Level == -1 || it->Level == lpItem->m_Level)
			{
				if(it->Grade == -1 || it->Grade == lpItem->m_NewOption)
				{
					(*value) = (int)(it->Value);
					(*coin1) = it->Coin1;
					(*coin2) = it->Coin2;
					(*coin3) = it->Coin3;
					(*sell)  = it->Sell;
					return 1;
				}
			}
		}
	}

	return 0;
}
