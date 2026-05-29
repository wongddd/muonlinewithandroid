#pragma once



struct nInformation
{
	int Index;
	int ItemIndexMin;
	int ItemIndexMax;
	int LevelMin;
	int LevelMax;
	int Exc;
	int Anc;
	//--
	char OptionName[32];
	int OptionColor;
	//--
	int TextIndex[15];
	int TextColor[15];
	int LevelItem[15];
};
struct nText
{
	int Index;
	char Text[254];
};

class nCInfo
{
public:
	nCInfo();
	virtual ~nCInfo();
	void Init();
	void Load(char* path);
	void Load2(char* path); // OK
	void SetInfo1(nInformation info, int CustomItemIndexCount);
	void SetInfo2(nText info, int CustomItemIndexCount);
public:
	nInformation n_TRSTooltipData[MaxLineItemToolTip]; //
	nInformation n_TRSTooltipSetData[MaxLineItemToolTip]; //Set
	nText n_TRSTooltipText[MaxLineItemToolTip];
};

extern nCInfo GInfo;