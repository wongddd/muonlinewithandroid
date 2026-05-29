#include "StdAfx.h"
#include "Util.h"
#include "Notice.h"
#include "Message.h"
#include "GameMain.h"
#include "MemScript.h"
#include "DSProtocol.h"
#include "GMHolyItem.h"
#include "ItemStack.h"

CGMHolyItem::CGMHolyItem()
{
	bItemCountMax = 8223372036854775807;
	bItemCountMin = 0x0000000000000000;

}

CGMHolyItem::~CGMHolyItem()
{
	m_bItems.clear();
}

void CGMHolyItem::LoadInfo(char* filename)
{
#ifdef INVENTORY_HOLY_JOH
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, filename);
		return;
	}

	if (lpMemScript->SetBuffer(filename) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	m_bItems.clear();

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if (strcmp("end", lpMemScript->GetString()) == 0)
			{
				break;
			}

			template_holy info;

			info.ItemIndex = lpMemScript->GetNumber();

			info.ItemLevel = lpMemScript->GetAsNumber();

			info.BundledIndex = lpMemScript->GetAsNumber();

			info.ValueUnit = lpMemScript->GetAsNumber();

			for(int i = 0; i < MAX_JEWEL_PACKAGE; i++)
			{
				info.ValuePack[i] = lpMemScript->GetAsNumber();
			}

			lpMemScript->GetAsString();

			m_bItems.push_back(info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
#endif // INVENTORY_HOLY_JOH
}

bool CGMHolyItem::GetInfo(int index, template_holy* lpInfo)
{
	if (index < m_bItems.size())
	{
		(*lpInfo) = m_bItems[index];
		return true;
	}
	return false;
}

void CGMHolyItem::DGHolyInventoryReq(int aIndex)
{
#ifdef INVENTORY_HOLY_JOH
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsAccountValid(aIndex, lpObj->Account) == 0)
	{
		LogAdd(LOG_RED, "[DGHolyInventoryReq] Invalid Account [%d](%s)", aIndex, lpObj->Account);
		return;
	}

	if (lpObj->LoadHolyInventory != 0)
	{
		return;
	}

	SDHP_INVENTORY_HOLY_SEND pMsg;

	pMsg.header.set(0x41, 0x00, sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account, gObj[aIndex].Account, sizeof(pMsg.account));

	memcpy(pMsg.name, gObj[aIndex].Name, sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.header.size);
#endif // INVENTORY_HOLY_JOH
}

void CGMHolyItem::DGHolyInventoryRecv(SDHP_INVENTORY_HOLY_RECV* lpMsg)
{
#ifdef INVENTORY_HOLY_JOH
	if (gObjIsAccountValid(lpMsg->index, lpMsg->account) == 0)
	{
		LogAdd(LOG_RED, "[DGHolyInventoryRecv] Invalid Account [%d](%s)", lpMsg->index, lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}


	LPOBJ lpObj = &gObj[lpMsg->index];

	if (lpObj->LoadHolyInventory != 0)
	{
		return;
	}

	lpObj->LoadHolyInventory = 1;

	for (int n = 0; n < HOLY_INVENTORY_SIZE; n++)
	{
		DBItemByteConvert(n, lpMsg->Inventory[n], &lpObj->HolyInventory[n]);
	}

	GCInventoryListSend(lpMsg->index);
#endif // INVENTORY_HOLY_JOH
}

void CGMHolyItem::DGHolyInventorySave(int aIndex)
{
#ifdef INVENTORY_HOLY_JOH
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->LoadHolyInventory == 0)
	{
		return;
	}

	SDHP_INVENTORY_HOLY_SAVE_SEND pMsg;

	pMsg.header.set(0x41, 0x01, sizeof(pMsg));

	memcpy(pMsg.account, lpObj->Account, sizeof(pMsg.account));

	memcpy(pMsg.name, lpObj->Name, sizeof(pMsg.name));

	for (int n = 0; n < HOLY_INVENTORY_SIZE; n++)
	{
		ItemByteConvert(n, pMsg.Inventory[n], lpObj->HolyInventory[n]);
	}

	gDataServerConnection.DataSend((BYTE*)&pMsg, sizeof(pMsg));
#endif // INVENTORY_HOLY_JOH
}

void CGMHolyItem::GCInventoryListSend(int aIndex)
{
#ifdef INVENTORY_HOLY_JOH
	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[512];

	PMSG_INVENTORY_HOLY_SEND pMsg;

	pMsg.header.setE(0x4F, 0x01, 0);

	int size = sizeof(pMsg);

	pMsg.Value = 0;

	PMSG_INVENTORY_HOLY info;

	for (size_t n = 0; n < HOLY_INVENTORY_SIZE; n++)
	{
		if (lpObj->HolyInventory[n].bItemindex != -1)
		{
			info.Inventory[11] = n;

			this->ItemByteConvert(n, info.Inventory, lpObj->HolyInventory[n]);

			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);

			pMsg.Value++;
		}
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
#endif // INVENTORY_HOLY_JOH
}

void CGMHolyItem::ItemByteConvert(int index, BYTE* btData, HOLY_INVENTORY lpItem)
{
	unsigned __int64 bItemCount = (unsigned)lpItem.bItemCount;

	memcpy(btData + 8, &lpItem.bItemindex, 2);

	btData[11] = index;

	btData[0] = (BYTE)(bItemCount & 0xFF);

	btData[1] = (BYTE)((bItemCount >> 8) & 0xFF);

	btData[2] = (BYTE)((bItemCount >> 16) & 0xFF);

	btData[3] = (BYTE)((bItemCount >> 24) & 0xFF);

	btData[4] = (BYTE)((bItemCount >> 32) & 0xFF);

	btData[5] = (BYTE)((bItemCount >> 40) & 0xFF);

	btData[6] = (BYTE)((bItemCount >> 48) & 0xFF);

	btData[7] = (BYTE)((bItemCount >> 56) & 0xFF);

	btData[10] = lpItem.bItemLevel;
}

void CGMHolyItem::DBItemByteConvert(int index, BYTE* btData, HOLY_INVENTORY* lpItem)
{
	lpItem->bItemCount = *(__int64*)btData;

	lpItem->bItemCount = 0x0000000000000000;

	// Supongamos que btData tiene 12 bytes y necesitas extraer bItemCount de los primeros 6 bytes
	lpItem->bItemCount |= (__int64)btData[0];

	lpItem->bItemCount |= (__int64)btData[1] << 8;

	lpItem->bItemCount |= (__int64)btData[2] << 16;

	lpItem->bItemCount |= (__int64)btData[3] << 24;

	lpItem->bItemCount |= (__int64)btData[4] << 32;

	lpItem->bItemCount |= (__int64)btData[5] << 40;

	if (lpItem->bItemCount < bItemCountMin)
	{
		lpItem->bItemCount = bItemCountMin;
	}
	if (lpItem->bItemCount > bItemCountMax)
	{
		lpItem->bItemCount = bItemCountMax;
	}

	if (index < m_bItems.size())
	{
		if (lpItem->bItemindex == -1 || lpItem->bItemindex != m_bItems[index].ItemIndex)
		{
			lpItem->bItemindex = m_bItems[index].ItemIndex;
		}

		lpItem->bItemLevel = m_bItems[index].ItemLevel;
	}
}

void CGMHolyItem::GCInventoryRemove(int aIndex, PMSG_INVENTORY_HOLY_RECV* Data)
{
#ifdef INVENTORY_HOLY_JOH
	LPOBJ lpObj = &gObj[aIndex];

	BYTE btType = Data->btType;

	BYTE btIndex = Data->btIndex;

	DWORD btValue = Data->btValue;

	template_holy item_info;

	if (btIndex > HOLY_INVENTORY_SIZE)
	{
		return;
	}

	if (this->GetInfo(btIndex, &item_info) == false)
	{
		return;
	}

	HOLY_INVENTORY* ItemHoly = &lpObj->HolyInventory[btIndex];


	__int64 btValueReal = btValue;
	__int64 btStockBank = ItemHoly->bItemCount;

	int btBundledLevel = (btType - 1);
	int ItemIndex = item_info.ItemIndex;
	int ItemLevel = item_info.ItemLevel;

	int BundledIndex = item_info.BundledIndex;

	if (btBundledLevel >= 0 && btBundledLevel < MAX_JEWEL_PACKAGE)
	{
		btValueReal = btValue * item_info.ValuePack[btBundledLevel];
	}

	if (btStockBank <= bItemCountMin || btValueReal > btStockBank)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "%s: (%lld of %lld)", gMessage.GlobalText(822), btValueReal, btStockBank);
		return;
	}

	bool transaction_concluded = false;

	if (btType == 0)
	{
		if (btValueReal > 0 && gItemManager.CheckItemInventorySpace(lpObj, ItemIndex, btValueReal, true, true) == false)
		{
			gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(824));
			return;
		}

		for (size_t i = 0; i < btValueReal; i++)
		{
			if(ItemLevel != -1)
				GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, ItemIndex, (BYTE)ItemLevel, 1, 0, 0, 0, lpObj->Index, 0, 0, 0, 0, 0, 0, 0);
			else
				GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, ItemIndex, (BYTE)0, 1, 0, 0, 0, lpObj->Index, 0, 0, 0, 0, 0, 0, 0);
		}
		transaction_concluded = true;
 	}
	else if (btBundledLevel >= 0 && btBundledLevel < MAX_JEWEL_PACKAGE)
	{
		if (BundledIndex != -1)
		{
			__int64 _BundledVal = btValue;

			if (_BundledVal > 0 && gItemManager.CheckItemInventorySpace(lpObj, BundledIndex, _BundledVal, true, true) == false)
			{
				gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(824));
				return;
			}

			for (size_t i = 0; i < _BundledVal; i++)
			{
				GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, BundledIndex, (BYTE)btBundledLevel, 1, 0, 0, 0, lpObj->Index, 0, 0, 0, 0, 0, 0, 0);
			}
			transaction_concluded = true;
		}
	}

	if (transaction_concluded)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(823));

		ItemHoly->bItemCount -= btValueReal;

		GCInventoryListSend(aIndex);
	}
	else
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "[%s][BundledIndex:%d | btBundledLevel:%d | btValueReal:%d]", gMessage.GlobalText(822), BundledIndex, btBundledLevel, btValueReal);
	}
#endif
}

void CGMHolyItem::GCInventoryAddItem(int aIndex, PMSG_INVENTORY_HOLY_RECV* Data)
{
#ifdef INVENTORY_HOLY_JOH
	LPOBJ lpObj = &gObj[aIndex];

	BYTE btType = Data->btType;

	BYTE btIndex = Data->btIndex;

	DWORD btValue = Data->btValue;

	template_holy item_info;

	if (btIndex > HOLY_INVENTORY_SIZE)
	{
		return;
	}

	if (this->GetInfo(btIndex, &item_info) == false)
	{
		return;
	}

	HOLY_INVENTORY* ItemHoly = &lpObj->HolyInventory[btIndex];

	if (lpObj->Lock > 0)
	{
		//gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 2, 0, gMessage.GlobalText(778));
		return;
	}

	__int64 btValueReal = 0;

	int ItemIndex = item_info.ItemIndex;
	int ItemLevel = item_info.ItemLevel;
	int BundledIndex = item_info.BundledIndex;
	__int64 PackBundled1 = item_info.ValuePack[0];
	__int64 PackBundled2 = item_info.ValuePack[1];
	__int64 PackBundled3 = item_info.ValuePack[2];


	int MaxValue = gItemManager.GetInventoryMaxValue(lpObj);

	for (int i = MaxValue-1; i >= INVENTORY_WEAR_SIZE; i--)
	{
		CItem* Item_info = &lpObj->Inventory[i];

		if (Item_info->IsItem())
		{
			if (Item_info->m_Index == ItemIndex && (ItemLevel == -1 || ItemLevel == Item_info->m_Level))
			{
				if (gItemStack.GetItemMaxStack(ItemIndex, ItemLevel) != 0)
				{
					if (Item_info->m_Durability >= 1)
					{
						btValueReal += Item_info->m_Durability;
					}
				}
				else
				{
					btValueReal++;
				}

				gItemManager.InventoryDelItem(lpObj->Index, i);
				gItemManager.GCItemDeleteSend(lpObj->Index, i, 1);
			}
			else if (Item_info->m_Index == BundledIndex && Item_info->m_Level == 2)
			{
				btValueReal+= PackBundled3;
				gItemManager.InventoryDelItem(lpObj->Index, i);
				gItemManager.GCItemDeleteSend(lpObj->Index, i, 1);
			}
			else if (Item_info->m_Index == BundledIndex && Item_info->m_Level == 1)
			{
				btValueReal += PackBundled2;
				gItemManager.InventoryDelItem(lpObj->Index, i);
				gItemManager.GCItemDeleteSend(lpObj->Index, i, 1);
			}
			else if (Item_info->m_Index == BundledIndex && Item_info->m_Level == 0)
			{
				btValueReal += PackBundled1;
				gItemManager.InventoryDelItem(lpObj->Index, i);
				gItemManager.GCItemDeleteSend(lpObj->Index, i, 1);
			}
		}
	}

	if (btValueReal > bItemCountMin)
	{
		ItemHoly->bItemCount += btValueReal;
		GCInventoryListSend(aIndex);
	}
#endif
}

