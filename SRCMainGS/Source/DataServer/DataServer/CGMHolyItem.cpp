#include "StdAfx.h"
#include "CGMHolyItem.h"
#include "QueryManager.h"
#include "SocketManager.h"

void CGMHolyItem::GDHolyInventoryRecv(SDHP_INVENTORY_HOLY_RECV* lpMsg, int index)
{
	SDHP_INVENTORY_HOLY_SEND pMsg;

	pMsg.header.set(0x41, 0x00, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account, lpMsg->account, sizeof(pMsg.account));

	if (gQueryManager.ExecQuery("SELECT HolyInventory FROM Character WHERE AccountID='%s' AND Name='%s'", lpMsg->account, lpMsg->name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		memset(pMsg.Inventory, 0xFF, sizeof(pMsg.Inventory));
	}
	else
	{
		gQueryManager.GetAsBinary("HolyInventory", pMsg.Inventory[0], sizeof(pMsg.Inventory));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
}

void CGMHolyItem::GDHolyInventorySaveRecv(SDHP_INVENTORY_HOLY_SAVE_RECV* lpMsg)
{
	gQueryManager.BindParameterAsBinary(1, lpMsg->Inventory[0], sizeof(lpMsg->Inventory));

	gQueryManager.ExecQuery("UPDATE Character SET HolyInventory=? WHERE AccountID='%s' AND Name='%s'", lpMsg->account, lpMsg->name);

	gQueryManager.Close();
}
