// ItemStack.cpp: implementation of the CItemStack class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemStack.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "Util.h"

CItemStack gItemStack;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemStack::CItemStack() // OK
{
	this->m_ItemStackInfo.clear();
}

CItemStack::~CItemStack() // OK
{

}

void CItemStack::Load(char* path) // OK
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

	this->m_ItemStackInfo.clear();

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

			ITEM_STACK_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.Level = lpMemScript->GetAsNumber();

			info.MaxStack = lpMemScript->GetAsNumber();

			info.CreateItemIndex = lpMemScript->GetAsNumber();

			info.break_up = lpMemScript->GetAsNumber();

			this->m_ItemStackInfo.push_back(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CItemStack::ExportXML(std::string filename)
{
	pugi::xml_node leaf;

	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("ItemStack");

	for (size_t i = 0; i < m_ItemStackInfo.size(); i++)
	{
		ITEM_STACK_INFO* info = &m_ItemStackInfo[i];

		if (info)
		{
			leaf = root.append_child("Info");

			leaf.append_attribute("Index").set_value(info->Index);

			leaf.append_attribute("Level").set_value(info->Level);

			leaf.append_attribute("MaxStack").set_value(info->MaxStack);

			leaf.append_attribute("CreateItemIndex").set_value(info->CreateItemIndex);

			leaf.append_attribute("BreakUp").set_value(info->break_up);

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

int CItemStack::GetItemMaxStack(int index, int Level) // OK
{
	ITEM_STACK_INFO* cstack = this->find(index, Level);

	if (cstack != NULL)
	{
		return cstack->MaxStack;
	}

	return 0;
}

int CItemStack::GetCreateItemIndex(int index, int Level) // OK
{
	ITEM_STACK_INFO* cstack = this->find(index, Level);

	if (cstack != NULL)
	{
		return cstack->CreateItemIndex;
	}

	return -1;
}

int CItemStack::IsBreakUp(int index, int Level)
{
	ITEM_STACK_INFO* cstack = this->find(index, Level);

	if (cstack != NULL)
	{
		return cstack->break_up != 0;
	}

	return 0;
}

ITEM_STACK_INFO* CItemStack::find(int _index, int _level)
{
	auto it = std::find_if(m_ItemStackInfo.begin(), m_ItemStackInfo.end(),
		[_index, _level](const ITEM_STACK_INFO& info) {
			return _index == info.Index && (info.Level == -1 || info.Level == _level);
		});

	return (it != m_ItemStackInfo.end()) ? &(*it) : NULL;
}
