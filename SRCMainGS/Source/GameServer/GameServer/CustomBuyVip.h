// CustomItem.h: interface for the CCustomItem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Protocol.h"

struct BUYPREMIUM_REQ
{
	PSBMSG_HEAD h;
	int	vipIndex;
};

struct CUSTOM_BUYVIP_INFO
{
	int Index;
	int Exp;
	int Drop;
	int Days;
	int Coin[3];
	int AccountLevel;
	std::string text_name;
};

class CCustomBuyVip
{
public:
	CCustomBuyVip();
	virtual ~CCustomBuyVip();
	void Init();
	bool LoadData(const std::string& filename);
	bool LoadTxtData(const std::string& filename);
	bool LoadXmlData(const std::string& filename, bool& is_open);

	void SetInfo(CUSTOM_BUYVIP_INFO info);
	CUSTOM_BUYVIP_INFO* GetInfo(int index);
	void ReceiveBuyAccountVip(BUYPREMIUM_REQ* lpMsg, int aIndex);
	void AddTime(LPOBJ lpObj);
public:
	std::vector<CUSTOM_BUYVIP_INFO> shopList;
	//CUSTOM_BUYVIP_INFO m_CustomBuyVipInfo[MAX_CUSTOM_BUYVIP];
};

extern CCustomBuyVip gCustomBuyVip;

#define gShopbuyvip		(&gCustomBuyVip)