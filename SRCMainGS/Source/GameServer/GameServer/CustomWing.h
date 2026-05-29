#pragma once
#define MAX_CUSTOM_WING 100
extern int OpcionExcAdd(int m_nOptionR);

class CUSTOM_WING_INFO
{
public:
	CUSTOM_WING_INFO()
	{
		ItemIndex = 0;
		DefenseConstA = 0;
		IncDamageConstA = 0;
		IncDamageConstB = 0;
		DecDamageConstA = 0;
		DecDamageConstB = 0;
		memset(OptionIndex, 0, sizeof(OptionIndex));
		memset(OptionValue, 0, sizeof(OptionValue));
		memset(NewOptionIndex, 0, sizeof(NewOptionIndex));
		memset(NewOptionValue, 0, sizeof(NewOptionValue));
	}
	~CUSTOM_WING_INFO() {};
public:
	int ItemIndex;
	int DefenseConstA;
	int IncDamageConstA;
	int IncDamageConstB;
	int DecDamageConstA;
	int DecDamageConstB;
	int OptionIndex[3];
	int OptionValue[3];
	int NewOptionIndex[4];
	int NewOptionValue[4];

	int GetCustomWingDefense(int ItemLevel)
	{
		return (DefenseConstA * ItemLevel);
	};
	int GetCustomWingIncDamage(int ItemLevel)
	{
		return (IncDamageConstA + (ItemLevel * IncDamageConstB));
	};
	int GetCustomWingDecDamage(int ItemLevel)
	{
		return (DecDamageConstA - (ItemLevel * DecDamageConstB));
	};
	int GetCustomWingOptionIndex(int OptionNumber)
	{
		int m_OptionIndex = 0;

		if (OptionNumber >= 0 && OptionNumber < 3)
		{
			m_OptionIndex = OptionIndex[OptionNumber];
		}
		return m_OptionIndex;
	};
	int GetCustomWingOptionValue(int OptionNumber)
	{
		int m_OptionValue = 0;

		if (OptionNumber >= 0 && OptionNumber < 3)
		{
			m_OptionValue = OptionValue[OptionNumber];
		}
		return m_OptionValue;
	};
	int GetCustomWingNewOptionIndex(int OptionNumber)
	{
		int m_NewOption = 0;

		if (OptionNumber >= 0 && OptionNumber < 4)
		{
			m_NewOption = NewOptionIndex[OptionNumber];
		}

		return m_NewOption;//OpcionExcAdd(m_NewOption);
	};
	int GetCustomWingNewOptionValue(int OptionNumber)
	{
		int m_OptionValue = 0;

		if (OptionNumber >= 0 && OptionNumber < 4)
		{
			m_OptionValue = NewOptionValue[OptionNumber];
		}

		return m_OptionValue;
	};
};

class CCustomWing
{
public:
	CCustomWing();
	virtual ~CCustomWing();
	void Init();
	void Load(char* path);
	void Mng__Register(int Index, CUSTOM_WING_INFO info);
	CUSTOM_WING_INFO* findWings007(int ItemIndex);

	bool CheckCustomWingByItem(int ItemIndex);
public:
	std::map<int, CUSTOM_WING_INFO> m_nBackByItem;
};

extern CCustomWing gCustomWing;
