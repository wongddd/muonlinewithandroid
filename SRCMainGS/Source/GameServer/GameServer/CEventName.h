#pragma once
#define MAX_SIZE_EVENT_TEMPLATE			50

#define BLOOD_CASTLE_TIME				0
#define DEVIL_SQUARE_TIME				1
#define CHAOS_CASTLE_TIME				2
#define ILUSION_TEMP_TIME				3
#define ONLY_LOTTERY_TIME				4
#define QUIZ_EVENT_TIME					5
#define BONUS_EVENT_TIME				6
#define DROP_EVENT_TIME					7
#define KING_EVENT_TIME					8
#define TVT_EVENT_TIME					9
#define GVG_EVENT_TIME					10
#define MOSS_MERCH_TIME					11
//#define BLOOD_CASTLE_TIME	1


struct TEMPLATE_EVENT
{
	int switch_on;
	int m_Key;
	int m_RemainTime;
	std::string m_Name;
	std::string m_Des;
	int m_Gate;

	TEMPLATE_EVENT() : m_Key(-1), m_Name(""), m_RemainTime(-1), m_Des(""), m_Gate(-1)
	{
	}

	TEMPLATE_EVENT(int key, const std::string& Name) : m_Key(key), m_Name(Name), m_RemainTime(-1), m_Des(""), m_Gate(-1)
	{
	}

	const char* GetName()
	{
		return m_Name.c_str();
	}
	const char* GetDes()
	{
		return m_Des.c_str();
	}
	int GetGate()
	{
		return m_Gate;
	}
};


class CEventName
{
public:
	CEventName();
	virtual~CEventName();
	void Release();
	void OpenFile(std::string filename);
	void push_back(int switch_on, const TEMPLATE_EVENT& info);

	bool IsArena(int Index);
	bool IsGlobal(int Index);
	bool IsInvasion(int Index);


	int ArenaRemainTime(int Index);
	int GlobalRemainTime(int Index);
	int InvasionRemainTime(int Index);

	const char* ArenaName(int Index);
	const char* GlobalName(int Index);
	const char* InvasionName(int Index);

	void ArenaRemainTime(int Index, int Time);
	void GlobalRemainTime(int Index, int Time);
	void InvasionRemainTime(int Index, int Time);

	std::vector <TEMPLATE_EVENT> m_SendClientDataEventTime;
	int CEventName::GetTimeEventSwitch(int switch_on, int Index);
private:
	std::vector<TEMPLATE_EVENT> m_EventGlobal;
	std::vector<TEMPLATE_EVENT> m_InvasionGlobal;
	std::vector<TEMPLATE_EVENT> m_ArenaGlobal;
};

extern CEventName gEventName;