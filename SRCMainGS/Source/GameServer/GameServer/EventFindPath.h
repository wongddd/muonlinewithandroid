#pragma once

#ifdef EVENT_FIND_PATH_NPC_CLIK

enum eEventState
{
	EVENT_STATE_BLANK = 0,
	EVENT_STATE_EMPTY = 1,
	EVENT_STATE_START = 2,
};

typedef struct
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
}EVENT_FIND_PATH_START_TIME;

class CEventFindPath
{
public:
	CEventFindPath();
	virtual~CEventFindPath();

	void Init();
	void Load(char* path);
	void MainProc();


	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_START();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_START();
	void CheckSync();
	void ClearNpc();
	void StartEvent();

	bool NpcTalk(LPOBJ lpNpc, LPOBJ lpObj);
	void CommandEventStart();
private:
	int m_state;
	int m_NpcIndex;
	int m_EventTime;
	int m_NotifyTime;
	int m_TickCount;
	int m_RemainTime;
	int m_TargetTime;
	int m_MinutesLeft;

	int m_NpcType;
	BOOL m_Hidecoord;
	std::vector<EVENT_FIND_PATH_START_TIME> StartTime;
};

extern CEventFindPath gEventFindPath;
#endif // EVENT_FIND_PATH_NPC_CLIK
