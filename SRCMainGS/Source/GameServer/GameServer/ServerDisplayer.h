// ServerDisplayer.h: interface for the CServerDisplayer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "CEventName.h"

#define MAX_LOG_TEXT_LINE 41
#define MAX_LOG_TEXT_SIZE 100

#define MAX_LOGCONNECT_TEXT_LINE 13
#define MAX_LOGCONNECT_TEXT_SIZE 55

#define MAX_LOGGLOBAL_TEXT_LINE 8
#define MAX_LOGGLOBAL_TEXT_SIZE 100

#define LOG_INACTIVE_BRUSH			0
#define LOG_ACTIVE_BRUSH			1
#define LOG_NAME_BRUSH				2
#define LOG_CONTAINER_BRUSH			3
#define LOG_MAIN_BRUSH				4

enum eLogColor
{
	LOG_BLACK = 0,
	LOG_RED = 1,
	LOG_GREEN = 2,
	LOG_BLUE = 3,
	//MC bot
	LOG_BOT = 4,
	LOG_USER = 5,
	LOG_EVENT = 6,
	LOG_ALERT = 7,
	LOG_YELOW = 8,
	//MC bot
};

struct LOG_DISPLAY_INFO
{
	char text[MAX_LOG_TEXT_SIZE];
	eLogColor color;
};

struct LOGCONNECT_DISPLAY_INFO
{
	char text[MAX_LOGCONNECT_TEXT_SIZE];
	eLogColor color;
};

struct LOGGLOBAL_DISPLAY_INFO
{
	char text[MAX_LOGGLOBAL_TEXT_SIZE];
	eLogColor color;
};

class CServerDisplayer
{
public:
	CServerDisplayer();
	virtual ~CServerDisplayer();
	void Init(HWND hWnd);
	void Run();
	void SetWindowName();
	void PaintAllInfo();
	void LogTextPaint();
	void LogTextPaintConnect();
	void LogTextPaintGlobalMessage();
	void PaintName();
	void PaintEventTime();
	void PaintInvasionTime();
	void PaintCustomArenaTime(); 
	void LogAddText(eLogColor color,char* text,int size);
	void LogAddTextConnect(eLogColor color,char* text,int size);
	void LogAddTextGlobal(eLogColor color,char* text,int size);
	void PaintOnline();
	void PaintPremium();
	void PaintSeason();
	//--
	//int EventInvasion[30];
	//int EventCustomArena[30];
	int EventCs;
	int EventCsState;
	int EventCastleDeep;
	int EventCryWolf;
	int EventCryWolfState;
private:
	HWND m_hwnd;
	HFONT m_font;
	HFONT m_font2;
	HFONT m_font3;
	HBRUSH m_brush[5];
	HBITMAP m_hBitmap;

	LOG_DISPLAY_INFO m_log[MAX_LOG_TEXT_LINE];
	LOGCONNECT_DISPLAY_INFO m_logConnect[MAX_LOGCONNECT_TEXT_LINE];
	LOGGLOBAL_DISPLAY_INFO m_logGlobal[MAX_LOGGLOBAL_TEXT_LINE];
	int m_count;
	int m_countConnect;
	int m_countGlobal;
	char m_DisplayerText[2][64];
};

extern CServerDisplayer gServerDisplayer;
