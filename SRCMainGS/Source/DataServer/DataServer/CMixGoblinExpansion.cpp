#include "StdAfx.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "CMixGoblinExpansion.h"

#ifdef CHAOS_MACHINE_EXTENSION


CMixGoblinExpansion gGoblinExpansion;

CMixGoblinExpansion::CMixGoblinExpansion()
{
}

CMixGoblinExpansion::~CMixGoblinExpansion()
{
}

void CMixGoblinExpansion::GDInventoryRecv(SDHP_MIX_GOBLIN_ITEM* lpMsg, int index)
{
	SDHP_MIX_GOBLIN_ITEM_SEND pMsg;

	pMsg.header.set(0xF8, 0x00, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.Account, lpMsg->Account, sizeof(pMsg.Account));

	memcpy(pMsg.iName, lpMsg->Name, sizeof(pMsg.iName));

	if (gQueryManager.ExecQuery("SELECT * FROM GoblinMix WHERE AccountID='%s' AND Name='%s'", lpMsg->Account, lpMsg->Name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.m_iMixIndex = 0xFF;

		memset(pMsg.Inventory, 0xFF, sizeof(pMsg.Inventory));
	}
	else
	{
		gQueryManager.GetAsBinary("Items", pMsg.Inventory[0], sizeof(pMsg.Inventory));

		pMsg.m_iMixIndex = gQueryManager.GetAsInteger("iMixIndex");

		pMsg.m_iMixState = gQueryManager.GetAsInteger("iMixState");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, sizeof(pMsg));
}

void CMixGoblinExpansion::GDInventorySaveRecv(SDHP_MIX_GOBBLIN_ITEM_SAVE_RECV* lpMsg)
{
	if (gQueryManager.ExecQuery("SELECT Name FROM GoblinMix WHERE Name='%s'", lpMsg->iName) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.BindParameterAsBinary(1, lpMsg->Inventory[0], sizeof(lpMsg->Inventory));
		gQueryManager.ExecQuery("INSERT INTO GoblinMix (AccountID,Name,Items,iMixIndex,iMixState) VALUES ('%s','%s',?, %d, %d)", lpMsg->Account, lpMsg->iName, lpMsg->m_iMixIndex, lpMsg->m_iMixState);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.BindParameterAsBinary(1, lpMsg->Inventory[0], sizeof(lpMsg->Inventory));
		gQueryManager.ExecQuery("UPDATE GoblinMix SET Items=?, iMixIndex=%d, iMixState=%d WHERE Name='%s'", lpMsg->m_iMixIndex, lpMsg->m_iMixState, lpMsg->iName);
		gQueryManager.Close();
	}
}

#endif // CHAOS_MACHINE_EXTENSION

