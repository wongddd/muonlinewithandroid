// 380ItemType.cpp: implementation of the C380ItemType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "380ItemType.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "380ItemOption.h"
#include "Util.h"

C380ItemType g380ItemType;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C380ItemType::C380ItemType() // OK
{
	this->m_380ItemTypeInfo.clear();
}

C380ItemType::~C380ItemType() // OK
{

}

void C380ItemType::Load(char* path) // OK
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

	this->m_380ItemTypeInfo.clear();

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

			ITEM_380_TYPE_INFO info;

			memset(&info, 0, sizeof(info));

			info.Index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(), lpMemScript->GetAsNumber()));

			for (int n = 0; n < MAX_380_ITEM_OPTION_INDEX; n++)
			{
				info.OptionIndex[n] = lpMemScript->GetAsNumber();

				info.OptionValue[n] = lpMemScript->GetAsNumber();
			}

			this->m_380ItemTypeInfo.insert(type_map_option380::value_type(info.Index, info));
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void C380ItemType::ExportXML(std::string filename)
{
	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("ItemType380");

	for (type_map_option380::iterator it = m_380ItemTypeInfo.begin(); it != m_380ItemTypeInfo.end(); it++)
	{
		ITEM_380_TYPE_INFO* s = &it->second;

		pugi::xml_node leaf = root.append_child("info");

		leaf.append_attribute("Section").set_value((int)(s->Index / 512));

		leaf.append_attribute("Index").set_value((int)(s->Index % 512));

		leaf.append_attribute("OptionIndex1").set_value(s->OptionIndex[0]);

		leaf.append_attribute("OptionValue1").set_value(s->OptionValue[0]);

		leaf.append_attribute("OptionIndex2").set_value(s->OptionIndex[1]);

		leaf.append_attribute("OptionValue2").set_value(s->OptionValue[1]);

		if (s->Index != -1 )
		{
			leaf.append_attribute("Comment").set_value(gItemManager.GetItemName(s->Index));
		}
		else
		{
			leaf.append_attribute("Comment").set_value(" ");
		}
	}

	doc.save_file(filename.c_str());
}

void C380ItemType::ExportBMD(std::string filename)
{
	int MaxLine = 8192;
	int Size = sizeof(ITEM_ADD_OPTION);

	ITEM_ADD_OPTION* m_ItemAddOption = new ITEM_ADD_OPTION[MaxLine];

	memset(m_ItemAddOption, 0, (Size* MaxLine));

	for (type_map_option380::iterator it = m_380ItemTypeInfo.begin(); it != m_380ItemTypeInfo.end(); it++)
	{
		ITEM_380_TYPE_INFO* s = &it->second;

		int index = s->Index;

		if (index == -1)
			continue;

		m_ItemAddOption[index].m_byOption1 = s->OptionIndex[0];

		m_ItemAddOption[index].m_byOption2 = s->OptionIndex[1];

		if (s->OptionValue[0] != -1)
			m_ItemAddOption[index].m_byValue1 = s->OptionValue[0];
		else
			m_ItemAddOption[index].m_byValue1 = g380ItemOption.GetValue(s->OptionIndex[0]);

		if (s->OptionValue[1] != -1)
			m_ItemAddOption[index].m_byValue2 = s->OptionValue[1];
		else
			m_ItemAddOption[index].m_byValue2 = g380ItemOption.GetValue(s->OptionIndex[1]);

		m_ItemAddOption[index].m_Type = 0;

		m_ItemAddOption[index].m_Time = 0;
	}

	PackFileEncrypt(filename.c_str(), (BYTE*)m_ItemAddOption, MaxLine, Size);

	delete[] m_ItemAddOption;
}

bool C380ItemType::Check380ItemType(int index) // OK
{
	type_map_option380::iterator it = this->m_380ItemTypeInfo.find(index);

	if (it == this->m_380ItemTypeInfo.end())
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int C380ItemType::Get380ItemOptionIndex(int index, int number) // OK
{
	type_map_option380::iterator it = this->m_380ItemTypeInfo.find(index);

	if (it == this->m_380ItemTypeInfo.end())
	{
		return -1;
	}
	else
	{
		return ((CHECK_RANGE(number, MAX_380_ITEM_OPTION_INDEX) == 0) ? -1 : it->second.OptionIndex[number]);
	}
}

int C380ItemType::Get380ItemOptionValue(int index, int number) // OK
{
	type_map_option380::iterator it = this->m_380ItemTypeInfo.find(index);

	if (it == this->m_380ItemTypeInfo.end())
	{
		return -1;
	}
	else
	{
		return ((CHECK_RANGE(number, MAX_380_ITEM_OPTION_INDEX) == 0) ? -1 : it->second.OptionValue[number]);
	}
}
