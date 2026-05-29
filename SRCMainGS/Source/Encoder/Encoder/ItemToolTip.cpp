#include "stdafx.h"
#include "ItemTooltip.h"
#include "MemScript.h"

nCInfo GInfo;

nCInfo::nCInfo()
{
	this->Init();
}
nCInfo::~nCInfo()
{
}


void nCInfo::Init() // OK
{
	for (int n = 0; n < MaxLineItemToolTip; n++)
	{
		this->n_TRSTooltipData[n].ItemIndexMin = -1;
		this->n_TRSTooltipData[n].ItemIndexMax = -1;
		this->n_TRSTooltipText[n].Index = -1;

		this->n_TRSTooltipSetData[n].ItemIndexMin = -1;
		this->n_TRSTooltipSetData[n].ItemIndexMax = -1;
	};
}

void nCInfo::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		printf(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}
	printf("Load ==> '%s' \n", path);
	int CustomItemIndex1Count = 0;
	int CustomItemIndex1CountSet = 0;

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if (section == 0)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					nInformation info;

					memset(&info, 0, sizeof(info));


					info.Index = CustomItemIndex1Count;

					info.ItemIndexMin = lpMemScript->GetNumber();

					info.ItemIndexMax = lpMemScript->GetAsNumber();

					info.LevelMin = lpMemScript->GetAsNumber();
					info.LevelMax = lpMemScript->GetAsNumber();

					info.Exc = lpMemScript->GetAsNumber();
					info.Anc = lpMemScript->GetAsNumber();

					strcpy_s(info.OptionName, lpMemScript->GetAsString());

					info.OptionColor = lpMemScript->GetAsNumber();

					for (int i = 0; i < 15; i++)
					{
						info.TextIndex[i] = lpMemScript->GetAsNumber();
						info.TextColor[i] = lpMemScript->GetAsNumber();
						info.LevelItem[i] = lpMemScript->GetAsNumber();
					}
					if (CustomItemIndex1Count >= MaxLineItemToolTip) break;
					this->n_TRSTooltipData[CustomItemIndex1Count] = info;
					//printf("ToolTipItem: '%d' ==> '%s' \n", CustomItemIndex1CountSet, info.OptionName);
					CustomItemIndex1Count++;
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					nInformation info;

					memset(&info, 0, sizeof(info));


					info.Index = CustomItemIndex1CountSet;

					info.ItemIndexMin = lpMemScript->GetNumber();

					info.ItemIndexMax = lpMemScript->GetAsNumber();

					info.LevelMin = 0;
					info.LevelMax = 15;

					info.Exc = lpMemScript->GetAsNumber();
					info.Anc = -1;

					strcpy_s(info.OptionName, lpMemScript->GetAsString());

					info.OptionColor = lpMemScript->GetAsNumber();

					for (int i = 0; i < 15; i++)
					{
						info.TextIndex[i] = lpMemScript->GetAsNumber();
						info.TextColor[i] = lpMemScript->GetAsNumber();
						info.LevelItem[i] = lpMemScript->GetAsNumber();
					}

					if (CustomItemIndex1CountSet >= MaxLineItemToolTip) break;
					this->n_TRSTooltipSetData[CustomItemIndex1CountSet] = info;
					//printf("ToolTipSet: '%d' ==> '%s' \n", CustomItemIndex1CountSet, info.OptionName);
					CustomItemIndex1CountSet++;
				}
				else
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void nCInfo::Load2(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		printf(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	static int CustomItemIndex1Count = 0;
	static int CustomItemIndex2Count = 0;

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
			nText info1;

			memset(&info1, 0, sizeof(info1));

			info1.Index = lpMemScript->GetNumber();

			strcpy_s(info1.Text, lpMemScript->GetAsString());

			this->SetInfo2(info1, CustomItemIndex2Count);

			CustomItemIndex2Count++;
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void nCInfo::SetInfo1(nInformation info, int CustomItemIndexCount) // OK
{
	if (CustomItemIndexCount >= MaxLineItemToolTip)
	{
		return;
	}

	this->n_TRSTooltipData[CustomItemIndexCount] = info;
}

void nCInfo::SetInfo2(nText info, int CustomItemIndexCount) // OK
{
	if (CustomItemIndexCount >= MaxLineItemToolTip)
	{
		return;
	}

	this->n_TRSTooltipText[CustomItemIndexCount] = info;
}
