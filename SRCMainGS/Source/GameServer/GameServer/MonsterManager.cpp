// MonsterManager.cpp: implementation of the CMonsterManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterManager.h"
#include "380ItemType.h"
#include "BloodCastle.h"
#include "BonusManager.h"
#include "CastleDeep.h"
#include "CastleSiege.h"
#include "ChaosCastle.h"
#include "Crywolf.h"
#include "CustomArena.h"
#include "CustomEventDrop.h"
#include "CustomOnlineLottery.h"
#include "CustomQuiz.h"
#include "DevilSquare.h"
#include "IllusionTemple.h"
#include "InvasionManager.h"
#include "ItemManager.h"
#include "EventGvG.h"
#include "EventTvT.h"
#include "Map.h"
#include "MemScript.h"
#include "Monster.h"
#include "MonsterSetBase.h"
#include "MossMerchant.h"
#include "ObjectManager.h"
#include "Raklion.h"
#include "ServerInfo.h"
#include "SocketItemType.h"
#include "User.h"
#include "Util.h"
#include "ReiDoMu.h"
#include "EventFindPath.h"
#include "MapServerManager.h"

CMonsterManager gMonsterManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterManager::CMonsterManager() // OK
{
	this->Init();
}

CMonsterManager::~CMonsterManager() // OK
{

}

void CMonsterManager::Init() // OK
{
	for (int n = 0; n < MAX_MONSTER_INFO; n++)
	{
		this->m_MonsterInfo[n].Index = -1;
	}

	for (int n = 0; n < MAX_MONSTER_LEVEL; n++)
	{
		memset(this->m_MonsterItemInfo[n].IndexTable, -1, sizeof(this->m_MonsterItemInfo[n].IndexTable));

		this->m_MonsterItemInfo[n].IndexCount = 0;
	}
}

void CMonsterManager::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

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

			MONSTER_INFO info;

			memset(&info, 0, sizeof(info));

			info.Index = lpMemScript->GetNumber();         // Mã số ID của quái vật
			info.Rate = lpMemScript->GetAsNumber();        // Tỷ lệ xuất hiện (hoặc tỷ lệ liên quan đến map, event)
			strcpy_s(info.Name, lpMemScript->GetAsString()); // Tên quái vật

			info.Level = lpMemScript->GetAsNumber();       // Cấp độ của quái
			info.Life = lpMemScript->GetAsNumber();        // Lượng máu (HP)
			info.Mana = lpMemScript->GetAsNumber();        // Mana (nếu có sử dụng kỹ năng phép)

			info.DamageMin = lpMemScript->GetAsNumber();   // Sát thương tối thiểu
			info.DamageMax = lpMemScript->GetAsNumber();   // Sát thương tối đa
			info.Defense = lpMemScript->GetAsNumber();     // Phòng thủ vật lý
			info.MagicDefense = lpMemScript->GetAsNumber(); // Phòng thủ phép

			info.AttackRate = lpMemScript->GetAsNumber();  // Chủ số tấn công (ATK Success Rate)
			info.DefenseRate = lpMemScript->GetAsNumber(); // Chủ số phòng thủ (DEF Success Rate)

			info.MoveRange = lpMemScript->GetAsNumber();   // Tầm di chuyển
			info.AttackType = lpMemScript->GetAsNumber();  // Loại hình tấn công (cận chiến, phép, v.v.)
			info.AttackRange = lpMemScript->GetAsNumber(); // Tầm tấn công
			info.ViewRange = lpMemScript->GetAsNumber();   // Tầm nhìn (phát hiện kẻ địch)

			info.MoveSpeed = lpMemScript->GetAsNumber();   // Tốc độ di chuyển
			info.AttackSpeed = lpMemScript->GetAsNumber(); // Tốc độ đánh
			info.RegenTime = lpMemScript->GetAsNumber();   // Thời gian hồi sinh (sau khi chết)

			info.Attribute = lpMemScript->GetAsNumber();   // Thuộc tính nguyên tố (ví dụ: Lửa, Nước, Gió...)
			info.ItemRate = lpMemScript->GetAsNumber();    // Tỷ lệ rơi đồ
			info.MoneyRate = lpMemScript->GetAsNumber();   // Tỷ lệ rơi Zen (tiền)

			info.MaxItemLevel = lpMemScript->GetAsNumber(); // Mức cấp độ vật phẩm tối đa có thể rơi
			info.MonsterSkill = lpMemScript->GetAsNumber(); // ID kỹ năng quái sử dụng

			// Kháng nguyên tố (0~3 tương ứng với Fire, Water, Earth, Wind...)
			info.Resistance[0] = lpMemScript->GetAsNumber();
			info.Resistance[1] = lpMemScript->GetAsNumber();
			info.Resistance[2] = lpMemScript->GetAsNumber();
			info.Resistance[3] = lpMemScript->GetAsNumber();


#if(GAMESERVER_UPDATE>=701)

			info.ElementalAttribute = lpMemScript->GetAsNumber();

			info.ElementalPattern = lpMemScript->GetAsNumber();

			info.ElementalDefense = lpMemScript->GetAsNumber();

			info.ElementalDamageMin = lpMemScript->GetAsNumber();

			info.ElementalDamageMax = lpMemScript->GetAsNumber();

			info.ElementalAttackRate = lpMemScript->GetAsNumber();

			info.ElementalDefenseRate = lpMemScript->GetAsNumber();

#endif

			this->SetInfo(info);
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	this->InitMonsterItem();

	delete lpMemScript;
}

void CMonsterManager::SetInfo(MONSTER_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_MONSTER_INFO)
	{
		return;
	}

	info.Life = ((__int64)info.Life * gServerInfo.m_MonsterLifeRate) / 100;

	info.ScriptLife = info.Life;

	this->m_MonsterInfo[info.Index] = info;
}

void CMonsterManager::ExportName(char* filename)
{
	FILE* fp = fopen(filename, "wb");

	if (fp)
	{
		std::vector<Script_Item> ItemMemory;

		char temp[128]; // Buffer temporal para cada campo
		char buffer[512]; // Tamaño suficiente para una línea completa

		for (int i = 0; i < MAX_MONSTER_INFO; i++)
		{
			MONSTER_INFO* item_info = &m_MonsterInfo[i];

			if (item_info->Index != i)
			{
				continue;
			}
			buffer[0] = '\0';

			// Formatear y concatenar cada campo al buffer
			sprintf(temp, "%-5d ", item_info->Index);
			strcat(buffer, temp);

			sprintf(temp, "%-5d ", 1);
			strcat(buffer, temp);

			// Agregar el modelo con comillas
			strcat(buffer, "\"");
			strcat(buffer, item_info->Name);
			strcat(buffer, "\"\n");

			// Escribir el buffer al archivo
			fwrite(buffer, 1, strlen(buffer), fp);
		}

		sprintf_s(buffer, "end");

		fwrite(buffer, 1, strlen(buffer), fp);

		fclose(fp);
	}
}

MONSTER_INFO* CMonsterManager::GetInfo(int index) // OK
{
	if (index >= 0 && index < MAX_MONSTER_INFO)
	{
		if (this->m_MonsterInfo[index].Index == index)
		{
			return &this->m_MonsterInfo[index];
		}
	}

	
	return NULL;
}

char* CMonsterManager::GetMonsterName(int index) // OK
{
	MONSTER_INFO* MonsterInfo = this->GetInfo(index);

	if (MonsterInfo == NULL)
	{
		return "No Name";
	}
	else
	{
		return MonsterInfo->Name;
	}
}

void CMonsterManager::InitMonsterItem() // OK
{
	for (int n = 0; n < MAX_MONSTER_LEVEL; n++)
	{
		this->InsertMonsterItem(n);
	}
}

void CMonsterManager::InsertMonsterItem(int level) // OK
{
	for (int n = 0; n < MAX_ITEM; n++)
	{
		ITEM_INFO ItemInfo;

		if (gItemManager.GetInfo(n, &ItemInfo) == 0)
		{
			continue;
		}

		if (ItemInfo.Level < 0 || ItemInfo.DropItem == 0)
		{
			continue;
		}

		if ((ItemInfo.Level + 4) >= level && (ItemInfo.Level - 2) <= level)
		{
			MONSTER_ITEM_INFO* lpItem = &this->m_MonsterItemInfo[level];

			if (lpItem->IndexCount >= 0 && lpItem->IndexCount < MAX_MONSTER_ITEM)
			{
				lpItem->IndexTable[lpItem->IndexCount++] = n;
			}
		}
	}
}

long CMonsterManager::GetMonsterItem(int level, int excellent, int socket) // OK
{
	if (level < 0 || level >= MAX_MONSTER_LEVEL)
	{
		return -1;
	}

	if (this->m_MonsterItemInfo[level].IndexCount == 0)
	{
		return -1;
	}

	int IndexTable[MAX_MONSTER_ITEM];
	int IndexCount = 0;

	for (int n = 0; n < this->m_MonsterItemInfo[level].IndexCount; n++)
	{
		if (excellent != 0 && this->m_MonsterItemInfo[level].IndexTable[n] >= GET_ITEM(12, 0))
		{
			continue;
		}

		if (excellent != 0 && g380ItemType.Check380ItemType(this->m_MonsterItemInfo[level].IndexTable[n]) != 0)
		{
			continue;
		}

		if ((excellent != 0 || socket == 0) && gSocketItemType.CheckSocketItemType(this->m_MonsterItemInfo[level].IndexTable[n]) != 0)
		{
			continue;
		}

		IndexTable[IndexCount++] = this->m_MonsterItemInfo[level].IndexTable[n];
	}

	return ((IndexCount == 0) ? -1 : IndexTable[GetLargeRand() % IndexCount]);
}

void CMonsterManager::SetMonsterData() // OK
{
	for (int _map = 0; _map < MAX_MAP; _map++)
	{
		if (gMapServerManager.CheckMapServer(_map))
		{
			std::vector<MONSTER_SET_BASE_INFO> base_info = gMonsterSetBase.GetMonsterMap(_map);

			for (int n = 0; n < base_info.size(); n++)
			{
				MONSTER_SET_BASE_INFO* lpInfo = &base_info[n];

				if (lpInfo->Type == 3 || lpInfo->Type == 4)
				{
					continue;
				}

				int index = gObjAddMonster(lpInfo->Map);

				if (OBJECT_RANGE(index) == 0)
				{
					continue;
				}

				if (gObjSetPosMonster(index, lpInfo->index) == 0)
				{
					gObjDel(index);
					continue;
				}

				if (gObjSetMonster(index, lpInfo->MonsterClass) == 0)
				{
					gObjDel(index);
					continue;
				}

#if(GAMESERVER_TYPE==1)
				LPOBJ lpObj = &gObj[index];

				if (lpObj->Map == MAP_CASTLE_SIEGE)
				{
					if (lpObj->Class == 216)
					{
						gCastleSiege.SetCrownIndex(index);
					}
				}

				if (lpObj->Map == MAP_CRYWOLF)
				{
					if (lpObj->Type == OBJECT_MONSTER)
					{
						if (gCrywolf.m_ObjCommonMonster.AddObj(index) == 0)
						{
							gObjDel(index);
							continue;
						}
					}

					if (lpObj->Type == OBJECT_NPC)
					{
						if (lpObj->Class >= 204 && lpObj->Class <= 209)
						{
							if (gCrywolf.m_ObjSpecialNPC.AddObj(index) == 0)
							{
								gObjDel(index);
								continue;
							}
						}
						else
						{
							if (lpObj->Class != 406 && lpObj->Class != 407)
							{
								if (gCrywolf.m_ObjCommonNPC.AddObj(index) == 0)
								{
									gObjDel(index);
									continue;
								}
							}
						}
					}
				}
#endif
			}
		}
	}

	gBloodCastle.Init();

	gBonusManager.Init();

	gCastleDeep.Init();

	gChaosCastle.Init();

	gCustomArena.Init();

	gCustomEventDrop.Init();

	gCustomOnlineLottery.Init();

	gCustomQuiz.Init();

	gTvTEvent.Init();

	gGvGEvent.Init();

	gDevilSquare.Init();

	gIllusionTemple.Init();

	gReiDoMU.Init();

	gInvasionManager.Init();

	gMossMerchant.Init();

#ifdef EVENT_FIND_PATH_NPC_CLIK
	gEventFindPath.Init();
#endif // EVENT_FIND_PATH_NPC_CLIK

	gRaklion.SetState(RAKLION_STATE_END);
}
