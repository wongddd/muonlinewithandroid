// MonsterSetBase.cpp: implementation of the CMonsterSetBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterSetBase.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Util.h"
#include "Path.h"
#include "MapManager.h"
#include "MonsterManager.h"

CMonsterSetBase gMonsterSetBase;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterSetBase::CMonsterSetBase() // OK
{
	this->m_count = 0;
	m_MonsterSetBaseInfo.clear();
}

CMonsterSetBase::~CMonsterSetBase() // OK
{
}

void CMonsterSetBase::Load(std::string path, int _map) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == NULL)
	{
#if GAMESERVER_CLIENTE_UPDATE < 16
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
#endif // GAMESERVER_CLIENTE_UPDATE < 16
		return;
	}

	if (lpMemScript->SetBuffer(path.c_str()) == 0)
	{
#if GAMESERVER_CLIENTE_UPDATE < 16
		ErrorMessageBox(lpMemScript->GetLastError());
#endif // GAMESERVER_CLIENTE_UPDATE < 16
		delete lpMemScript;
		return;
	}


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
				if (strcmp("end", lpMemScript->GetAsString()) == 0)
				{
					break;
				}

				MONSTER_SET_BASE_INFO info;

				memset(&info, 0, sizeof(info));

				info.Type = section;

				info.MonsterClass = lpMemScript->GetNumber();

				info.Map = lpMemScript->GetAsNumber();

				info.Dis = lpMemScript->GetAsNumber();

				info.X = lpMemScript->GetAsNumber();

				info.Y = lpMemScript->GetAsNumber();

				if (section == 1 || section == 3)
				{
					info.TX = lpMemScript->GetAsNumber();
					info.TY = lpMemScript->GetAsNumber();
				}
				else if (section == 2)
				{
#ifdef GAMESERVER_EDITH_EXPORT
					info.X = (info.X - 3) + GetLargeRand() % 7;
					info.Y = (info.Y - 3) + GetLargeRand() % 7;
#endif // !GAMESERVER_EDITH_EXPORT
				}

				info.Dir = lpMemScript->GetAsNumber();

				if (_map != -1)
				{
					info.Map = _map;
				}

				if (section == 1 || section == 3)
				{
					info.Count = lpMemScript->GetAsNumber();

					if (section == 3)
					{
						info.Value = lpMemScript->GetAsNumber();
					}

#ifdef GAMESERVER_EDITH_EXPORT
					for (int n = 0; n < info.Count; n++)
					{
						this->SetInfo(info);
					}
#else
					this->SetInfo(info);
#endif // GAMESERVER_EDITH_EXPORT
				}
				else
				{
					this->SetInfo(info);
				}
			}
		}
	}
	catch (...)
	{
		if (_map == -1)
		{
#if GAMESERVER_CLIENTE_UPDATE < 16
			ErrorMessageBox(lpMemScript->GetLastError());
#endif // GAMESERVER_CLIENTE_UPDATE < 16
		}
	}

	delete lpMemScript;
}

void CMonsterSetBase::LoadInfo()
{
	this->m_count = 0;
	m_MonsterSetBaseInfo.clear();

#if(GAMESERVER_TYPE==0)
	this->Load(gPath.GetFullPath("Monster\\MonsterSetBase.txt"));
#else
	this->Load(gPath.GetFullPath("Monster\\MonsterSetBaseCS.txt"));
#endif

#if GAMESERVER_CLIENTE_UPDATE >= 16
	this->ProcessFilesInDirectory(gPath.GetFullPath("MonsterSetBase"));
#endif // GAMESERVER_CLIENTE_UPDATE >= 16
}

#if GAMESERVER_CLIENTE_UPDATE >= 16
void CMonsterSetBase::ProcessFilesInDirectory(const std::string& directory)
{
	WIN32_FIND_DATA data;

	std::string wildcard_path = directory + "\\*";

	HANDLE file = FindFirstFile(wildcard_path.c_str(), &data);

	if (file != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string file_name = data.cFileName;

			if (file_name == "." || file_name == "..") {
				continue;
			}

			std::string full_path = directory + "\\" + file_name;

			if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				if ((isdigit(data.cFileName[0]) != 0 && isdigit(data.cFileName[1]) != 0 && isdigit(data.cFileName[2]) != 0)
					&& (data.cFileName[3] == ' ' && data.cFileName[4] == '-' && data.cFileName[5] == ' '))
				{
					int _map = atoi(data.cFileName);

					if (gMapServerManager.CheckMapServer(_map))
					{
						this->Load(full_path, _map);
					}
				}
			}
			else
			{
				this->ProcessFilesInDirectory(full_path);
			}
		} while (FindNextFile(file, &data) != 0);

		FindClose(file);
	}
}
#endif // GAMESERVER_CLIENTE_UPDATE >= 16

void CMonsterSetBase::SetInfo(MONSTER_SET_BASE_INFO info) // OK
{
	if (this->m_count < 0 || this->m_count >= MAX_MSB_MONSTER)
	{
		return;
	}

	if (gMapServerManager.CheckMapServer(info.Map) == 0)
	{
		return;
	}

	info.Dir = ((info.Dir == -1) ? (GetLargeRand() % 8) : info.Dir);
	info.index = this->m_count++;
	m_MonsterSetBaseInfo[info.Map].push_back(info);
}

bool CMonsterSetBase::GetPosition(int index, short map, short* ox, short* oy) // OK
{
	if (index < 0 || index >= MAX_MSB_MONSTER)
	{
		return 0;
	}

	MONSTER_SET_BASE_INFO* lpInfo = this->GetMonsterMap(map, index);

	if (lpInfo)
	{
		if (lpInfo->Type == 0 || lpInfo->Type == 4)
		{
			(*ox) = lpInfo->X;
			(*oy) = lpInfo->Y;
			return 1;
		}
		else if (lpInfo->Type == 1 || lpInfo->Type == 3)
		{
			return this->GetBoxPosition(map, lpInfo->X, lpInfo->Y, lpInfo->TX, lpInfo->TY, ox, oy);
		}
		else if (lpInfo->Type == 2)
		{
			return this->GetBoxPosition(map, (lpInfo->X - 3), (lpInfo->Y - 3), (lpInfo->X + 3), (lpInfo->Y + 3), ox, oy);
		}
	}

	return 0;
}

bool CMonsterSetBase::GetBoxPosition(int map, int x, int y, int tx, int ty, short* ox, short* oy) // OK
{
	for (int n = 0; n < 100; n++)
	{
		int subx = tx - x;
		int suby = ty - y;

		subx = ((subx < 1) ? 1 : subx);
		suby = ((suby < 1) ? 1 : suby);

		subx = x + (GetLargeRand() % subx);
		suby = y + (GetLargeRand() % suby);

		if (gMap[map].CheckAttr(subx, suby, 1) == 0 && gMap[map].CheckAttr(subx, suby, 4) == 0 && gMap[map].CheckAttr(subx, suby, 8) == 0)
		{
			(*ox) = subx;
			(*oy) = suby;
			return 1;
		}
	}

	return 0;
}

void CMonsterSetBase::SetBoxPosition(int index, int map, int x, int y, int tx, int ty) // OK
{
	//if (index < 0 || index >= MAX_MSB_MONSTER)
	//{
	//	return;
	//}
	//
	//MONSTER_SET_BASE_INFO* lpInfo = &this->m_MonsterSetBaseInfo[index];
	//
	//lpInfo->Map = map;
	//lpInfo->X = x;
	//lpInfo->Y = y;
	//lpInfo->TX = tx;
	//lpInfo->TY = ty;
}

MONSTER_SET_BASE_INFO* CMonsterSetBase::FindMonsterByIndex(int _index)
{
	for (auto it = m_MonsterSetBaseInfo.begin(); it != m_MonsterSetBaseInfo.end(); ++it)
	{
		std::vector<MONSTER_SET_BASE_INFO>& monsters = it->second;

		auto monsterIt = std::find_if(monsters.begin(), monsters.end(),
			[_index](const MONSTER_SET_BASE_INFO& monster)
			{
				return monster.index == _index;
			});

		if (monsterIt != monsters.end()) {
			return &(*monsterIt); // Retornar el puntero al monstruo encontrado
		}
	}

	return NULL; // No se encontró el monstruo
}

MONSTER_SET_BASE_INFO* CMonsterSetBase::GetMonsterMap(int _map, int _index)
{
	auto it = m_MonsterSetBaseInfo.find(_map);

	if (it == m_MonsterSetBaseInfo.end())
	{
		return NULL; // No se encontró el mapa
	}

	std::vector<MONSTER_SET_BASE_INFO>& monsters = it->second;

	// Usamos std::find_if para encontrar el monstruo con el índice dado
	auto monster_it = std::find_if(monsters.begin(), monsters.end(),
		[_index](const MONSTER_SET_BASE_INFO& monster) {
			return monster.index == _index;
		});

	// Si encontramos el monstruo, devolvemos un puntero a él, si no, nullptr
	return (monster_it != monsters.end()) ? &(*monster_it) : NULL;
}

std::vector<MONSTER_SET_BASE_INFO> CMonsterSetBase::GetMonsterMap(int _map)
{
	static std::vector<MONSTER_SET_BASE_INFO> emptyList; // Lista vacía para evitar nullptr

	auto it = m_MonsterSetBaseInfo.find(_map);

	return (it != m_MonsterSetBaseInfo.end()) ? it->second : emptyList;
}


std::vector<MONSTER_SET_BASE_INFO> CMonsterSetBase::GetMonsterMapSorted(int _map)
{
	std::vector<MONSTER_SET_BASE_INFO> sortedList = this->GetMonsterMap(_map); // Lista vacía para evitar nullptr

	if (sortedList.size() != 0)
	{
		std::sort(sortedList.begin(), sortedList.end(),
			[](const MONSTER_SET_BASE_INFO& a, const MONSTER_SET_BASE_INFO& b) {
				return a.Type < b.Type;
			});
	}

	return sortedList;
}

#ifndef GAMESERVER_EDITH_EXPORT
void CMonsterSetBase::Export(char* filename, int _map)
{
	std::vector<MONSTER_SET_BASE_INFO> baseList = gMonsterSetBase.GetMonsterMapSorted(_map);

	if (baseList.size() != 0)
	{
		FILE* fp = fopen(filename, "wb");

		if (fp)
		{
			char buffer[512]; // Tamaño suficiente para una línea completa

			for (size_t renderType = 0; renderType < 5; renderType++)
			{
				std::string textStr = "";

				sprintf_s(buffer, "%d\n", renderType);
				fwrite(buffer, 1, strlen(buffer), fp);

				if (renderType == 0)
				{
					textStr = "//Monster      MapNumber      Range      PositionX      PositionY      Direction      Comment\n";
				}
				else if (renderType == 1)
				{
					textStr = "//Monster      MapNumber      Range      BeginPosX      BeginPosY      EndPosX      EndPosY      Direction      Quantity      Comment\n";
				}
				else if (renderType == 2)
				{
					textStr = "//Monster      MapNumber      Range      PositionX      PositionY      Direction      Comment\n";
				}
				else if (renderType == 3)
				{
					textStr = "//Monster      MapNumber      Range      BeginPosX      BeginPosY      EndPosX      EndPosY      Direction     Quantity      Value      Comment\n";
				}
				else
				{
					textStr = "//Monster      MapNumber      Range      PositionX      PositionY      Direction      Comment\n";
				}

				fwrite(textStr.data(), 1, textStr.size(), fp);
				textStr.clear();

				for (int n = 0; n < baseList.size(); n++)
				{
					MONSTER_SET_BASE_INFO* Info = &baseList[n];

					if (Info->Type == renderType)
					{

						if (renderType == 0)
						{
							sprintf_s(buffer, "%03d            %02d             %02d         %03d            %03d            %02d             //%s\n", Info->MonsterClass, Info->Map, Info->Dis, Info->X, Info->Y, Info->Dir, gMonsterManager.GetMonsterName(Info->MonsterClass));
						}
						else if (renderType == 1)
						{
							sprintf_s(buffer, "%03d            %02d             %02d         %03d            %03d            %03d          %03d          %d             %02d            //%s\n", Info->MonsterClass, Info->Map, Info->Dis, Info->X, Info->Y, Info->TX, Info->TY, Info->Dir, Info->Count, gMonsterManager.GetMonsterName(Info->MonsterClass));
						}
						else if (renderType == 2)
						{
							sprintf_s(buffer, "%03d            %02d             %02d         %03d            %03d            %d             //%s\n", Info->MonsterClass, Info->Map, Info->Dis, Info->X, Info->Y, Info->Dir, gMonsterManager.GetMonsterName(Info->MonsterClass));
						}
						else if (renderType == 3)
						{
							sprintf_s(buffer, "%03d            %02d             %02d         %03d            %03d            %03d          %03d          %d             %02d            %d          //%s\n", Info->MonsterClass, Info->Map, Info->Dis, Info->X, Info->Y, Info->TX, Info->TY, Info->Dir, Info->Count, Info->Value, gMonsterManager.GetMonsterName(Info->MonsterClass));
						}
						else
						{
							sprintf_s(buffer, "%03d            %02d             %02d         %03d            %03d            %02d             //%s\n", Info->MonsterClass, Info->Map, Info->Dis, Info->X, Info->Y, Info->Dir, gMonsterManager.GetMonsterName(Info->MonsterClass));
						}
						fwrite(buffer, 1, strlen(buffer), fp);
					}
				}

				textStr = "end\n\n\n";
				fwrite(textStr.data(), 1, textStr.size(), fp);
				textStr.clear();
			}
			fclose(fp);
		}
	}
}


void CMonsterSetBase::ReadInfo()
{
	for (int _map = 0; _map < MAX_MAP; _map++)
	{
		if (gMapServerManager.CheckMapServer(_map))
		{
			char wildcard_path[MAX_PATH];

			sprintf_s(wildcard_path, "MonsterSetBase\\%03d - %s.txt", _map, gMapManager.GetMapName(_map));

			this->Export(gPath.GetFullPath(wildcard_path), _map);
		}
	}
}
#endif // !GAMESERVER_EDITH_EXPORT