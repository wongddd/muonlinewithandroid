#include "stdafx.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "ServerInfo.h"
#include "CustomEventTime.h"
#include "MemScript.h"
#include "CEventName.h"
#include "Notice.h"

CCustomEventTime gCustomEventTime;


//void CCustomEventTime::GCReqEventTime(int Index, PMSG_CUSTOM_EVENTTIME_RECV* lpMsg)
//{
//
//#if (GAMESERVER_CLIENTE_UPDATE >= 8)
//
//	if (gServerInfo.m_CustomEventTimeSwitch == 0)
//	{
//		return;
//	}
//
//	if (gObjIsConnected(Index) == false)
//	{
//		return;
//	}
//
//	BYTE send[4096];
//
//	PMSG_CUSTOM_EVENTTIME_SEND pMsg;
//
//	pMsg.header.set(0xF3, 0xE8, 0);
//
//	int size = sizeof(pMsg);
//
//	pMsg.count = 0;
//
//	CUSTOM_EVENTTIME_DATA info;
//
//	for (int n = 0; n < 20; n++)
//	{
//		info.index = n;
//
//		info.time = gEventName.GlobalRemainTime(n);
//		/*if (n == 0)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 1)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 2)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 3)
//			info.time = gEventName.GlobalRemainTime(n);
//
//		if (n == 4)
//			info.time = gServerDisplayer.EventCustomLottery;
//
//		if (n == 5)
//			info.time = gServerDisplayer.EventCustomQuiz;
//
//		if (n == 6)
//			info.time = gServerDisplayer.EventCustomBonus;
//
//		if (n == 7)
//			info.time = gServerDisplayer.EventMoss;
//
//		if (n == 8)
//			info.time = gServerDisplayer.EventDrop;
//
//		if (n == 9)
//			info.time = gServerDisplayer.EventKing;
//
//		if (n == 10)
//			info.time = gServerDisplayer.EventTvT;
//
//		if (n == 11)
//			info.time = -1;
//
//		if (n == 12)
//			info.time = -1;
//
//		if (n == 13)
//			info.time = -1;
//
//		if (n > 13)
//			info.time = -1;*/
//
//		pMsg.count++;
//
//		memcpy(&send[size], &info, sizeof(info));
//		size += sizeof(info);
//	}
//
//	for (int n = 0; n < 20; n++)
//	{
//		info.index = n + 20;
//
//		info.time = gEventName.InvasionRemainTime(n);
//
//		pMsg.count++;
//
//		memcpy(&send[size], &info, sizeof(info));
//		size += sizeof(info);
//	}
//
//	for (int n = 0; n < 20; n++)
//	{
//		info.index = n + 40;
//
//		info.time = gEventName.ArenaRemainTime(n);
//
//		pMsg.count++;
//
//		memcpy(&send[size], &info, sizeof(info));
//		size += sizeof(info);
//	}
//
//	pMsg.header.size[0] = SET_NUMBERHB(size);
//	pMsg.header.size[1] = SET_NUMBERLB(size);
//	// ---
//	memcpy(send, &pMsg, sizeof(pMsg));
//
//	DataSend(Index, send, size);
//
//#endif
//	return;
//}


int MaxPerPage = 14;
void CCustomEventTime::GCReqEventTime(int Index, PMSG_CUSTOM_EVENTTIME_RECV* lpMsg)
{

#if (GAMESERVER_CLIENTE_UPDATE >= 2)

	if (gServerInfo.m_CustomEventTimeSwitch == 0)
	{
		return;
	}

	if (gObjIsConnected(Index) == false)
	{
		return;
	}
	LogAdd(LOG_RED, "GCReqEventTime1");
	int GetPage = lpMsg->Page;

	BYTE send[2048];

	PMSG_CUSTOM_EVENTTIME_SEND pMsg;

	pMsg.header.set(0xF3, 0xE8, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.MaxList = gEventName.m_SendClientDataEventTime.size();

	CUSTOM_EVENTTIME_DATA info;

	for (int n = (GetPage * MaxPerPage); n < gEventName.m_SendClientDataEventTime.size(); n++)
	{
		if (pMsg.count >= MaxPerPage) break;

		int CatE = gEventName.m_SendClientDataEventTime[n].switch_on;
		int IndexE = gEventName.m_SendClientDataEventTime[n].m_Key;

		info.index = n;

		info.time = gEventName.GetTimeEventSwitch(CatE, IndexE);
		if (info.time == -1) continue;
		info.NumberGate = gEventName.m_SendClientDataEventTime[n].GetGate();
		memcpy(&info.NameEvent, gEventName.m_SendClientDataEventTime[n].GetName(), sizeof(info.NameEvent));
		memcpy(&info.DesString, gEventName.m_SendClientDataEventTime[n].GetDes(), sizeof(info.DesString));

		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(Index, send, size);

#endif
	return;
}