// ItemOptionManager.cpp: implementation of the CItemOptionManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemOptionRate.h"
#include "CustomWing.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "PentagramSystem.h"
#include "RandomManager.h"
#include "SetItemType.h"
#include "SocketItemType.h"
#include "Util.h"

CItemOptionRate gItemOptionRate;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemOptionRate::CItemOptionRate() // OK
{
	this->m_ItemOption0RateInfo.clear();

	this->m_ItemOption1RateInfo.clear();

	this->m_ItemOption2RateInfo.clear();

	this->m_ItemOption3RateInfo.clear();

	this->m_ItemOption4RateInfo.clear();

	this->m_ItemOption5RateInfo.clear();

	this->m_ItemOption6RateInfo.clear();
}

CItemOptionRate::~CItemOptionRate() // OK
{

}

void CItemOptionRate::Load(char* path) // OK
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

	this->m_ItemOption0RateInfo.clear();

	this->m_ItemOption1RateInfo.clear();

	this->m_ItemOption2RateInfo.clear();

	this->m_ItemOption3RateInfo.clear();

	this->m_ItemOption4RateInfo.clear();

	this->m_ItemOption5RateInfo.clear();

	this->m_ItemOption6RateInfo.clear();

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

					ITEM_OPTION0_RATE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for (int n = 0; n < MAX_ITEM_OPTION0_RATE; n++)
					{
						info.Rate[n] = lpMemScript->GetAsNumber();
					}

					this->m_ItemOption0RateInfo.insert(std::pair<int, ITEM_OPTION0_RATE_INFO>(info.Index, info));
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_OPTION1_RATE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for (int n = 0; n < MAX_ITEM_OPTION1_RATE; n++)
					{
						info.Rate[n] = lpMemScript->GetAsNumber();
					}

					this->m_ItemOption1RateInfo.insert(std::pair<int, ITEM_OPTION1_RATE_INFO>(info.Index, info));
				}
				else if (section == 2)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_OPTION2_RATE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for (int n = 0; n < MAX_ITEM_OPTION2_RATE; n++)
					{
						info.Rate[n] = lpMemScript->GetAsNumber();
					}

					this->m_ItemOption2RateInfo.insert(std::pair<int, ITEM_OPTION2_RATE_INFO>(info.Index, info));
				}
				else if (section == 3)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_OPTION3_RATE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for (int n = 0; n < MAX_ITEM_OPTION3_RATE; n++)
					{
						info.Rate[n] = lpMemScript->GetAsNumber();
					}

					this->m_ItemOption3RateInfo.insert(std::pair<int, ITEM_OPTION3_RATE_INFO>(info.Index, info));
				}
				else if (section == 4)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_OPTION4_RATE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for (int n = 0; n < MAX_ITEM_OPTION4_RATE; n++)
					{
						info.Rate[n] = lpMemScript->GetAsNumber();
					}

					this->m_ItemOption4RateInfo.insert(std::pair<int, ITEM_OPTION4_RATE_INFO>(info.Index, info));
				}
				else if (section == 5)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_OPTION5_RATE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for (int n = 0; n < MAX_ITEM_OPTION5_RATE; n++)
					{
						info.Rate[n] = lpMemScript->GetAsNumber();
					}

					this->m_ItemOption5RateInfo.insert(std::pair<int, ITEM_OPTION5_RATE_INFO>(info.Index, info));
				}
				else if (section == 6)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_OPTION6_RATE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for (int n = 0; n < MAX_ITEM_OPTION6_RATE; n++)
					{
						info.Rate[n] = lpMemScript->GetAsNumber();
					}

					this->m_ItemOption6RateInfo.insert(std::pair<int, ITEM_OPTION6_RATE_INFO>(info.Index, info));
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

void CItemOptionRate::ExportXML(std::string filename)
{
	pugi::xml_node leaf;

	pugi::xml_document doc;

	pugi::xml_node root;

	root = doc.append_child("Option0");

	for (type_option_rate0::iterator it = m_ItemOption0RateInfo.begin(); it != m_ItemOption0RateInfo.end(); it++)
	{
		ITEM_OPTION0_RATE_INFO* s = &it->second;

		leaf = root.append_child("info");

		leaf.append_attribute("Index").set_value(s->Index);

		for (int i = 0; i < MAX_ITEM_OPTION0_RATE; i++)
		{
			char name[64];
			sprintf_s(name, "Rate%d", i);
			leaf.append_attribute(name).set_value(s->Rate[i]);
		}
		leaf.append_attribute("Comment").set_value(" ");
	}
	//---------------------------------
	root = doc.append_child("Option1");

	for (type_option_rate1::iterator it = m_ItemOption1RateInfo.begin(); it != m_ItemOption1RateInfo.end(); it++)
	{
		ITEM_OPTION1_RATE_INFO* s = &it->second;

		leaf = root.append_child("info");

		leaf.append_attribute("Index").set_value(s->Index);

		for (int i = 0; i < MAX_ITEM_OPTION1_RATE; i++)
		{
			char name[64];
			sprintf_s(name, "Rate%d", i);
			leaf.append_attribute(name).set_value(s->Rate[i]);
		}
		leaf.append_attribute("Comment").set_value(" ");
	}
	//---------------------------------
	root = doc.append_child("Option2");

	for (type_option_rate2::iterator it = m_ItemOption2RateInfo.begin(); it != m_ItemOption2RateInfo.end(); it++)
	{
		ITEM_OPTION2_RATE_INFO* s = &it->second;

		leaf = root.append_child("info");

		leaf.append_attribute("Index").set_value(s->Index);

		for (int i = 0; i < MAX_ITEM_OPTION2_RATE; i++)
		{
			char name[64];
			sprintf_s(name, "Rate%d", i);
			leaf.append_attribute(name).set_value(s->Rate[i]);
		}
		leaf.append_attribute("Comment").set_value(" ");
	}
	//---------------------------------
	root = doc.append_child("Option3");

	for (type_option_rate3::iterator it = m_ItemOption3RateInfo.begin(); it != m_ItemOption3RateInfo.end(); it++)
	{
		ITEM_OPTION3_RATE_INFO* s = &it->second;

		leaf = root.append_child("info");

		leaf.append_attribute("Index").set_value(s->Index);

		for (int i = 0; i < MAX_ITEM_OPTION3_RATE; i++)
		{
			char name[64];
			sprintf_s(name, "Rate%d", i);
			leaf.append_attribute(name).set_value(s->Rate[i]);
		}
		leaf.append_attribute("Comment").set_value(" ");
	}
	//---------------------------------
	root = doc.append_child("Option4");

	for (type_option_rate4::iterator it = m_ItemOption4RateInfo.begin(); it != m_ItemOption4RateInfo.end(); it++)
	{
		ITEM_OPTION4_RATE_INFO* s = &it->second;

		leaf = root.append_child("info");

		leaf.append_attribute("Index").set_value(s->Index);

		for (int i = 0; i < MAX_ITEM_OPTION4_RATE; i++)
		{
			char name[64];
			sprintf_s(name, "Rate%d", i);
			leaf.append_attribute(name).set_value(s->Rate[i]);
		}
		leaf.append_attribute("Comment").set_value(" ");
	}
	//---------------------------------
	root = doc.append_child("Option5");

	for (type_option_rate5::iterator it = m_ItemOption5RateInfo.begin(); it != m_ItemOption5RateInfo.end(); it++)
	{
		ITEM_OPTION5_RATE_INFO* s = &it->second;

		leaf = root.append_child("info");

		leaf.append_attribute("Index").set_value(s->Index);

		for (int i = 0; i < MAX_ITEM_OPTION5_RATE; i++)
		{
			char name[64];
			sprintf_s(name, "Rate%d", i);
			leaf.append_attribute(name).set_value(s->Rate[i]);
		}
		leaf.append_attribute("Comment").set_value(" ");
	}
	//---------------------------------
	root = doc.append_child("Option6");

	for (type_option_rate6::iterator it = m_ItemOption6RateInfo.begin(); it != m_ItemOption6RateInfo.end(); it++)
	{
		ITEM_OPTION6_RATE_INFO* s = &it->second;

		leaf = root.append_child("info");

		leaf.append_attribute("Index").set_value(s->Index);

		for (int i = 0; i < MAX_ITEM_OPTION6_RATE; i++)
		{
			char name[64];
			sprintf_s(name, "Rate%d", i);
			leaf.append_attribute(name).set_value(s->Rate[i]);
		}
		leaf.append_attribute("Comment").set_value(" ");
	}

	doc.save_file(filename.c_str());
}

bool CItemOptionRate::GetItemOption0(int index, BYTE* option) // OK
{
	std::map<int, ITEM_OPTION0_RATE_INFO>::iterator it = this->m_ItemOption0RateInfo.find(index);

	if (it == this->m_ItemOption0RateInfo.end())
	{
		return 0;
	}

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_ITEM_OPTION0_RATE; n++)
	{
		RandomManager.AddElement(n, it->second.Rate[n]);
	}

	return RandomManager.GetRandomElement(option);
}

bool CItemOptionRate::GetItemOption1(int index, BYTE* option) // OK
{
	std::map<int, ITEM_OPTION1_RATE_INFO>::iterator it = this->m_ItemOption1RateInfo.find(index);

	if (it == this->m_ItemOption1RateInfo.end())
	{
		return 0;
	}

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_ITEM_OPTION1_RATE; n++)
	{
		RandomManager.AddElement(n, it->second.Rate[n]);
	}

	return RandomManager.GetRandomElement(option);
}

bool CItemOptionRate::GetItemOption2(int index, BYTE* option) // OK
{
	std::map<int, ITEM_OPTION2_RATE_INFO>::iterator it = this->m_ItemOption2RateInfo.find(index);

	if (it == this->m_ItemOption2RateInfo.end())
	{
		return 0;
	}

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_ITEM_OPTION2_RATE; n++)
	{
		RandomManager.AddElement(n, it->second.Rate[n]);
	}

	return RandomManager.GetRandomElement(option);
}

bool CItemOptionRate::GetItemOption3(int index, BYTE* option) // OK
{
	std::map<int, ITEM_OPTION3_RATE_INFO>::iterator it = this->m_ItemOption3RateInfo.find(index);

	if (it == this->m_ItemOption3RateInfo.end())
	{
		return 0;
	}

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_ITEM_OPTION3_RATE; n++)
	{
		RandomManager.AddElement(n, it->second.Rate[n]);
	}

	return RandomManager.GetRandomElement(option);
}

bool CItemOptionRate::GetItemOption4(int index, BYTE* option) // OK
{
	std::map<int, ITEM_OPTION4_RATE_INFO>::iterator it = this->m_ItemOption4RateInfo.find(index);

	if (it == this->m_ItemOption4RateInfo.end())
	{
		return 0;
	}

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_ITEM_OPTION4_RATE; n++)
	{
		RandomManager.AddElement(n, it->second.Rate[n]);
	}

	return RandomManager.GetRandomElement(option);
}

bool CItemOptionRate::GetItemOption5(int index, BYTE* option) // OK
{
	std::map<int, ITEM_OPTION5_RATE_INFO>::iterator it = this->m_ItemOption5RateInfo.find(index);

	if (it == this->m_ItemOption5RateInfo.end())
	{
		return 0;
	}

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_ITEM_OPTION5_RATE; n++)
	{
		RandomManager.AddElement(n, it->second.Rate[n]);
	}

	return RandomManager.GetRandomElement(option);
}

bool CItemOptionRate::GetItemOption6(int index, BYTE* option) // OK
{
	std::map<int, ITEM_OPTION6_RATE_INFO>::iterator it = this->m_ItemOption6RateInfo.find(index);

	if (it == this->m_ItemOption6RateInfo.end())
	{
		return 0;
	}

	CRandomManager RandomManager;

	for (int n = 0; n < MAX_ITEM_OPTION6_RATE; n++)
	{
		RandomManager.AddElement(n, it->second.Rate[n]);
	}

	return RandomManager.GetRandomElement(option);
}

bool CItemOptionRate::MakeNewOption(int ItemIndex, int value, BYTE* option) // OK
{
	(*option) = 0;

	int count = 0;

	int MaxNewOption = MAX_EXC_OPTION;

	if ((ItemIndex >= GET_ITEM(12, 3) && ItemIndex <= GET_ITEM(12, 6)) || ItemIndex == GET_ITEM(12, 42) || ItemIndex == GET_ITEM(12, 49) || ItemIndex == GET_ITEM(13, 30)) // 2sd Wings
	{
		MaxNewOption = ((ItemIndex == GET_ITEM(13, 30)) ? 4 : 3);
	}

	if ((ItemIndex >= GET_ITEM(12, 36) && ItemIndex <= GET_ITEM(12, 40)) || ItemIndex == GET_ITEM(12, 43) || ItemIndex == GET_ITEM(12, 50)) // 3rd Wings
	{
		MaxNewOption = 4;
	}

	if (ItemIndex >= GET_ITEM(12, 262) && ItemIndex <= GET_ITEM(12, 265)) // Monster Wings
	{
		MaxNewOption = 2;
	}

	if (ItemIndex == GET_ITEM(12, 266)) // Wings of Conqueror
	{
		MaxNewOption = 3;
	}

	if (ItemIndex == GET_ITEM(12, 267)) // Wings of Angel and Devil
	{
		MaxNewOption = 3;
	}

	if (gCustomWing.CheckCustomWingByItem(ItemIndex) != 0)
	{
		MaxNewOption = 4;
	}

	if (ItemIndex == GET_ITEM(13, 3)) // Dinorant
	{
		MaxNewOption = 3;
	}

	if (ItemIndex == GET_ITEM(13, 37)) // Fenrir
	{
		MaxNewOption = 3;
	}

	while (true)
	{
		if (count >= value || count >= MaxNewOption)
		{
			break;
		}

		int value = 1 << (GetLargeRand() % MaxNewOption);

		if (((*option) & value) == 0)
		{
			(*option) |= value;
			count++;
		}
	}

	return 1;
}

bool CItemOptionRate::MakeSetOption(int ItemIndex, int SetOption, BYTE* option) // OK
{
	if (gSetItemType.CheckSetItemType(ItemIndex) == 0 || SetOption == 0)
	{
		(*option) = 0;
		return 0;
	}

	(*option) = 0;

	for (int n = 0; n < MAX_SET_ITEM_OPTION_INDEX; n++)
	{
		if (gSetItemType.GetSetItemOptionIndex(ItemIndex, n) > 0)
		{
			if ((n == 0 && (SetOption == 1 || SetOption == 3 || SetOption == 4))
				|| (n == 1 && (SetOption == 2 || SetOption == 5 || SetOption == 6))
				)
			{
				(*option) = (((*option) == 0) ? (n + 1) : (((GetLargeRand() % 2) == 0) ? (n + 1) : (*option)));
			}
		}
	}

	//if (SetOption == 1 || SetOption == 3 || SetOption == 5)
	//{
	//	(*option) |= (((*option) > 0) ? 4 : 0);
	//}
	//else if (SetOption == 2 || SetOption == 4 || SetOption == 6)
	//{
	//	(*option) |= (((*option) > 0) ? 8 : 0);
	//}
	//else
	//{
	//	(*option) |= (((*option) > 0) ? (4 * SetOption) : 0);
	//}

	(*option) |= (((*option) > 0) ? (4 << (SetOption % 2)) : 0);

	return 1;
}

bool CItemOptionRate::MakeSocketOption(int ItemIndex, int value, BYTE* option) // OK
{
	if ((gSocketItemType.CheckSocketItemType(ItemIndex) == 0 && gPentagramSystem.IsPentagramItem(ItemIndex) == 0) || value == 0 || value == 0xFF)
	{
		memset(option, 0xFF, MAX_SOCKET_OPTION);
		return 0;
	}

	if (gSocketItemType.CheckSocketItemType(ItemIndex) == 0)
	{
		for (int n = 0; n < ((value > MAX_SOCKET_OPTION) ? MAX_SOCKET_OPTION : value); n++)
		{
			option[n] = 0xFE;
		}
	}
	else
	{
		for (int n = 0; n < ((value > gSocketItemType.GetSocketItemMaxSocket(ItemIndex)) ? gSocketItemType.GetSocketItemMaxSocket(ItemIndex) : value); n++)
		{
			option[n] = 0xFE;
		}
	}

	return 1;
}
