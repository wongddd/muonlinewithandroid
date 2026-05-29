// ServerDisplayer.cpp: implementation of the CServerDisplayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerDisplayer.h"
#include "CustomArena.h"
#include "GameMain.h"
#include "Log.h"
#include "Protect.h"
#include "resource.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "User.h"
#include "Util.h"

CServerDisplayer gServerDisplayer;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerDisplayer::CServerDisplayer() // OK
{
	for (int n = 0; n < MAX_LOG_TEXT_LINE; n++)
	{
		memset(&this->m_log[n], 0, sizeof(this->m_log[n]));
	}

	this->m_font = CreateFont(50, 0, 0, 0, FW_THIN, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times");
	this->m_font2 = CreateFont(24, 0, 0, 0, FW_THIN, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Times");
	this->m_font3 = CreateFont(15, 0, 0, 0, FW_THIN, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");

	this->m_brush[LOG_INACTIVE_BRUSH] = CreateSolidBrush(RGB(47, 47, 47));
	this->m_brush[LOG_ACTIVE_BRUSH] = CreateSolidBrush(RGB(0, 190, 0));
	this->m_brush[LOG_NAME_BRUSH] = CreateSolidBrush(RGB(33, 33, 33));
	this->m_brush[LOG_CONTAINER_BRUSH] = CreateSolidBrush(RGB(13, 13, 13)); //-- log general
	this->m_brush[LOG_MAIN_BRUSH] = CreateSolidBrush(RGB(33, 33, 33)); //-- log general

	strcpy_s(this->m_DisplayerText[0], "STANDBY MODE");
	strcpy_s(this->m_DisplayerText[1], "ACTIVE MODE");

	m_hBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2));
}

CServerDisplayer::~CServerDisplayer() // OK
{
	DeleteObject(this->m_font);
	DeleteObject(this->m_brush[LOG_INACTIVE_BRUSH]);
	DeleteObject(this->m_brush[LOG_ACTIVE_BRUSH]);
	DeleteObject(this->m_brush[LOG_NAME_BRUSH]);
	DeleteObject(this->m_brush[LOG_CONTAINER_BRUSH]);
	DeleteObject(this->m_brush[LOG_MAIN_BRUSH]);

	if (m_hBitmap != nullptr)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = nullptr;
	}
}

void CServerDisplayer::Init(HWND hWnd) // OK
{
	PROTECT_START

	this->m_hwnd = hWnd;

	CreateDirectory("LOGS", 0);


	gLog.AddLog(1, "LOGS\\LOG");

	gLog.AddLog(gServerInfo.m_WriteChatLog, "LOGS\\CHAT_LOG");

	gLog.AddLog(gServerInfo.m_WriteCommandLog, "LOGS\\COMMAND_LOG");

	gLog.AddLog(gServerInfo.m_WriteTradeLog, "LOGS\\TRADE_LOG");

	gLog.AddLog(gServerInfo.m_WriteConnectLog, "LOGS\\CONNECT_LOG");

	gLog.AddLog(gServerInfo.m_WriteHackLog, "LOGS\\HACK_LOG");

	gLog.AddLog(gServerInfo.m_WriteCashShopLog, "LOGS\\CASH_SHOP_LOG");

	gLog.AddLog(gServerInfo.m_WriteChaosMixLog, "LOGS\\CHAOS_MIX_LOG");

	PROTECT_FINAL
}

void CServerDisplayer::Run() // OK
{
	this->LogTextPaint();

	this->PaintName();

	this->SetWindowName();

	this->PaintAllInfo();

	this->PaintOnline();

	this->PaintPremium();

	this->PaintSeason();

	this->PaintEventTime();

	this->PaintInvasionTime();

	this->PaintCustomArenaTime();

	this->LogTextPaintConnect();

	this->LogTextPaintGlobalMessage();
}

void CServerDisplayer::PaintName() // OK
{
	if (m_hBitmap != nullptr)
	{
		RECT Rect;
		HDC hdc = GetDC(this->m_hwnd);
		GetClientRect(this->m_hwnd, &Rect);

		HDC  hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcMem, m_hBitmap); // Usar m_hBitmap

		Rect.top = 0;
		Rect.bottom = 150;
		StretchBlt(hdc, 0, 0, 600, 150, hdcMem, 0, 0, 600, 150, SRCCOPY);
		SelectObject(hdcMem, hBitmapOld);
		DeleteDC(hdcMem);
		ReleaseDC(this->m_hwnd, hdc);
	}
}

void CServerDisplayer::SetWindowName() // OK
{
	char buff[256];

	wsprintf(buff, "[%s] %s (ON: %d) %s", GAMESERVER_VERSION, gServerInfo.m_ServerName, gObjTotalUser, GAMESERVER_CLIENT);

	SetWindowText(this->m_hwnd, buff);

	HWND hWndStatusBar = GetDlgItem(this->m_hwnd, IDC_STATUSBAR);

	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	RECT rect2;

	GetClientRect(hWndStatusBar, &rect2);

	MoveWindow(hWndStatusBar, 0, rect.bottom - rect2.bottom + rect2.top, rect.right, rect2.bottom - rect2.top, true);

	int iStatusWidths[] = { 190,270,360,450,580, -1 };

	char text[256];

	SendMessage(hWndStatusBar, SB_SETPARTS, 6, (LPARAM)iStatusWidths);

	wsprintf(text, "Gameserver %s - Update %d ", GAMESERVER_NAME, GAMESERVER_CLIENTE_UPDATE);

	SendMessage(hWndStatusBar, SB_SETTEXT, 0, (LPARAM)text);

	wsprintf(text, "OffStore: %d", gObjOffStore);

	SendMessage(hWndStatusBar, SB_SETTEXT, 1, (LPARAM)text);

	wsprintf(text, "OffAttack: %d", gObjOffAttack);

	SendMessage(hWndStatusBar, SB_SETTEXT, 2, (LPARAM)text);

	wsprintf(text, "Bots Buffer: %d", gObjTotalBot);

	SendMessage(hWndStatusBar, SB_SETTEXT, 3, (LPARAM)text);

	wsprintf(text, "Monsters: %d/%d", gObjTotalMonster, MAX_OBJECT_MONSTER);

	SendMessage(hWndStatusBar, SB_SETTEXT, 4, (LPARAM)text);

	SendMessage(hWndStatusBar, SB_SETTEXT, 5, (LPARAM)NULL);

	ShowWindow(hWndStatusBar, SW_SHOW);
}

void CServerDisplayer::PaintAllInfo() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	rect.left = rect.right - 180;
	rect.top = rect.bottom - 245;
	rect.bottom = rect.top + 50;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font2);

	if (gJoinServerConnection.CheckState() == 0 || gDataServerConnection.CheckState() == 0)
	{
		SetTextColor(hdc, RGB(200, 200, 200));
		FillRect(hdc, &rect, this->m_brush[LOG_INACTIVE_BRUSH]);
		WriteUnicode(hdc, rect.right - 170, rect.top + 14, this->m_DisplayerText[0], strlen(this->m_DisplayerText[0]));
	}
	else
	{
		gGameServerDisconnect = 0;
		SetTextColor(hdc, RGB(250, 250, 250));
		FillRect(hdc, &rect, this->m_brush[LOG_ACTIVE_BRUSH]);
		WriteUnicode(hdc, rect.right - 170, rect.top + 14, this->m_DisplayerText[1], strlen(this->m_DisplayerText[1]));
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::PaintOnline() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	rect.left = rect.right - 180;
	rect.top = rect.bottom - (245 - 58);
	rect.bottom = rect.top + 50;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font2);

	char text[50];

	wsprintf(text, "ONLINE: %d/%d", gObjTotalUser, gServerInfo.m_ServerMaxUserNumber);

	if (gObjTotalUser > 0)
	{
		SetTextColor(hdc, RGB(250, 250, 250));
		FillRect(hdc, &rect, this->m_brush[LOG_ACTIVE_BRUSH]);
		WriteUnicode(hdc, rect.right - 170, rect.top + 14, text, strlen(text));
	}
	else
	{
		SetTextColor(hdc, RGB(200, 200, 200));
		FillRect(hdc, &rect, this->m_brush[LOG_INACTIVE_BRUSH]);
		WriteUnicode(hdc, rect.right - 170, rect.top + 14, text, strlen(text));
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::PaintSeason() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	rect.left = rect.right - 180;
	rect.top = rect.bottom - (245 - 116);
	rect.bottom = rect.top + 50;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font2);

	SetTextColor(hdc, RGB(250, 250, 250));
	FillRect(hdc, &rect, this->m_brush[LOG_ACTIVE_BRUSH]);
	WriteUnicode(hdc, rect.right - 170, rect.top + 14, GAMESERVER_SEASON, sizeof(GAMESERVER_SEASON));

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::PaintPremium() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	rect.left = rect.right - 180;
	rect.top = rect.bottom - 70;
	rect.bottom = rect.bottom - 21;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font2);

	SetTextColor(hdc, RGB(250, 250, 250));
	FillRect(hdc, &rect, this->m_brush[LOG_ACTIVE_BRUSH]);
	WriteUnicode(hdc, rect.right - 170, rect.top + 14, GAMESERVER_NAME, 7);

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::PaintEventTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	int posX1 = rect.right - 295;
	int posX2 = rect.right - 200;

	rect.left = rect.right - 300;
	rect.right = rect.right - 150;
	rect.top = 5;
	rect.bottom = 290;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	FillRect(hdc, &rect, this->m_brush[LOG_CONTAINER_BRUSH]);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font3);

	char text1[20];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc, RGB(252, 2, 169));
	WriteUnicode(hdc, rect.left + 5, rect.top + 2, "EVENTS:", 7);

	for (int n = 0; n < 18; n++)
	{
		if (gEventName.IsGlobal(n) == false)
			continue;

		SetTextColor(hdc, RGB(0, 102, 204));

		int RemainTime = gEventName.GlobalRemainTime(n);

		wsprintf(text1, gEventName.GlobalName(n));

		if (RemainTime == -1)
		{
			wsprintf(text2, "Disabled");
		}
		else if (RemainTime == 0)
		{
			wsprintf(text2, "Online");
		}
		else
		{
			totalseconds = RemainTime;
			hours = totalseconds / 3600;
			minutes = (totalseconds / 60) % 60;
			seconds = totalseconds % 60;

			if (hours > 23)
			{
				days = hours / 24;
				wsprintf(text2, "%d day(s)+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}
		WriteUnicode(hdc, posX1, rect.top + 20 + (15 * n), text1, strlen(text1));

		if (RemainTime == -1)
		{
			SetTextColor(hdc, RGB(255, 0, 0));
		}
		else if (RemainTime == 0)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else if (RemainTime < 300)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else
		{
			SetTextColor(hdc, RGB(255, 255, 255));
		}
		WriteUnicode(hdc, posX2, rect.top + 20 + (15 * n), text2, strlen(text2));
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
#endif
}

void CServerDisplayer::PaintInvasionTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	int posX1 = rect.right - 445;
	int posX2 = rect.right - 355;

	rect.left = rect.right - 450;
	rect.right = rect.right - 305;
	rect.top = 5;
	rect.bottom = 290;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	FillRect(hdc, &rect, this->m_brush[LOG_CONTAINER_BRUSH]);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font3);

	char text1[20];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc, RGB(252, 2, 169));
	WriteUnicode(hdc, rect.left + 5, rect.top + 2, "INVASION:", 9);

	for (int n = 0; n < 18; n++)
	{
		if (gEventName.IsInvasion(n) == false)
			continue;

		SetTextColor(hdc, RGB(0, 102, 204));

		int RemainTime = gEventName.InvasionRemainTime(n);

		wsprintf(text1, gEventName.InvasionName(n));

		if (RemainTime == -1)
		{
			wsprintf(text2, "Disabled");
		}
		else if (RemainTime == 0)
		{
			wsprintf(text2, "Online");
		}
		else
		{
			totalseconds = RemainTime;
			hours = totalseconds / 3600;
			minutes = (totalseconds / 60) % 60;
			seconds = totalseconds % 60;

			if (hours > 23)
			{
				days = hours / 24;
				wsprintf(text2, "%d day(s)+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}

		WriteUnicode(hdc, posX1, rect.top + 20 + (15 * n), text1, strlen(text1));
		if (RemainTime == -1)
		{
			SetTextColor(hdc, RGB(255, 0, 0));
		}
		else if (RemainTime == 0)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else if (RemainTime < 300)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else
		{
			SetTextColor(hdc, RGB(255, 255, 255));
		}
		WriteUnicode(hdc, posX2, rect.top + 20 + (15 * n), text2, strlen(text2));
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
#endif
}

void CServerDisplayer::PaintCustomArenaTime() // OK
{
#if(GAMESERVER_TYPE==0)
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	int posX1 = rect.right - 140;
	int posX2 = rect.right - 60;

	rect.left = rect.right - 145;
	rect.right = rect.right - 5;
	rect.top = 5;
	rect.bottom = 290;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	FillRect(hdc, &rect, this->m_brush[LOG_CONTAINER_BRUSH]);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font3);

	char text1[20];
	char text2[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc, RGB(252, 2, 169));
	WriteUnicode(hdc, rect.left + 5, rect.top + 2, "CUSTOM ARENA:", 13);

	for (int n = 0; n < 18; n++)
	{
		if (gEventName.IsArena(n) == false)
			continue;

		SetTextColor(hdc, RGB(0, 102, 204));

		int RemainTime = gEventName.ArenaRemainTime(n);

		wsprintf(text1, gEventName.ArenaName(n));

		if (RemainTime == -1)
		{
			wsprintf(text2, "Disabled");
		}
		else if (RemainTime == 0)
		{
			wsprintf(text2, "Online");
		}
		else
		{
			totalseconds = RemainTime;
			hours = totalseconds / 3600;
			minutes = (totalseconds / 60) % 60;
			seconds = totalseconds % 60;

			if (hours > 23)
			{
				days = hours / 24;
				wsprintf(text2, "%d day(s)+", days);
			}
			else
			{
				wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
			}
		}

		WriteUnicode(hdc, posX1, rect.top + 20 + (15 * n), text1, strlen(text1));
		if (RemainTime == -1)
		{
			SetTextColor(hdc, RGB(255, 0, 0));
		}
		else if (RemainTime == 0)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else if (RemainTime < 300)
		{
			SetTextColor(hdc, RGB(0, 190, 0));
		}
		else
		{
			SetTextColor(hdc, RGB(255, 255, 255));
		}
		WriteUnicode(hdc, posX2, rect.top + 20 + (15 * n), text2, strlen(text2));
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);

#else
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	int posX1 = rect.right - 140;
	int posX2 = rect.right - 70;

	rect.left = rect.right - 145;
	rect.right = rect.right - 5;
	rect.top = 5;
	rect.bottom = 290;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	FillRect(hdc, &rect, this->m_brush[LOG_CONTAINER_BRUSH]);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font3);

	char text1[20];
	char text2[30];
	char text3[30];
	char text4[30];
	int totalseconds;
	int hours;
	int minutes;
	int seconds;
	int days;

	SetTextColor(hdc, RGB(252, 2, 169));
	WriteUnicode(hdc, rect.left + 5, rect.top + 2, "EVENTS:", 7);

	int PosY = rect.top + 20;

	SetTextColor(hdc, RGB(0, 102, 204));
	wsprintf(text1, "Loren Deep: ");

	if (this->EventCastleDeep == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCastleDeep == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds = this->EventCastleDeep;
		hours = totalseconds / 3600;
		minutes = (totalseconds / 60) % 60;
		seconds = totalseconds % 60;

		if (hours > 23)
		{
			days = hours / 24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	WriteUnicode(hdc, posX1, PosY, text1, strlen(text1));
	if (this->EventCastleDeep == -1)
	{
		SetTextColor(hdc, RGB(255, 0, 0));
	}
	else if (this->EventCastleDeep == 0)
	{
		SetTextColor(hdc, RGB(0, 190, 0));
	}
	else if (this->EventCastleDeep < 300)
	{
		SetTextColor(hdc, RGB(0, 190, 0));
	}
	else
	{
		SetTextColor(hdc, RGB(255, 255, 255));
	}
	WriteUnicode(hdc, posX2, PosY, text2, strlen(text2));

	SetTextColor(hdc, RGB(0, 102, 204));

	wsprintf(text1, "CryWolf: ");

	if (this->EventCryWolf == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCryWolf == 0)
	{
		wsprintf(text2, "Online");
	}
	else
	{
		totalseconds = this->EventCryWolf;
		hours = totalseconds / 3600;
		minutes = (totalseconds / 60) % 60;
		seconds = totalseconds % 60;

		if (hours > 23)
		{
			days = hours / 24;
			wsprintf(text2, "%d day(s)+", days);
		}
		else
		{
			wsprintf(text2, "%02d:%02d:%02d", hours, minutes, seconds);
		}
	}

	WriteUnicode(hdc, posX1, PosY + 15, text1, strlen(text1));
	if (this->EventCryWolf == -1)
	{
		SetTextColor(hdc, RGB(255, 0, 0));
	}
	else if (this->EventCryWolf == 0)
	{
		SetTextColor(hdc, RGB(0, 190, 0));
	}
	else if (this->EventCryWolf < 300)
	{
		SetTextColor(hdc, RGB(0, 190, 0));
	}
	else
	{
		SetTextColor(hdc, RGB(255, 255, 255));
	}
	WriteUnicode(hdc, posX2, PosY + 15, text2, strlen(text2));

	SetTextColor(hdc, RGB(0, 102, 204));
	wsprintf(text1, "Castle Siege: ");

	if (this->EventCs == -1)
	{
		wsprintf(text2, "Disabled");
	}
	else if (this->EventCs == 0)
	{
		wsprintf(text2, "Adjust Date");
		wsprintf(text3, " ");
		wsprintf(text4, " ");
	}
	else
	{
		totalseconds = this->EventCs;
		hours = totalseconds / 3600;
		minutes = (totalseconds / 60) % 60;
		seconds = totalseconds % 60;

		if (hours > 23)
		{
			days = hours / 24;
			wsprintf(text4, "- Next Stage: %d day(s)+", days);
		}
		else
		{
			wsprintf(text4, "- Next Stage: %02d:%02d:%02d", hours, minutes, seconds);
		}

		if (this->EventCsState == -1)
			wsprintf(text3, "- Stage %d: None", this->EventCsState);
		if (this->EventCsState == 0)
			wsprintf(text3, "- Stage %d: Idle 1", this->EventCsState);
		if (this->EventCsState == 1)
			wsprintf(text3, "- Stage %d: Guild Register", this->EventCsState);
		if (this->EventCsState == 2)
			wsprintf(text3, "- Stage %d: Idle 2", this->EventCsState);
		if (this->EventCsState == 3)
			wsprintf(text3, "- Stage %d: Mark Register", this->EventCsState);
		if (this->EventCsState == 4)
			wsprintf(text3, "- Stage %d: Idle 3", this->EventCsState);
		if (this->EventCsState == 5)
			wsprintf(text3, "- Stage %d: Notify", this->EventCsState);
		if (this->EventCsState == 6)
			wsprintf(text3, "- Stage %d: Ready Siege", this->EventCsState);
		if (this->EventCsState == 7)
			wsprintf(text3, "- Stage %d: Started Siege", this->EventCsState);
		if (this->EventCsState == 8)
			wsprintf(text3, "- Stage %d: Ended Siege", this->EventCsState);
		if (this->EventCsState == 9)
			wsprintf(text3, "- Stage %d: End All", this->EventCsState);

		if (this->EventCs)
			wsprintf(text2, "Stage %d", this->EventCsState);
	}

	WriteUnicode(hdc, posX1, PosY + 30, text1, strlen(text1));
	if (this->EventCs == -1)
	{
		SetTextColor(hdc, RGB(255, 0, 0));
	}
	else
	{
		SetTextColor(hdc, RGB(255, 255, 255));
	}
	WriteUnicode(hdc, posX2, PosY + 30, text2, strlen(text2));

	WriteUnicode(hdc, posX1 + 5, PosY + 45, text3, strlen(text3));

	WriteUnicode(hdc, posX1 + 5, PosY + 60, text4, strlen(text4));

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);

#endif
}

void CServerDisplayer::LogTextPaint() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	HDC hdc = GetDC(this->m_hwnd);

	FillRect(hdc, &rect, m_brush[LOG_MAIN_BRUSH]);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font3);

	int line = 0;

	int count = (((this->m_count - 1) >= 0) ? (this->m_count - 1) : (MAX_LOG_TEXT_LINE - 1));

	int RenderFrameY = rect.bottom - 38;

	for (int n = 0; n < MAX_LOG_TEXT_LINE; n++)
	{
		switch (this->m_log[count].color)
		{
		case LOG_BLACK:
			SetTextColor(hdc, RGB(255, 255, 255));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_RED:
			SetTextColor(hdc, RGB(255, 0, 0));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_GREEN:
			SetTextColor(hdc, RGB(0, 190, 0));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_BLUE:
			SetTextColor(hdc, RGB(0, 102, 204));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_BOT:
			SetTextColor(hdc, RGB(255, 0, 64));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_USER:
			SetTextColor(hdc, RGB(254, 154, 46));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_EVENT:
			SetTextColor(hdc, RGB(0, 102, 204));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_ALERT:
			SetTextColor(hdc, RGB(252, 2, 169));
			SetBkMode(hdc, TRANSPARENT);
			break;
		case LOG_YELOW:
			SetTextColor(hdc, RGB(255, 204, 25));
			SetBkMode(hdc, TRANSPARENT);
			break;
		}

		int size = strlen(this->m_log[count].text);

		if (size > 1 && (line * 15) <= (rect.bottom - 38) && count < MAX_LOG_TEXT_LINE)
		{
			WriteUnicode(hdc, 0, RenderFrameY, this->m_log[count].text, size);
			RenderFrameY -= 15;
			line++;
		}

		count = (((--count) >= 0) ? count : (MAX_LOG_TEXT_LINE - 1));
	}

	SelectObject(hdc, OldFont);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::LogTextPaintConnect() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	rect.left = rect.right - 450;
	rect.right = rect.right - 185;
	rect.top = rect.bottom - 245;
	rect.bottom = rect.bottom - 21;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font3);

	FillRect(hdc, &rect, this->m_brush[LOG_CONTAINER_BRUSH]);

	SetTextColor(hdc, RGB(252, 2, 169));
	WriteUnicode(hdc, rect.left + 5, rect.top + 2, "CONNECTION LOG:", 15);

	int line = MAX_LOGCONNECT_TEXT_LINE;

	int count = (((this->m_countConnect - 1) >= 0) ? (this->m_countConnect - 1) : (MAX_LOGCONNECT_TEXT_LINE - 1));

	for (int n = 0; n < MAX_LOGCONNECT_TEXT_LINE; n++)
	{
		SetBkMode(hdc, TRANSPARENT);
		switch (this->m_logConnect[count].color)
		{
		case LOG_BLACK:
			SetTextColor(hdc, RGB(255, 255, 255));
			break;
		case LOG_RED:
			SetTextColor(hdc, RGB(255, 0, 0));
			break;
		case LOG_GREEN:
			SetTextColor(hdc, RGB(0, 190, 0));
			break;
		case LOG_BLUE:
			SetTextColor(hdc, RGB(0, 102, 204));
			break;
		case LOG_BOT:
			SetTextColor(hdc, RGB(255, 0, 64));
			break;
		case LOG_USER:
			SetTextColor(hdc, RGB(254, 154, 46));
			break;
		case LOG_EVENT:
			SetTextColor(hdc, RGB(0, 102, 204));
			break;
		case LOG_ALERT:
			SetTextColor(hdc, RGB(252, 2, 169));
			break;
		case LOG_YELOW:
			SetTextColor(hdc, RGB(255, 204, 25));
			break;
		}

		int size = strlen(this->m_logConnect[count].text);

		if (size > 1)
		{
			WriteUnicode(hdc, rect.left + 10, (rect.top + 5 + (line * 15)), this->m_logConnect[count].text, size);
			line--;
		}

		count = (((--count) >= 0) ? count : (MAX_LOGCONNECT_TEXT_LINE - 1));
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::LogTextPaintGlobalMessage() // OK
{
	RECT rect;

	GetClientRect(this->m_hwnd, &rect);

	rect.left = rect.right - 450;
	rect.top = 295;
	rect.bottom = 440;

	HDC hdc = GetDC(this->m_hwnd);

	int OldBkMode = SetBkMode(hdc, TRANSPARENT);

	HFONT OldFont = (HFONT)SelectObject(hdc, this->m_font3);

	FillRect(hdc, &rect, this->m_brush[LOG_CONTAINER_BRUSH]);

	SetTextColor(hdc, RGB(252, 2, 169));
	WriteUnicode(hdc, rect.left + 5, rect.top + 2, "GLOBAL MESSAGE:", 15);

	int line = MAX_LOGGLOBAL_TEXT_LINE;

	int count = (((this->m_countGlobal - 1) >= 0) ? (this->m_countGlobal - 1) : (MAX_LOGGLOBAL_TEXT_LINE - 1));

	for (int n = 0; n < MAX_LOGGLOBAL_TEXT_LINE; n++)
	{
		SetTextColor(hdc, RGB(0, 190, 0));

		int size = strlen(this->m_logGlobal[count].text);

		if (size > 1)
		{
			WriteUnicode(hdc, rect.left + 10, (rect.top + 5 + (line * 15)), this->m_logGlobal[count].text, size);
			line--;
		}

		count = (((--count) >= 0) ? count : (MAX_LOGGLOBAL_TEXT_LINE - 1));
	}

	SelectObject(hdc, OldFont);
	SetBkMode(hdc, OldBkMode);
	ReleaseDC(this->m_hwnd, hdc);
}

void CServerDisplayer::LogAddText(eLogColor color, char* text, int size) // OK
{
	PROTECT_START

	size = ((size >= MAX_LOG_TEXT_SIZE) ? (MAX_LOG_TEXT_SIZE - 1) : size);

	memset(&this->m_log[this->m_count].text, 0, sizeof(this->m_log[this->m_count].text));

	memcpy(&this->m_log[this->m_count].text, text, size);

	this->m_log[this->m_count].color = color;

	this->m_count = (((++this->m_count) >= MAX_LOG_TEXT_LINE) ? 0 : this->m_count);

	gLog.Output(LOG_GENERAL, "%s", &text[9]);

	PROTECT_FINAL
}

void CServerDisplayer::LogAddTextConnect(eLogColor color, char* text, int size) // OK
{
	PROTECT_START

	size = ((size >= MAX_LOGCONNECT_TEXT_SIZE) ? (MAX_LOGCONNECT_TEXT_SIZE - 1) : size);

	memset(&this->m_logConnect[this->m_countConnect].text, 0, sizeof(this->m_logConnect[this->m_countConnect].text));

	memcpy(&this->m_logConnect[this->m_countConnect].text, text, size);

	this->m_logConnect[this->m_countConnect].color = color;

	this->m_countConnect = (((++this->m_countConnect) >= MAX_LOGCONNECT_TEXT_LINE) ? 0 : this->m_countConnect);

	gLog.Output(LOG_GENERAL, "%s", &text[9]);

	PROTECT_FINAL
}

void CServerDisplayer::LogAddTextGlobal(eLogColor color, char* text, int size) // OK
{
	PROTECT_START

	size = ((size >= MAX_LOGGLOBAL_TEXT_SIZE) ? (MAX_LOGGLOBAL_TEXT_SIZE - 1) : size);

	memset(&this->m_logGlobal[this->m_countGlobal].text, 0, sizeof(this->m_logGlobal[this->m_countGlobal].text));

	memcpy(&this->m_logGlobal[this->m_countGlobal].text, text, size);

	this->m_logGlobal[this->m_countGlobal].color = color;

	this->m_countGlobal = (((++this->m_countGlobal) >= MAX_LOGGLOBAL_TEXT_LINE) ? 0 : this->m_countGlobal);

	PROTECT_FINAL
}