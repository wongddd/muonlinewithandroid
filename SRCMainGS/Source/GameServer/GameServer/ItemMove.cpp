// ItemMove.cpp: implementation of the CItemMove class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemMove.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "Util.h"

CItemMove gItemMove;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemMove::CItemMove() // OK
{
	this->m_ItemMoveInfo.clear();
}

CItemMove::~CItemMove() // OK
{

}

void CItemMove::Load(char* path) // OK
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

	this->m_ItemMoveInfo.clear();

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

			ITEM_MOVE_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.AllowDrop = lpMemScript->GetAsNumber();

			info.AllowSell = lpMemScript->GetAsNumber();

			info.AllowTrade = lpMemScript->GetAsNumber();

			info.AllowVault = lpMemScript->GetAsNumber();

			this->m_ItemMoveInfo.insert(type_move_item::value_type(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CItemMove::ExportXML(std::string filename)
{
	pugi::xml_node leaf;

	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("ItemMove");

	for (type_move_item::iterator it = m_ItemMoveInfo.begin(); it != m_ItemMoveInfo.end(); it++)
	{
		ITEM_MOVE_INFO* info = &it->second;

		if (info)
		{
			leaf = root.append_child("Info");

			leaf.append_attribute("Index").set_value(info->Index);

			leaf.append_attribute("BanDrop").set_value(info->AllowDrop);

			leaf.append_attribute("BanSell").set_value(info->AllowSell);

			leaf.append_attribute("BanTrade").set_value(info->AllowTrade);

			leaf.append_attribute("BanVaul").set_value(info->AllowVault);

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

bool CItemMove::CheckItemMoveAllowDrop(int index) // OK
{
	std::map<int,ITEM_MOVE_INFO>::iterator it = this->m_ItemMoveInfo.find(index);

	if(it == this->m_ItemMoveInfo.end())
	{
		return 1;
	}
	else
	{
		return ((it->second.AllowDrop==0)?0:1);
	}
}

bool CItemMove::CheckItemMoveAllowSell(int index) // OK
{
	std::map<int,ITEM_MOVE_INFO>::iterator it = this->m_ItemMoveInfo.find(index);

	if(it == this->m_ItemMoveInfo.end())
	{
		return 1;
	}
	else
	{
		return ((it->second.AllowSell==0)?0:1);
	}
}

bool CItemMove::CheckItemMoveAllowTrade(int index) // OK
{
	std::map<int,ITEM_MOVE_INFO>::iterator it = this->m_ItemMoveInfo.find(index);

	if(it == this->m_ItemMoveInfo.end())
	{
		return 1;
	}
	else
	{
		return ((it->second.AllowTrade==0)?0:1);
	}
}

bool CItemMove::CheckItemMoveAllowVault(int index) // OK
{
	std::map<int,ITEM_MOVE_INFO>::iterator it = this->m_ItemMoveInfo.find(index);

	if(it == this->m_ItemMoveInfo.end())
	{
		return 1;
	}
	else
	{
		return ((it->second.AllowVault==0)?0:1);
	}
}
