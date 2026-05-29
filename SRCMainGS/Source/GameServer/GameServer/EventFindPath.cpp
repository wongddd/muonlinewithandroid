#include "StdAfx.h"
#include "Util.h"
#include "Notice.h"
#include "Message.h"
#include "MemScript.h"
#include "Monster.h"
#include "ServerInfo.h"
#include "EventFindPath.h"
#include "ItemBagManager.h"
#include "ScheduleManager.h"
#include "MapManager.h"

#ifdef EVENT_FIND_PATH_NPC_CLIK

CEventFindPath gEventFindPath;


CEventFindPath::CEventFindPath()
{
	this->m_state = EVENT_STATE_BLANK;
	this->m_NotifyTime = 0;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_MinutesLeft = -1;
	this->m_TickCount = GetTickCount();

	this->m_NpcIndex = -1;

	this->StartTime.clear();
}

CEventFindPath::~CEventFindPath()
{
}

void CEventFindPath::Init()
{
	if (gServerInfo.m_EventFindPathClickToNpc == 0)
	{
		this->SetState(EVENT_STATE_BLANK);
	}
	else
	{
		this->SetState(EVENT_STATE_EMPTY);
	}
}

void CEventFindPath::Load(char* path)
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

	this->m_Hidecoord = 0;

	this->m_NpcType = 465;

	this->m_NpcIndex = -1;

	this->m_EventTime = 0;

	this->m_MinutesLeft = -1;

	this->StartTime.clear();

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

					this->m_NotifyTime = lpMemScript->GetNumber();

					this->m_EventTime = lpMemScript->GetAsNumber();

					this->m_NpcType = lpMemScript->GetAsNumber();

					this->m_Hidecoord = lpMemScript->GetAsNumber();
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					EVENT_FIND_PATH_START_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->StartTime.push_back(info);
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

void CEventFindPath::MainProc()
{
	DWORD elapsed = GetTickCount() - this->m_TickCount;

	if (elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	this->m_RemainTime = (int)difftime(this->m_TargetTime, time(0));

	switch (this->m_state)
	{
	case EVENT_STATE_BLANK:
		this->ProcState_BLANK();
		break;
	case EVENT_STATE_EMPTY:
		this->ProcState_EMPTY();
		break;
	case EVENT_STATE_START:
		this->ProcState_START();
		break;
	}
}

void CEventFindPath::ProcState_BLANK()
{
}

void CEventFindPath::ProcState_EMPTY()
{
	if (this->m_RemainTime > 0 && this->m_RemainTime <= (this->m_NotifyTime * 60))
	{
		int minutes = this->m_RemainTime / 60;

		if ((this->m_RemainTime % 60) == 0)
		{
			minutes--;
		}

		if (this->m_MinutesLeft != minutes)
		{
			this->m_MinutesLeft = minutes;

			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(901), (this->m_MinutesLeft + 1));
		}
	}

	if (this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(902));

		this->SetState(EVENT_STATE_START);
	}
}

void CEventFindPath::ProcState_START()
{
	if (this->m_RemainTime > 0 && this->m_RemainTime <= 5)
	{
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(905), this->m_RemainTime);
	}

	if (this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(904));

		this->SetState(EVENT_STATE_EMPTY);
	}
}

void CEventFindPath::SetState(int state)
{
	this->m_state = state;

	switch (state)
	{
	case EVENT_STATE_BLANK:
		this->SetState_BLANK();
		break;
	case EVENT_STATE_EMPTY:
		this->SetState_EMPTY();
		break;
	case EVENT_STATE_START:
		this->SetState_START();
		break;
	}
}

void CEventFindPath::SetState_BLANK()
{
}

void CEventFindPath::SetState_EMPTY()
{
	this->ClearNpc();

	this->CheckSync();

	this->m_MinutesLeft = -1;
}

void CEventFindPath::SetState_START()
{
	this->StartEvent();

	this->m_RemainTime = (this->m_EventTime * 60);

	this->m_TargetTime = (int)(time(0) + this->m_RemainTime);
}

void CEventFindPath::CheckSync()
{
	if (this->StartTime.empty() != 0)
	{
		this->SetState(EVENT_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for (std::vector<EVENT_FIND_PATH_START_TIME>::iterator it = this->StartTime.begin(); it != this->StartTime.end(); it++)
	{
		ScheduleManager.AddSchedule(it->Year, it->Month, it->Day, it->Hour, it->Minute, it->Second, it->DayOfWeek);
	}

	if (ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(EVENT_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(), time(0));

	this->m_TargetTime = (int)ScheduleTime.GetTime();
}

void CEventFindPath::ClearNpc()
{
	gObjDel(this->m_NpcIndex);

	this->m_NpcIndex = -1;
}

void CEventFindPath::StartEvent()
{
	srand(static_cast<unsigned int>(time(0)));  // Inicializa la semilla

	int random_number = rand() % 3;  // Genera un número entre 0 y 4

	int x = 0;
	int y = 0;
	int map = random_number == 1 ? 0 : random_number;

	if (gObjGetRandomFreeLocation(map, &x, &y, 255, 255, 50) == 0)
	{
		return;
	}

	int index = gObjAddMonster(map);

	if (OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpNPC = &gObj[index];

	int px = x;
	int py = y;

	lpNPC->PosNum = -1;
	lpNPC->X = px;
	lpNPC->Y = py;
	lpNPC->TX = px;
	lpNPC->TY = py;
	lpNPC->OldX = px;
	lpNPC->OldY = py;
	lpNPC->StartX = px;
	lpNPC->StartY = py;
	lpNPC->Dir = 1;
	lpNPC->Map = map;

	if (gObjSetMonster(index, this->m_NpcType) == 0)
	{
		gObjDel(index);
		return;
	}

	this->m_NpcIndex = index;

	char message_global[100];


	if (this->m_Hidecoord == TRUE)
	{
		sprintf_s(message_global, "%s (?, ?)", gMapManager.GetMapName(map));
	}
	else
	{
		sprintf_s(message_global, "%s (%d, %d)", gMapManager.GetMapName(map), x, y);
	}

	gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(900), message_global);
}

bool CEventFindPath::NpcTalk(LPOBJ lpNpc, LPOBJ lpObj)
{
	if (gServerInfo.m_EventFindPathClickToNpc != 0)
	{
		if (lpNpc->Index == this->m_NpcIndex)
		{
			ClearNpc();

			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GlobalText(903), lpObj->Name);

			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_FIND_PATH_NPC, lpNpc, lpObj->Map, lpObj->X, lpObj->Y);

			if (this->m_RemainTime != 0)
			{
				StartEvent();
			}

			return true;
		}
	}
	return false;
}

void CEventFindPath::CommandEventStart()
{
	if (gServerInfo.m_EventFindPathClickToNpc == 0)
	{
		return;
	}

	CTime CurrentTime = CTime::GetTickCount();

	int hour = (int)CurrentTime.GetHour();
	int minute = (int)CurrentTime.GetMinute() + 2;

	if (minute >= 60)
	{
		hour++;
		minute = minute - 60;
	}

	EVENT_FIND_PATH_START_TIME info;

	info.Year = (int)CurrentTime.GetYear();

	info.Month = (int)CurrentTime.GetMonth();

	info.Day = (int)CurrentTime.GetDay();

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->StartTime.push_back(info);

	LogAdd(LOG_EVENT, "[Set Event Find Path Start] At %2d:%2d:00", hour, minute);

	this->SetState(EVENT_STATE_EMPTY);
}
#endif // EVENT_FIND_PATH_NPC_CLIK

