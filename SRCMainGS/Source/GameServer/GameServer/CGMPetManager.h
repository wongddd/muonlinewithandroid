#pragma once

struct CUSTOM_PET_INFO
{
	int ItemIndex;
	int IncDamageRate;
	int DecDamageRate;
	int Opt[MAX_ADD_OPTION_INFO];
	int Val[MAX_ADD_OPTION_INFO];
	int Inmortal;
};

class CGMPetManager
{
public:
	CGMPetManager();
	virtual~CGMPetManager();
	static CGMPetManager* Instance();


	void Load(char* path);
	bool CheckPetByItem(int ItemIndex);
	bool PetIsInmortal(int ItemIndex);
	int GetItemPetIncDamageRate(int ItemIndex);
	int GetItemPetDecDamageRate(int ItemIndex);
	void CalcItemCommonOption(LPOBJ lpObj, bool flag);
	void InsertOption(LPOBJ lpObj, CItem* lpItem, bool flag);
	void UpdateInfo(int Index);
	int GetIndex(int index);
	int GetValue(int index);
private:
	std::map<int, CUSTOM_PET_INFO> m_ItemOption;
	std::map<int, CUSTOM_PET_INFO>::iterator CurrentInfo;
};

#define gmPetManager		(CGMPetManager::Instance())