// CustomItem.cpp: implementation of the CCustomItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CashShop.h"
#include "CustomBuyVip.h"
#include "DSProtocol.h"
#include "JSProtocol.h"
#include "Util.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"

CCustomBuyVip gCustomBuyVip;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomBuyVip::CCustomBuyVip() // OK
{
	this->Init();
}

CCustomBuyVip::~CCustomBuyVip() // OK
{
	shopList.clear();
}

void CCustomBuyVip::Init() // OK
{
	shopList.clear();
}

bool CCustomBuyVip::LoadData(const std::string& filename)
{
	this->Init();

	bool is_open = true;
	bool Success = false;

	Success = this->LoadXmlData(filename + ".xml", is_open);

	if (!Success && !is_open)
	{
		return this->LoadTxtData(filename + ".txt");
	}

	return Success;
}

bool CCustomBuyVip::LoadTxtData(const std::string& filename)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return false;
	}

	if (lpMemScript->SetBuffer(filename.c_str()) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return false;
	}


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

			CUSTOM_BUYVIP_INFO info;

			memset(&info, 0, sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.Exp = lpMemScript->GetAsNumber();

			info.Drop = lpMemScript->GetAsNumber();

			info.Days = lpMemScript->GetAsNumber();

			info.AccountLevel = lpMemScript->GetAsNumber();

			info.Coin[0] = lpMemScript->GetAsNumber();

			info.Coin[1] = lpMemScript->GetAsNumber();

			info.Coin[2] = lpMemScript->GetAsNumber();

			info.text_name = lpMemScript->GetAsString();

			this->SetInfo(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;
	return true;
}

bool CCustomBuyVip::LoadXmlData(const std::string& filename, bool& is_open)
{
	const char* lpszFileName = filename.c_str();

	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(lpszFileName);

	if (res.status != pugi::status_ok)
	{
		is_open = false;
		return false;
	}

	is_open = true;

	pugi::xml_node root = file.child("shopbuyvip");

	if (!root)
	{
		ErrorMessageBox("[shopbuyvip] shopbuyvip::LoadData() - Data Fail");
		return false;
	}

	for (pugi::xml_node child = root.child("item"); child; child = child.next_sibling())
	{
		CUSTOM_BUYVIP_INFO shopbuyvip;

		(&shopbuyvip)->Index = child.attribute("Index").as_int();

		(&shopbuyvip)->Exp = child.attribute("Exp").as_int();

		(&shopbuyvip)->Drop = child.attribute("Drop").as_int();

		(&shopbuyvip)->Days = child.attribute("Days").as_int();

		(&shopbuyvip)->AccountLevel = child.attribute("AccountLevel").as_int();

		(&shopbuyvip)->Coin[0] = child.attribute("Coin1").as_int();

		(&shopbuyvip)->Coin[1] = child.attribute("Coin2").as_int();

		(&shopbuyvip)->Coin[2] = child.attribute("Coin3").as_int();

		(&shopbuyvip)->text_name = child.attribute("VipName").as_string();

		this->SetInfo(shopbuyvip);
	}

	return true;
}

void CCustomBuyVip::SetInfo(CUSTOM_BUYVIP_INFO info) // OK
{
	if (info.Index > 0)
	{
		shopList.push_back(info);
	}
}

CUSTOM_BUYVIP_INFO* CCustomBuyVip::GetInfo(int index) // OK
{
	auto it = std::find_if(shopList.begin(), shopList.end(),
		[index](const CUSTOM_BUYVIP_INFO& res) {
			return res.Index == index;
		});

	return (it != shopList.end()) ? &(*it) : NULL;
}

void CCustomBuyVip::ReceiveBuyAccountVip(BUYPREMIUM_REQ* lpMsg, int aIndex)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) != 0)
	{
		CUSTOM_BUYVIP_INFO* shopbuyvip = this->GetInfo(lpMsg->vipIndex);

		if (shopbuyvip)
		{
			if (shopbuyvip->AccountLevel <= lpObj->AccountLevel)
			{
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(759));
				return;
			}

			lpObj->vipIndex = lpMsg->vipIndex;

			gCashShop.CGCashShopPointRecv(lpObj->Index);
		}
	}
#endif
}

void CCustomBuyVip::AddTime(LPOBJ lpObj)
{
#if (GAMESERVER_CLIENTE_UPDATE >= 6)
	if (gObjIsConnectedGP(lpObj->Index) == 0 || lpObj->vipIndex <= 0)
	{
		return;
	}

	if (lpObj->vipIndex <= lpObj->AccountLevel)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(759));
		return;
	}

	CUSTOM_BUYVIP_INFO* shopbuyvip = this->GetInfo(lpObj->vipIndex);

	if (shopbuyvip)
	{
		if ((shopbuyvip->Coin[0] != 0 && lpObj->Coin1 < shopbuyvip->Coin[0])
			|| (shopbuyvip->Coin[1] != 0 && lpObj->Coin2 < shopbuyvip->Coin[1])
			|| (shopbuyvip->Coin[2] != 0 && lpObj->Coin2 < shopbuyvip->Coin[2]))
		{
			lpObj->vipIndex = 0;
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(757));
			return;
		}

		LogAdd(LOG_EVENT, "[%s][%s] vipIndex (Level: %d)", lpObj->Account, lpObj->Name, lpObj->vipIndex);

		GDSetCoinSend(lpObj->Index, -(shopbuyvip->Coin[0]), -(shopbuyvip->Coin[1]), -(shopbuyvip->Coin[2]), "shop vip");

		GJAccountLevelSaveSend(lpObj->Index, shopbuyvip->AccountLevel, shopbuyvip->Days * 86400);

		GJAccountLevelSend(lpObj->Index);

		gCashShop.CGCashShopPointRecv(lpObj->Index);

		if (!shopbuyvip->text_name.empty())
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GlobalText(758), shopbuyvip->text_name.c_str());
		}

		lpObj->vipIndex = 0;
	}
#endif
}
