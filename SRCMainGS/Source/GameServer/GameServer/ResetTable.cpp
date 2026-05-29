// ResetTable.cpp: implementation of the CResetTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResetTable.h"
#include "Command.h"
#include "MemScript.h"
#include "ServerInfo.h"
#include "Util.h"
#include "Notice.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "ItemBagManager.h"
#include "CommandManager.h"

CResetTable gResetTable;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResetTable::CResetTable() // OK
{
	this->Init();
}

CResetTable::~CResetTable() // OK
{
	this->Init();
}

void CResetTable::Init()
{
	this->m_ResetTableInfo.clear();
	this->m_ResetTableRewardInfo.clear();
}

void CResetTable::Load(char* path) // OK
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

					RESET_TABLE_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.MinReset = lpMemScript->GetAsNumber();

					info.MaxReset = lpMemScript->GetAsNumber();

					info.Level[0] = lpMemScript->GetAsNumber();

					info.Level[1] = lpMemScript->GetAsNumber();

					info.Level[2] = lpMemScript->GetAsNumber();

					info.Level[3] = lpMemScript->GetAsNumber();

					info.Money[0] = lpMemScript->GetAsNumber();

					info.Money[1] = lpMemScript->GetAsNumber();

					info.Money[2] = lpMemScript->GetAsNumber();

					info.Money[3] = lpMemScript->GetAsNumber();

					for (int i = 0; i < MAX_ITEM_JEWEL_KOF; i++)
					{
						info.ItemCount[i] = lpMemScript->GetAsNumber();
					}

					info.ItemIndex[0] = GET_ITEM(14, 13); //-- Jewel of Bless

					info.ItemIndex[1] = GET_ITEM(14, 14); //-- Jewel of Soul

					info.ItemIndex[2] = GET_ITEM(14, 16); //-- Jewel of Life

					info.ItemIndex[3] = GET_ITEM(14, 22); //-- Jewel of Creation

					info.ItemIndex[4] = GET_ITEM(14, 31); //-- Jewel of Guardian

					info.ItemIndex[5] = GET_ITEM(14, 41); //-- Jewel of Gemstone

					info.ItemIndex[6] = GET_ITEM(14, 42); //-- Jewel of Harmony

					info.ItemIndex[7] = GET_ITEM(12, 15); //-- Jewel of Chaos

					info.ItemIndex[8] = GET_ITEM(12, 43); //-- Lower refining stone

					info.ItemIndex[9] = GET_ITEM(12, 44); //-- Higher refining stone

					this->m_ResetTableInfo.push_back(info);
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					RESET_TABLE_REWARD_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.Point[0] = lpMemScript->GetAsNumber();

					info.Point[1] = lpMemScript->GetAsNumber();

					info.Point[2] = lpMemScript->GetAsNumber();

					info.Point[3] = lpMemScript->GetAsNumber();

					info.PackageBag[0] = lpMemScript->GetAsNumber();

					info.PackageBag[1] = lpMemScript->GetAsNumber();

					info.PackageBag[2] = lpMemScript->GetAsNumber();

					info.PackageBag[3] = lpMemScript->GetAsNumber();

					m_ResetTableRewardInfo[info.Index] = info;
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

int CResetTable::GetResetMax(int resetCount)
{
	int resetLevel = resetCount;

	auto it = this->FindResetRange(resetCount);

	if (it != this->m_ResetTableInfo.end())
	{
		if (it->MaxReset != -1)
		{
			resetLevel = it->MaxReset;
		}
	}

	return resetLevel;
}

int CResetTable::GetResetLevel(int resetCount, int accountLevel) // OK
{
	int resetLevel = gServerInfo.m_CommandResetLevel[accountLevel];

	auto it = this->FindResetRange(resetCount);

	if (it != this->m_ResetTableInfo.end())
	{
		if (it->Level[accountLevel] != -1)
		{
			resetLevel = it->Level[accountLevel];
		}
	}

	return resetLevel;
}

int CResetTable::GetResetMoney(int resetCount, int accountLevel) // OK
{
	int resetMoney = gServerInfo.m_CommandResetMoney[accountLevel];

	auto it = this->FindResetRange(resetCount);

	if (it != this->m_ResetTableInfo.end())
	{
		if (it->Money[accountLevel] != -1)
		{
			resetMoney = it->Money[accountLevel];
		}
		else
		{
			COMMAND_LIST CommandData;

			if (gCommand.GetInfo(COMMAND_RESET - 1, &CommandData) && CommandData.Money[accountLevel] > 0)
			{
				resetMoney = CommandData.Money[accountLevel];
			}
		}
	}

	return resetMoney;
}

int CResetTable::GetResetJewel(int resetCount, int accountLevel, int* _ItemIndex, int* _ItemCount)
{
	auto it = this->FindResetRange(resetCount);

	if (it != this->m_ResetTableInfo.end())
	{
		for (int i = 0; i < MAX_ITEM_JEWEL_KOF; i++)
		{
			_ItemIndex[i] = it->ItemIndex[i];
			_ItemCount[i] = it->ItemCount[i];
		}
	}
	else
	{
		for (int i = 0; i < MAX_ITEM_JEWEL_KOF; i++)
		{
			_ItemIndex[i] = -1;
			_ItemCount[i] = 0;
		}
	}
	return 0;
}

int CResetTable::GetResetJewel(LPOBJ lpObj, int resetCount)
{
	auto it = this->FindResetRange(resetCount);

	if (it != this->m_ResetTableInfo.end())
	{
		for (int i = 0; i < MAX_ITEM_JEWEL_KOF; i++)
		{
			if (it->ItemIndex[i] != -1 && it->ItemCount[i] > 0)
			{
				gItemManager.DeleteInventoryItemCount(lpObj, it->ItemIndex[i], -1, it->ItemCount[i]);
			}
		}
	}

	return 1;
}

int CResetTable::CheckResetJewel(LPOBJ lpObj, int resetCount, bool NoticeSend)
{
	auto it = this->FindResetRange(resetCount);

	if (it != this->m_ResetTableInfo.end())
	{
		for (int i = 0; i < MAX_ITEM_JEWEL_KOF; i++)
		{
			if (it->ItemIndex[i] != -1 && it->ItemCount[i] > 0)
			{
				int Cnt = gItemManager.GetInventoryItemCount(lpObj, it->ItemIndex[i], -1);

				if (Cnt < it->ItemCount[i])
				{
					if (NoticeSend)
					{
						gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "Missing: %s quantity: %d ", gItemManager.GetItemName(it->ItemIndex[i]), it->ItemCount[i] - Cnt);
					}
					return 0;
				}
			}
		}
	}

	return 1;
}

int CResetTable::GetResetPoint(int resetCount, int accountLevel) // OK
{
	int point = gServerInfo.m_CommandResetPoint[accountLevel];

	auto it = this->FindResetRange(resetCount);

	// Si se encuentra un rango específico, se calcula los puntos para ese rango
	if (it != this->m_ResetTableInfo.end())
	{
		RESET_TABLE_REWARD_INFO* reward = this->GetTableInfo(it->Index);

		if (reward != NULL && reward->Point[accountLevel] != -1)
		{
			point = reward->Point[accountLevel];
		}
	}
	return (point * resetCount);
}

int CResetTable::GetResetPackage(LPOBJ lpObj, int resetCount, int accountLevel)
{
	auto it = this->FindResetRange(resetCount);

	if (it != this->m_ResetTableInfo.end())
	{
		RESET_TABLE_REWARD_INFO* reward = this->GetTableInfo(it->Index);

		if (reward != NULL && reward->PackageBag[accountLevel] != -1)
		{
			int index = reward->PackageBag[accountLevel];

			CItem item;

			if (gItemBagManager.GetItemBySpecialValue(index, lpObj, &item))
			{
				GDCreateItemSend(lpObj->Index, 0xFF, 0, 0, item.m_Index, (BYTE)item.m_Level, 0, item.m_Option1, item.m_Option2, item.m_Option3, -1, item.m_NewOption, item.m_SetOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption, item.m_SocketOptionBonus, 0);
			}
		}
	}
	return 0;
}

void CResetTable::send_player_info(LPOBJ lpObj, int resetCount, int accountLevel)
{
	PMSG_RESET_TABLE_INFO pMsg;
	pMsg.Head.setE(0x4E, 0x39, sizeof(PMSG_RESET_TABLE_INFO));

	pMsg.Reset = lpObj->Reset;
	pMsg.MaxReset = this->GetResetMax(resetCount);
	pMsg.MinLevel = this->GetResetLevel(resetCount, accountLevel);
	pMsg.ReqMoney = this->GetResetMoney(resetCount, accountLevel);
	pMsg.RewardPoint = this->GetResetPoint(resetCount, accountLevel);


	memset(pMsg.ItemCount, 0, sizeof(pMsg.ItemCount));
	memset(pMsg.ItemIndex, 0, sizeof(pMsg.ItemIndex));

	this->GetResetJewel(resetCount, accountLevel, pMsg.ItemIndex, pMsg.ItemCount);

	DataSend(lpObj->Index, (BYTE*)&pMsg, sizeof(pMsg));
}

RESET_TABLE_REWARD_INFO* CResetTable::GetTableInfo(int index)
{
	std::map<int, RESET_TABLE_REWARD_INFO>::iterator li = m_ResetTableRewardInfo.find(index);

	if (li != m_ResetTableRewardInfo.end())
	{
		return &li->second;
	}

	return NULL;
}

std::vector<RESET_TABLE_INFO>::iterator CResetTable::FindResetRange(int resetCount)
{
	return std::find_if(m_ResetTableInfo.begin(), m_ResetTableInfo.end(),
		[resetCount](const RESET_TABLE_INFO& info) {
			return resetCount >= info.MinReset && resetCount <= info.MaxReset;
		});
}
