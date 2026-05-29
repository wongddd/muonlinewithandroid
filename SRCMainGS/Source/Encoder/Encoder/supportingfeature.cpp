#include "StdAfx.h"
#include "MemScript.h"
#include "supportingfeature.h"

void ConvertOptionIndex(int* OptionIndex) // OK
{
	switch ((*OptionIndex))
	{
	case 80://--ITEM_OPTION_ADD_PHYSI_DAMAGE
		(*OptionIndex) = 80;// ~Additional Dmg +%d
		break;
	case 81://--ITEM_OPTION_ADD_MAGIC_DAMAGE
		(*OptionIndex) = 81;// ~Additional Wizardry Dmg +%d
		break;
	case 82://--ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE
		(*OptionIndex) = 83;// ~Additional defense rate +%d
		break;
	case 83://--ITEM_OPTION_ADD_DEFENSE
		(*OptionIndex) = 84;// ~Additional defense +%d
		break;
	case 84://--ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE
		(*OptionIndex) = 85;// ~Luck (success rate of Jewel of Soul +25%%)
		break;
	case 85://--ITEM_OPTION_ADD_HP_RECOVERY_RATE
		(*OptionIndex) = 86;// ~Automatic HP recovery %d%%
		break;
	case 102://--ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE
		(*OptionIndex) = 103;// ~Ignor opponent's defensive power by %d%%
		break;
	case 109://--ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE
		(*OptionIndex) = 106;// ~Return's the enemy's attack power in %d%%
		break;
	case 110://--ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE
		(*OptionIndex) = 107;// ~Complete recovery of life in %d%% rate
		break;
	case 111://--ITEM_OPTION_ADD_DEFENSIVE_FULL_MP_RESTORE_RATE
		(*OptionIndex) = 108;// ~Complete recover of Mana in %d%% rate
		break;
	case 113://--ITEM_OPTION_ADD_CURSE_DAMAGE
		(*OptionIndex) = 82;// ~Additional Curse Spell +%d
		break;
	}
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_ITEM_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
	}

	try
	{
		int ItemIndexCount = 0;

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

			CUSTOM_ITEM_INFO info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.Itemindex = nIndex + (nType << 9);

			info.RenderColor = lpMemScript->GetAsCLRREF();

			strcpy_s(info.filename, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_MAPE_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_MAPE_INFO info;

			info.mapIndex = lpMemScript->GetNumber();

			info.checkIndex = lpMemScript->GetAsNumber();

			info.textIndex = lpMemScript->GetAsNumber();

			info.TerrainExt = lpMemScript->GetAsNumber();

			strcpy_s(info.ImgName, lpMemScript->GetAsString());

			strcpy_s(info.lpszMp3, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_IMAGE_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_IMAGE_INFO info;

			info.ImgIndex = lpMemScript->GetNumber();

			info.Wrap = lpMemScript->GetAsNumber();

			info.Type = lpMemScript->GetAsNumber();

			strcpy_s(info.ImgName, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_MONSTER_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_MONSTER_INFO info;

			info.MonsterIndex = lpMemScript->GetNumber();

			info.Kind = lpMemScript->GetAsNumber();

			info.fSize = lpMemScript->GetAsFloatNumber();

			strcpy_s(info.Name, lpMemScript->GetAsString());

			strcpy_s(info.directory, lpMemScript->GetAsString());

			strcpy_s(info.filename, lpMemScript->GetAsString());

			if (info.Kind == 1)
				info.Kind = KIND_MONSTER;
			else
				info.Kind = KIND_NPC;

			info.RenderIndex = 0;

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_WING_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_WING_INFO info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.ItemIndex = nIndex + (nType << 9);

			info.DefenseConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstB = lpMemScript->GetAsNumber();

			info.DecDamageConstA = lpMemScript->GetAsNumber();

			info.DecDamageConstB = lpMemScript->GetAsNumber();

			info.OptionIndex1 = lpMemScript->GetAsNumber();

			info.OptionValue1 = lpMemScript->GetAsNumber();

			info.OptionIndex2 = lpMemScript->GetAsNumber();

			info.OptionValue2 = lpMemScript->GetAsNumber();

			info.OptionIndex3 = lpMemScript->GetAsNumber();

			info.OptionValue3 = lpMemScript->GetAsNumber();

			info.NewOptionIndex1 = lpMemScript->GetAsNumber();

			info.NewOptionValue1 = lpMemScript->GetAsNumber();

			info.NewOptionIndex2 = lpMemScript->GetAsNumber();

			info.NewOptionValue2 = lpMemScript->GetAsNumber();

			info.NewOptionIndex3 = lpMemScript->GetAsNumber();

			info.NewOptionValue3 = lpMemScript->GetAsNumber();

			info.NewOptionIndex4 = lpMemScript->GetAsNumber();

			info.NewOptionValue4 = lpMemScript->GetAsNumber();

			info.ModelLinkType = lpMemScript->GetAsNumber();

			strcpy_s(info.filename, lpMemScript->GetAsString());

			ConvertOptionIndex(&info.OptionIndex1);

			ConvertOptionIndex(&info.OptionIndex2);

			ConvertOptionIndex(&info.OptionIndex3);

			ConvertOptionIndex(&info.NewOptionIndex1);

			ConvertOptionIndex(&info.NewOptionIndex2);

			ConvertOptionIndex(&info.NewOptionIndex3);

			ConvertOptionIndex(&info.NewOptionIndex4);

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_ITEM_STACK>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_ITEM_STACK info;

			info.ItemIndex = lpMemScript->GetNumber();

			info.Level = lpMemScript->GetAsNumber();

			info.MaxStack = lpMemScript->GetAsNumber();

			info.ItemConvert = lpMemScript->GetAsNumber();

			info.break_up = lpMemScript->GetAsNumber();

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_PET_STACK>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_PET_STACK info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.ItemIndex = nIndex + (nType << 9);

			info.RenderType = lpMemScript->GetAsNumber();

			info.MovementType = lpMemScript->GetAsNumber();

			info.RenderEffect = lpMemScript->GetAsNumber();

			info.RenderScale = lpMemScript->GetAsFloatNumber();

			strcpy_s(info.filename, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_EFFECT_STACK>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_EFFECT_STACK info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.ItemIndex = ITEM_DEF(nType, nIndex);

			info.Texture = lpMemScript->GetAsNumber();

			info.Join = lpMemScript->GetAsNumber();

			lpMemScript->GetAsNumber(); //--Level

			info.EffectColor = lpMemScript->GetAsCLRREFF();

			info.Scale = lpMemScript->GetAsFloatNumber();

			info.Angle = lpMemScript->GetAsFloatNumber();

			info.SubType = lpMemScript->GetAsNumber();

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_SMOKE_STACK>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_SMOKE_STACK info;

			info.ItemIndex = lpMemScript->GetNumber();

			info.EffectColor = lpMemScript->GetAsCLRREFF();

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}
			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_NPC_NAME>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_NPC_NAME info;

			info.IndexNpc = lpMemScript->GetNumber();

			info.WorldIndex = lpMemScript->GetAsNumber();

			info.PositionX = lpMemScript->GetAsNumber();

			info.PositionY = lpMemScript->GetAsNumber();

			strcpy_s(info.Name, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<TEMPLATE_MEMBERSHIP>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
	}

	try
	{
		size_t ItemIndexCount = 0;

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

			TEMPLATE_MEMBERSHIP info;

			info.Index = lpMemScript->GetNumber();

			info.CntExp = lpMemScript->GetAsNumber();

			info.CntDrop = lpMemScript->GetAsNumber();

			info.CntDays = lpMemScript->GetAsNumber();

			info.AccountLevel = lpMemScript->GetAsNumber();

			info.Coin[0] = lpMemScript->GetAsNumber();

			info.Coin[1] = lpMemScript->GetAsNumber();

			info.Coin[2] = lpMemScript->GetAsNumber();

			strcpy_s(info.text_name, lpMemScript->GetAsString());

			if (ItemIndexCount++ > size)
				goto FROM_EXIT_CODE;

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_COMMAND_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_COMMAND_INFO info;

			info.m_Index = lpMemScript->GetNumber();

			info.m_bColor = lpMemScript->GetAsNumber();

			strcpy_s(info.Command, lpMemScript->GetAsString());

			info.m_AccountLevel[0] = lpMemScript->GetAsNumber();

			info.m_AccountLevel[1] = lpMemScript->GetAsNumber();

			info.m_AccountLevel[2] = lpMemScript->GetAsNumber();

			info.m_AccountLevel[3] = lpMemScript->GetAsNumber();

			info.m_bGameMaster = lpMemScript->GetAsNumber();

			strcpy_s(info.Description, lpMemScript->GetAsString());

			strcpy_s(info.Information, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CHARACTER_NAME_EFFECT>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CHARACTER_NAME_EFFECT info;

			info.index = lpMemScript->GetNumber();

			strcpy_s(info.Name, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<RENDER_MESH_EFFECT>& item_info, size_t size, BYTE Type)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			RENDER_MESH_EFFECT info;

			if (Type == RENDER_MESH_OBJ_ITEM)
			{
				int nType = lpMemScript->GetNumber();

				int nIndex = lpMemScript->GetAsNumber();

				info.Index = ITEM_DEF(nType, nIndex);
			}
			else
			{
				info.Index = lpMemScript->GetNumber();
			}

			info.GroupId = lpMemScript->GetAsNumber(); //-- meshId

			info.RenderFlag = lpMemScript->GetAsNumber();

			info.Texture = lpMemScript->GetAsNumber();

			BOOL BodyEnable = lpMemScript->GetAsNumber();

			info.BodyLight = lpMemScript->GetAsCLRREFF();

			if (BodyEnable != TRUE)
			{
				info.BodyLight = 0;
			}

			info.BlendLight = lpMemScript->GetAsFloatNumber();

			info.RenderType = lpMemScript->GetAsNumber(); //-- EffectType

			info.TimeEffect = lpMemScript->GetAsFloatNumber();

			info.TimeTexture = lpMemScript->GetAsFloatNumber();

			if (Type == RENDER_MESH_OBJ_ITEM)
			{
				info.RenderLevel = lpMemScript->GetAsFloatNumber();
			}
			else
			{
				info.RenderLevel = FALSE;
			}

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<RENDER_MESH_EFFECT_DEFAULT>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			RENDER_MESH_EFFECT_DEFAULT info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.ItemIndex = ITEM_DEF(nType, nIndex);

			info.EffectIndex = lpMemScript->GetAsNumber(); //-- meshId

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_JEWEL_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			CUSTOM_JEWEL_INFO info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.ItemIndex = ITEM_DEF(nType, nIndex);

			info.MinItemLevel = lpMemScript->GetAsNumber();

			info.MaxItemLevel = lpMemScript->GetAsNumber();

			info.MaxItemOption1 = lpMemScript->GetAsNumber();

			info.MaxItemOption2 = lpMemScript->GetAsNumber();

			info.MinItemOption3 = lpMemScript->GetAsNumber();

			info.MaxItemOption3 = lpMemScript->GetAsNumber();

			info.MinItemNewOption = lpMemScript->GetAsNumber();

			info.MaxItemNewOption = lpMemScript->GetAsNumber();

			info.MaxItemSetOption = lpMemScript->GetAsNumber();

			info.MinItemSocketOption = lpMemScript->GetAsNumber();

			info.MaxItemSocketOption = lpMemScript->GetAsNumber();

			info.EnableSlotWeapon = lpMemScript->GetAsNumber();

			info.EnableSlotArmor = lpMemScript->GetAsNumber();

			info.EnableSlotWing = lpMemScript->GetAsNumber();

			info.SuccessRate[0] = lpMemScript->GetAsNumber();

			info.SuccessRate[1] = lpMemScript->GetAsNumber();

			info.SuccessRate[2] = lpMemScript->GetAsNumber();

			info.SuccessRate[3] = lpMemScript->GetAsNumber();

			info.SalePrice = lpMemScript->GetAsNumber();

			strcpy_s(info.Filename, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_unique_document_file(std::string filename, std::vector<WINGS_PHYSICS_INFO>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			WINGS_PHYSICS_INFO info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.ItemIndex = ITEM_DEF(nType, nIndex);

			info.RenderType = lpMemScript->GetAsNumber();

			info.iStrip = lpMemScript->GetAsNumber();

			info.iBoneLayer = lpMemScript->GetAsNumber();

			info.iBoneStrip = lpMemScript->GetAsNumber();

			strcpy_s(info.textureLayer, lpMemScript->GetAsString());

			strcpy_s(info.textureStrip, lpMemScript->GetAsString());

			strcpy_s(info.textureLayer_R, lpMemScript->GetAsString());

			strcpy_s(info.textureStrip_R, lpMemScript->GetAsString());

			if (item_info.size() > size)
			{
				goto FROM_EXIT_CODE;
			}

			item_info.push_back(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}

void runtime_kapocha_document_file(std::string filename, std::vector<RENDER_MESH_EFFECT>& item_info, size_t size)
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, filename.c_str());
		return;
	}

	if (lpMemScript->SetBuffer(filename) == false)
	{
		printf(lpMemScript->GetLastError());
		goto FROM_EXIT_CODE;
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

			RENDER_MESH_EFFECT info;

			int nType = lpMemScript->GetNumber();

			int nIndex = lpMemScript->GetAsNumber();

			info.Index = ITEM_DEF(nType, nIndex);

			info.BodyLight = TRUE;

			info.BodyLight = lpMemScript->GetAsCLRREFF();

			info.GroupId = lpMemScript->GetAsNumber(); //-- Group

			info.RenderFlag = lpMemScript->GetAsNumber(); //-- RenderEffect

			info.RenderType = lpMemScript->GetAsNumber(); //-- EffectType

			info.TimeEffect = lpMemScript->GetAsFloatNumber(); //-- TimeEffectType

			if (info.TimeEffect == 1.f)
			{
				info.TimeEffect = -1.f;
			}

			info.Texture = lpMemScript->GetAsNumber();

			info.TimeTexture = lpMemScript->GetAsFloatNumber();

			if (info.RenderType == 4 || info.RenderType >= 5)
			{
				info.BlendLight = 0.25f;
			}
			else
			{
				info.BlendLight = 1.f;
			}

			info.RenderLevel = lpMemScript->GetAsFloatNumber();

			item_info.push_back(info);

			if (item_info.size() >= size)
			{
				goto FROM_EXIT_CODE;
			}
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

FROM_EXIT_CODE:
	SAFE_DELETE(lpMemScript);
}
