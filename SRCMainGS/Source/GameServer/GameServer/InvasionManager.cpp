// InvasionManager.cpp: implementation of the CInvasionManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InvasionManager.h"
#include "Map.h"
#include "MemScript.h"
#include "Message.h"
#include "Monster.h"
#include "MonsterSetBase.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "ScheduleManager.h"
#include "ServerInfo.h"
#include "Util.h"

CInvasionManager gInvasionManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInvasionManager::CInvasionManager() // OK
{
	for (int n = 0; n < MAX_INVASION; n++)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[n];

		lpInfo->Index = n;
		lpInfo->State = INVASION_STATE_BLANK;
		lpInfo->RemainTime = 0;
		lpInfo->TargetTime = 0;
		lpInfo->TickCount = GetTickCount();

		this->CleanMonster(lpInfo);
	}

	memset(map_dragon, 0, sizeof(map_dragon));
}

CInvasionManager::~CInvasionManager() // OK
{
}

void CInvasionManager::Init() // OK
{
	for (int n = 0; n < MAX_INVASION; n++)
	{
		if (gServerInfo.m_InvasionManagerSwitch == 0)
		{
			this->SetState(&this->m_InvasionInfo[n], INVASION_STATE_BLANK);
		}
		else
		{
			this->SetState(&this->m_InvasionInfo[n], INVASION_STATE_EMPTY);
		}
	}
}

void CInvasionManager::Load(char* path) // OK
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

	for (int n = 0; n < MAX_INVASION; n++)
	{
#ifdef INVASION_TIME_MANAGER_KILL
		this->m_InvasionInfo[n].Name = "Invasion";
#endif // INVASION_TIME_MANAGER_KILL
		this->m_InvasionInfo[n].RespawnMessage = -1;
		this->m_InvasionInfo[n].DespawnMessage = -1;
		this->m_InvasionInfo[n].BossIndex = -1;
		this->m_InvasionInfo[n].BossMessage = -1;
		this->m_InvasionInfo[n].InvasionTime = 0;
		this->m_InvasionInfo[n].StartTime.clear();
		this->m_InvasionInfo[n].RespawnInfo[0].clear();
		this->m_InvasionInfo[n].RespawnInfo[1].clear();
		this->m_InvasionInfo[n].RespawnInfo[2].clear();
		this->m_InvasionInfo[n].RespawnInfo[3].clear();
		this->m_InvasionInfo[n].RespawnInfo[4].clear();
		this->m_InvasionInfo[n].RespawnInfo[5].clear();
		this->m_InvasionInfo[n].RespawnInfo[6].clear();
		this->m_InvasionInfo[n].RespawnInfo[7].clear();
		this->m_InvasionInfo[n].RespawnInfo[8].clear();
		this->m_InvasionInfo[n].RespawnInfo[9].clear();
		this->m_InvasionInfo[n].RespawnInfo[10].clear();
		this->m_InvasionInfo[n].RespawnInfo[11].clear();
		this->m_InvasionInfo[n].RespawnInfo[12].clear();
		this->m_InvasionInfo[n].RespawnInfo[13].clear();
		this->m_InvasionInfo[n].RespawnInfo[14].clear();
		this->m_InvasionInfo[n].RespawnInfo[15].clear();
		this->m_InvasionInfo[n].RespawnInfo[16].clear();
		this->m_InvasionInfo[n].RespawnInfo[17].clear();
		this->m_InvasionInfo[n].RespawnInfo[18].clear();
		this->m_InvasionInfo[n].RespawnInfo[19].clear();
		this->m_InvasionInfo[n].MonsterInfo.clear();
	}

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

					INVASION_START_TIME info;

					int index = lpMemScript->GetNumber();

					info.Year = lpMemScript->GetAsNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_InvasionInfo[index].StartTime.push_back(info);
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int index = lpMemScript->GetNumber();

					this->m_InvasionInfo[index].Index = index;
#ifdef INVASION_TIME_MANAGER_KILL
					this->m_InvasionInfo[index].Name = lpMemScript->GetAsString();
#endif // INVASION_TIME_MANAGER_KILL

					this->m_InvasionInfo[index].RespawnMessage = lpMemScript->GetAsNumber();

					this->m_InvasionInfo[index].DespawnMessage = lpMemScript->GetAsNumber();

					this->m_InvasionInfo[index].BossIndex = lpMemScript->GetAsNumber();

					this->m_InvasionInfo[index].BossMessage = lpMemScript->GetAsNumber();

					this->m_InvasionInfo[index].InvasionTime = lpMemScript->GetAsNumber();
				}
				else if (section == 2)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					INVASION_RESPWAN_INFO info;

					int index = lpMemScript->GetNumber();

					info.Group = lpMemScript->GetAsNumber();

					info.Map = lpMemScript->GetAsNumber();

					info.Value = lpMemScript->GetAsNumber();

					this->m_InvasionInfo[index].RespawnInfo[info.Group].push_back(info);
				}
				else if (section == 3)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					INVASION_MONSTER_INFO info;

					int index = lpMemScript->GetNumber();

					info.Group = lpMemScript->GetAsNumber();

					info.MonsterClass = lpMemScript->GetAsNumber();

					info.RegenType = lpMemScript->GetAsNumber();

					info.RegenTime = lpMemScript->GetAsNumber();

					this->m_InvasionInfo[index].MonsterInfo.push_back(info);
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

void CInvasionManager::MainProc() // OK
{
	for (int n = 0; n < MAX_INVASION; n++)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[n];

		DWORD elapsed = GetTickCount() - lpInfo->TickCount;

		if (elapsed < 1000)
		{
			continue;
		}

		if (lpInfo->State == INVASION_STATE_BLANK)
		{
			if (gEventName.InvasionRemainTime(n) != -1)
			{
				gEventName.InvasionRemainTime(n, -1);
			}
		}
		else
		{
			if (lpInfo->State == INVASION_STATE_EMPTY)
			{
				gEventName.InvasionRemainTime(n, lpInfo->RemainTime);
			}
			else
			{
				if (gEventName.InvasionRemainTime(n) != 0)
				{
					gEventName.InvasionRemainTime(n, 0);
				}
			}
		}

		if (gEventName.InvasionRemainTime(n) > 0)
		{
			if (lpInfo->State != INVASION_STATE_EMPTY && lpInfo->State != INVASION_STATE_BLANK)
			{
				gEventName.InvasionRemainTime(n, 0);
			}
		}

		lpInfo->TickCount = GetTickCount();

		lpInfo->RemainTime = (int)difftime(lpInfo->TargetTime, time(0));

		switch (lpInfo->State)
		{
		case INVASION_STATE_BLANK:
			this->ProcState_BLANK(lpInfo);
			break;
		case INVASION_STATE_EMPTY:
			this->ProcState_EMPTY(lpInfo);
			break;
		case INVASION_STATE_START:
			this->ProcState_START(lpInfo);
			break;
		}
	}
}

void CInvasionManager::ProcState_BLANK(INVASION_INFO* lpInfo) // OK
{
}

void CInvasionManager::ProcState_EMPTY(INVASION_INFO* lpInfo) // OK
{
	if (lpInfo->RemainTime <= 0)
	{
		if (lpInfo->RespawnMessage != -1)
		{
			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(lpInfo->RespawnMessage));
		}

		this->SetState(lpInfo, INVASION_STATE_START);
	}
}

void CInvasionManager::ProcState_START(INVASION_INFO* lpInfo) // OK
{
	if (lpInfo->RemainTime <= 0)
	{
		if (lpInfo->DespawnMessage != -1)
		{
			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(lpInfo->DespawnMessage));
		}

		this->SetState(lpInfo, INVASION_STATE_EMPTY);
	}
}

void CInvasionManager::SetState(INVASION_INFO* lpInfo, int state) // OK
{
	lpInfo->State = state;

	switch (lpInfo->State)
	{
	case INVASION_STATE_BLANK:
		this->SetState_BLANK(lpInfo);
		break;
	case INVASION_STATE_EMPTY:
		this->SetState_EMPTY(lpInfo);
		break;
	case INVASION_STATE_START:
		this->SetState_START(lpInfo);
		break;
	}
}

void CInvasionManager::SetState_BLANK(INVASION_INFO* lpInfo) // OK
{
}

void CInvasionManager::SetState_EMPTY(INVASION_INFO* lpInfo) // OK
{
	this->ClearMonster(lpInfo);

	this->CheckSync(lpInfo);

#ifdef INVASION_TIME_MANAGER_KILL
	lpInfo->total_data_monster.clear();

	send_info_invasion(lpInfo, 0, -1);
#endif // INVASION_TIME_MANAGER_KILL

	if (lpInfo->Index == 1 || lpInfo->Index == 2)
	{
		memset(map_dragon[lpInfo->Index - 1], 1, MAX_MAP);

		state_effect(lpInfo->Index);

		memset(map_dragon[lpInfo->Index - 1], 0, MAX_MAP);
	}
}

void CInvasionManager::SetState_START(INVASION_INFO* lpInfo) // OK
{
#ifdef INVASION_TIME_MANAGER_KILL
	lpInfo->total_data_monster.clear();
#endif // INVASION_TIME_MANAGER_KILL

	for (int n = 0; n < MAX_INVASION_RESPAWN_GROUP; n++)
	{
		if (lpInfo->RespawnInfo[n].empty() == 0)
		{
			INVASION_RESPWAN_INFO* lpRespawnInfo = &lpInfo->RespawnInfo[n][(GetLargeRand() % lpInfo->RespawnInfo[n].size())];

			for (std::vector<INVASION_MONSTER_INFO>::iterator it = lpInfo->MonsterInfo.begin(); it != lpInfo->MonsterInfo.end(); it++)
			{
				if (it->Group == lpRespawnInfo->Group)
				{
					this->SetMonster(lpInfo, lpRespawnInfo, &(*it));
				}
			}
		}
	}

	lpInfo->RemainTime = lpInfo->InvasionTime;

	lpInfo->TargetTime = (int)(time(0) + lpInfo->RemainTime);

#ifdef INVASION_TIME_MANAGER_KILL
	send_info_invasion(lpInfo, 1, -1);
#endif // INVASION_TIME_MANAGER_KILL
}

void CInvasionManager::CheckSync(INVASION_INFO* lpInfo) // OK
{
	if (lpInfo->StartTime.empty() != 0)
	{
		this->SetState(lpInfo, INVASION_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for (std::vector<INVASION_START_TIME>::iterator it = lpInfo->StartTime.begin(); it != lpInfo->StartTime.end(); it++)
	{
		ScheduleManager.AddSchedule(it->Year, it->Month, it->Day, it->Hour, it->Minute, it->Second, it->DayOfWeek);
	}

	if (ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(lpInfo, INVASION_STATE_BLANK);
		return;
	}

	lpInfo->RemainTime = (int)difftime(ScheduleTime.GetTime(), time(0));

	lpInfo->TargetTime = (int)ScheduleTime.GetTime();
}

int CInvasionManager::GetState(int index) // OK
{
	if (index < 0 || index >= MAX_INVASION)
	{
		return INVASION_STATE_BLANK;
	}

	return this->m_InvasionInfo[index].State;
}

int CInvasionManager::GetRemainTime(int index) // OK
{
	if (index < 0 || index >= MAX_INVASION)
	{
		return 0;
	}

	INVASION_INFO* lpInfo = &this->m_InvasionInfo[index];

	if (lpInfo->StartTime.empty() != 0)
	{
		return 0;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for (std::vector<INVASION_START_TIME>::iterator it = lpInfo->StartTime.begin(); it != lpInfo->StartTime.end(); it++)
	{
		ScheduleManager.AddSchedule(it->Year, it->Month, it->Day, it->Hour, it->Minute, it->Second, it->DayOfWeek);
	}

	if (ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		return 0;
	}

	int RemainTime = (int)difftime(ScheduleTime.GetTime(), time(0));

	return (((RemainTime % 60) == 0) ? (RemainTime / 60) : ((RemainTime / 60) + 1));
}

bool CInvasionManager::AddMonster(INVASION_INFO* lpInfo, int aIndex) // OK
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if (this->GetMonster(lpInfo, aIndex) != 0)
	{
		return 0;
	}

	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (OBJECT_RANGE(lpInfo->MonsterIndex[n]) == 0)
		{
			lpInfo->MonsterIndex[n] = aIndex;
			return 1;
		}
	}

	return 0;
}

bool CInvasionManager::DelMonster(INVASION_INFO* lpInfo, int aIndex) // OK
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	int* index = this->GetMonster(lpInfo, aIndex);

	if (index == 0)
	{
		return 0;
	}

	(*index) = -1;
	return 1;
}

int* CInvasionManager::GetMonster(INVASION_INFO* lpInfo, int aIndex) // OK
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (lpInfo->MonsterIndex[n] == aIndex)
		{
			return &lpInfo->MonsterIndex[n];
		}
	}

	return 0;
}

void CInvasionManager::CleanMonster(INVASION_INFO* lpInfo) // OK
{
	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		lpInfo->MonsterIndex[n] = -1;
	}
}

void CInvasionManager::ClearMonster(INVASION_INFO* lpInfo) // OK
{
	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (OBJECT_RANGE(lpInfo->MonsterIndex[n]) != 0)
		{
			gObjDel(lpInfo->MonsterIndex[n]);
			lpInfo->MonsterIndex[n] = -1;
		}
	}
}

int CInvasionManager::GetMonsterCount(INVASION_INFO* lpInfo) // OK
{
	int count = 0;

	for (int n = 0; n < MAX_INVASION_MONSTER; n++)
	{
		if (OBJECT_RANGE(lpInfo->MonsterIndex[n]) != 0)
		{
			count++;
		}
	}

	return count;
}

void CInvasionManager::SetMonster(INVASION_INFO* lpInfo, INVASION_RESPWAN_INFO* lpRespawnInfo, INVASION_MONSTER_INFO* lpMonsterInfo) // OK
{
	std::vector<MONSTER_SET_BASE_INFO> base_info = gMonsterSetBase.GetMonsterMap(lpRespawnInfo->Map);

	//for (int n = 0; n < gMonsterSetBase.m_count; n++)
	for (int n = 0; n < base_info.size(); n++)
	{
		MONSTER_SET_BASE_INFO* lpMsbInfo = &base_info[n];
		//MONSTER_SET_BASE_INFO* lpMsbInfo = &gMonsterSetBase.m_MonsterSetBaseInfo[n];

		if (lpMsbInfo->Type != 3 || lpMsbInfo->MonsterClass != lpMonsterInfo->MonsterClass || lpMsbInfo->Map != lpRespawnInfo->Map || lpMsbInfo->Value != lpRespawnInfo->Value)
		{
			continue;
		}

		int index = gObjAddMonster(lpMsbInfo->Map);

		if (OBJECT_RANGE(index) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[index];

		if (gObjSetPosMonster(index, lpMsbInfo->index) == 0)
		{
			gObjDel(index);
			continue;
		}

		if (gObjSetMonster(index, lpMsbInfo->MonsterClass) == 0)
		{
			gObjDel(index);
			continue;
		}

		lpObj->MaxRegenTime = ((lpMonsterInfo->RegenType == 0) ? (lpInfo->InvasionTime * 1000) : lpMonsterInfo->RegenTime);

		if (lpObj->Class == lpInfo->BossIndex && lpInfo->BossMessage != -1)
		{
			LogAdd(LOG_EVENT, "[Invasion Manager] Index(%d) Boss Position (Map: %d X: %d Y: %d)", lpInfo->Index, lpObj->Map, lpObj->X, lpObj->Y);
		}

		if (this->AddMonster(lpInfo, index) == 0)
		{
			gObjDel(index);
			continue;
		}

		if (lpInfo->Index == 1 || lpInfo->Index == 2)
		{
			map_dragon[lpInfo->Index-1][lpObj->Map] = 2;
		}

#ifdef INVASION_TIME_MANAGER_KILL
		type_count_monster::iterator li = lpInfo->total_data_monster.find(lpMsbInfo->MonsterClass);

		if (li != lpInfo->total_data_monster.end())
		{
			li->second.MonsterCount++;
		}
		else
		{
			INVASION_INFO_MONSTER info;
			info.MonsterIndex = lpMsbInfo->MonsterClass;
			info.Monster_Kill = 0;
			info.MonsterCount = 1;
			lpInfo->total_data_monster.insert(type_count_monster::value_type(lpMsbInfo->MonsterClass, info));
		}
#endif // INVASION_TIME_MANAGER_KILL
	}
}

void CInvasionManager::MonsterDieProc(LPOBJ lpObj, LPOBJ lpTarget) // OK
{
	int aIndex = gObjMonsterGetTopHitDamageUser(lpObj);

	if (OBJECT_RANGE(aIndex) != 0)
	{
		lpTarget = &gObj[aIndex];
	}

	for (int n = 0; n < MAX_INVASION; n++)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[n];

		if (this->GetState(lpInfo->Index) != INVASION_STATE_START)
		{
			continue;
		}

		if (this->GetMonster(lpInfo, lpObj->Index) == 0)
		{
			continue;
		}

#ifdef INVASION_TIME_MANAGER_KILL
		for (auto it = lpInfo->total_data_monster.begin(); it != lpInfo->total_data_monster.end(); it++)
		{
			if (it->second.MonsterIndex == lpObj->Class)
			{
				it->second.Monster_Kill++;

				this->send_info_kill(&it->second, lpInfo->Index);
			}
		}
#endif // INVASION_TIME_MANAGER_KILL

		if (lpObj->Class == lpInfo->BossIndex && lpInfo->BossMessage != -1)
		{
			if (lpInfo->Index == 1 || lpInfo->Index == 2)
			{
				map_dragon[lpInfo->Index - 1][lpObj->Map] = 1;
				state_effect(lpInfo->Index);
			}

			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(lpInfo->BossMessage), lpTarget->Name);
			continue;
		}
	}
}

void CInvasionManager::StartInvasion(int index)
{
	CTime CurrentTime = CTime::GetTickCount();

	int hour = (int)CurrentTime.GetHour();
	int minute = (int)CurrentTime.GetMinute() + 2;

	if (minute >= 60)
	{
		hour++;
		minute = minute - 60;
	}

	INVASION_START_TIME info;

	info.Year = (int)CurrentTime.GetYear();

	info.Month = (int)CurrentTime.GetMonth();

	info.Day = (int)CurrentTime.GetDay();

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_InvasionInfo[index].StartTime.push_back(info);

	LogAdd(LOG_EVENT, "[Set Invasion Start] Index %d At %2d:%2d:00", index, hour, minute);

	this->SetState(&this->m_InvasionInfo[index], INVASION_STATE_EMPTY);
}

#ifdef INVASION_TIME_MANAGER_KILL
void CInvasionManager::send_player_info(int index)
{
	INVASION_INFO* lpInfo;

	for (int n = 0; n < MAX_INVASION; n++)
	{
		lpInfo = &this->m_InvasionInfo[n];

		if (lpInfo->State == INVASION_STATE_START)
		{
			if (lpInfo->total_data_monster.size() > 0)
			{
				send_info_invasion(lpInfo, 1, index);
			}
		}
		else
		{
			send_info_invasion(lpInfo, 0, index);
		}
	}
}

void CInvasionManager::send_info_kill(INVASION_INFO_MONSTER* lpInfo, int index)
{
	PMSG_INVASION_KILL_INFO pMsg;

	pMsg.Head.setE(0x4E, 0x3e, sizeof(pMsg));

	pMsg.Index = index;

	pMsg.MonsterIndex = lpInfo->MonsterIndex;

	pMsg.MonsterCount = lpInfo->MonsterCount;

	pMsg.Monster_Kill = lpInfo->Monster_Kill;

	DataSendAll((BYTE*)&pMsg, sizeof(pMsg));
}

void CInvasionManager::send_info_invasion(INVASION_INFO* lpInfo, int status, int index)
{
	if (status == 1)
	{
		PMSG_INVASION_INFO pMsg;

		pMsg.Head.setE(0x4E, 0x3f, 0);

		pMsg.count = 0;

		pMsg.Index = lpInfo->Index;

		pMsg.RemainTime = lpInfo->RemainTime;

		strncpy_s(pMsg.Name, lpInfo->Name.c_str(), sizeof(pMsg.Name));

		int size = sizeof(pMsg);

		INVASION_INFO_MONSTER info;

		BYTE send[2048];

		for (auto it = lpInfo->total_data_monster.begin(); it != lpInfo->total_data_monster.end(); it++)
		{
			info.MonsterIndex = it->first;

			info.MonsterCount = it->second.MonsterCount;

			info.Monster_Kill = it->second.Monster_Kill;

			memcpy(&send[size], &info, sizeof(info));

			size += sizeof(info);

			pMsg.count++;
		}

		pMsg.Head.size[0] = SET_NUMBERHB(size);
		pMsg.Head.size[1] = SET_NUMBERLB(size);

		memcpy(send, &pMsg, sizeof(pMsg));

		if (index != -1)
			DataSend(index, send, size);
		else
			DataSendAll(send, size);
	}
	else
	{
		PMSG_INVASION_INFO pMsg;

		int size = sizeof(pMsg);

		pMsg.Head.setE(0x4E, 0x3f, size);

		pMsg.count = 0;

		pMsg.Index = lpInfo->Index;

		pMsg.RemainTime = 0;

		strncpy_s(pMsg.Name, lpInfo->Name.c_str(), sizeof(pMsg.Name));

		if (index != -1)
			DataSend(index, (BYTE*)&pMsg, size);
		else
			DataSendAll((BYTE*)&pMsg, size);
	}

	if (lpInfo->Index == 1 || lpInfo->Index == 2)
	{
		if (index != -1)
			state_event(index, lpInfo->Index);
		else
			state_effect(lpInfo->Index);
	}
}
#endif // INVASION_TIME_MANAGER_KILL

void CInvasionManager::cast_effect(LPOBJ lpObj)
{
	INVASION_INFO* lpInfo = &this->m_InvasionInfo[2];

	if (lpInfo->State == INVASION_STATE_START)
	{
		state_event(lpObj->Index, lpInfo->Index);
	}
	else
	{
		lpInfo = &this->m_InvasionInfo[1];

		if (lpInfo->State == INVASION_STATE_START)
		{
			state_event(lpObj->Index, lpInfo->Index);
		}
	}
}

void CInvasionManager::state_event(int index, BYTE _event)
{
	if (_event == 2)
	{
		LPOBJ pCharacter = &gObj[index];

		if (map_dragon[1][pCharacter->Map] != 0)
		{
			GCEventStateSend(index, map_dragon[1][pCharacter->Map] - 1, (_event * 2) - 1);
		}
	}
	else if (_event == 1)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[2];

		if (lpInfo->State != INVASION_STATE_START)
		{
			LPOBJ pCharacter = &gObj[index];

			if (map_dragon[0][pCharacter->Map] != 0)
			{
				GCEventStateSend(index, map_dragon[0][pCharacter->Map] - 1, (_event * 2) - 1);
			}
		}
	}

	/*BYTE cur_event = 1;
	BYTE cur_state = 0;

	INVASION_INFO* lpInfo = &this->m_InvasionInfo[2];

	if (lpInfo->State == INVASION_STATE_START)
	{
		cur_state = 1;
		cur_event = 3;
	}
	else
	{
		lpInfo = &this->m_InvasionInfo[1];

		if (lpInfo->State == INVASION_STATE_START)
		{
			cur_state = 1;
			cur_event = 1;
		}
	}

	if (index != -1)
	{
		if ((effect_map_dragon[gObj[index].Map] - 1) == cur_state)
		{
			GCEventStateSend(index, cur_state, cur_event);
		}
	}
	else
	{
		state_effect(cur_event);
	}*/
}

void CInvasionManager::state_effect(BYTE _event)
{
	if (_event == 2)
	{
		for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
		{
			if (gObjIsConnectedGP(n) != 0)
			{
				LPOBJ pCharacter = &gObj[n];

				if (map_dragon[1][pCharacter->Map] != 0)
				{
					GCEventStateSend(n, map_dragon[1][pCharacter->Map] - 1, (_event * 2) - 1);
				}
			}
		}
	}
	else if (_event == 1)
	{
		INVASION_INFO* lpInfo = &this->m_InvasionInfo[2];

		if (lpInfo->State != INVASION_STATE_START)
		{
			for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
			{
				if (gObjIsConnectedGP(n) != 0)
				{
					LPOBJ pCharacter = &gObj[n];

					if (map_dragon[0][pCharacter->Map] != 0)
					{
						GCEventStateSend(n, map_dragon[0][pCharacter->Map] - 1, (_event * 2) - 1);
					}
				}
			}
		}
	}
}
