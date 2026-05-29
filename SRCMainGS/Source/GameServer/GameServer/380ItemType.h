// 380ItemType.h: interface for the C380ItemType class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_380_ITEM_OPTION_INDEX 2

struct ITEM_380_TYPE_INFO
{
	int Index;
	int OptionIndex[MAX_380_ITEM_OPTION_INDEX];
	int OptionValue[MAX_380_ITEM_OPTION_INDEX];
};

typedef struct _ITEM_ADD_OPTION
{
	BYTE	m_byOption1;
	WORD	m_byValue1;
	BYTE	m_byOption2;
	WORD	m_byValue2;
	BYTE	m_Type;
	DWORD	m_Time;

	_ITEM_ADD_OPTION()
	{
		m_byOption1 = 0;
		m_byValue1 = 0;
		m_byOption2 = 0;
		m_byValue2 = 0;
		m_Type = 0;
		m_Time = 0;
	}
} ITEM_ADD_OPTION;

typedef std::map<int, ITEM_380_TYPE_INFO> type_map_option380;

class C380ItemType
{
public:
	C380ItemType();
	virtual ~C380ItemType();
	void Load(char* path);
	void ExportXML(std::string filename);
	void ExportBMD(std::string filename);


	bool Check380ItemType(int index);
	int Get380ItemOptionIndex(int index,int number);
	int Get380ItemOptionValue(int index,int number);
private:
	type_map_option380 m_380ItemTypeInfo;
};

extern C380ItemType g380ItemType;
