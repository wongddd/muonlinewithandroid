#pragma once

struct CUSTOM_FLAG_INFO
{
	int ItemIndex;
	int IncDamageRate;
	int DecDamageRate;
	int Opt[MAX_ADD_OPTION_INFO];
	int Val[MAX_ADD_OPTION_INFO];
};


class CGMFlagNatManager
{
public:
	CGMFlagNatManager();
	virtual~CGMFlagNatManager();
	static CGMFlagNatManager* Instance();


	void Load(char* path);
	bool CheckFlagByItem(int ItemIndex);
	int GetItemIncDamageRate(int ItemIndex);
	int GetItemDecDamageRate(int ItemIndex);
	void CalcItemCommonOption(LPOBJ lpObj, bool flag);
	void InsertOption(LPOBJ lpObj, CItem* lpItem, bool flag);
	void UpdateInfo(int Index);
	int GetIndex(int index);
	int GetValue(int index);
private:
	std::map<int, CUSTOM_FLAG_INFO> m_ItemOption;
	std::map<int, CUSTOM_FLAG_INFO>::iterator CurrentInfo;
};

#define gmFlagManager		(CGMFlagNatManager::Instance())