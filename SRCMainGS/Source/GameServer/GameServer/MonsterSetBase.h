// MonsterSetBase.h: interface for the CMonsterSetBase class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

#define MAX_MSB_MONSTER			10000
#define OBJ_MAXMONSTER			(OBJECT_START_USER_AND_BOTS)

struct MONSTER_SET_BASE_INFO
{
	int index;
	int Type;
	int MonsterClass;
	int Map;
	int Dis;
	int X;
	int Y;
	int Dir;
	int TX;
	int TY;
	int Value;
	int Count;
};

class CMonsterSetBase
{
public:
	CMonsterSetBase();
	virtual ~CMonsterSetBase();
	void Load(std::string path, int _map = -1);
	void LoadInfo();
#if GAMESERVER_CLIENTE_UPDATE >= 16
	void ProcessFilesInDirectory(const std::string& directory);
#endif //GAMESERVER_CLIENTE_UPDATE >= 16

	void SetInfo(MONSTER_SET_BASE_INFO info);
	bool GetPosition(int index,short map,short* ox,short* oy);
	bool GetBoxPosition(int map,int x,int y,int tx,int ty,short* ox,short* oy);
	void SetBoxPosition(int index,int map,int x,int y,int tx,int ty);
	MONSTER_SET_BASE_INFO* FindMonsterByIndex(int _index);
	MONSTER_SET_BASE_INFO* GetMonsterMap(int _map, int _index);
	std::vector<MONSTER_SET_BASE_INFO> GetMonsterMap(int _map);
	std::vector<MONSTER_SET_BASE_INFO> GetMonsterMapSorted(int _map);


#ifndef GAMESERVER_EDITH_EXPORT
	void Export(char* filename, int _map);
	void ReadInfo();
#endif // GAMESERVER_EDITH_EXPORT
private:
	int m_count;
	std::map<int, std::vector<MONSTER_SET_BASE_INFO>> m_MonsterSetBaseInfo;
};

extern CMonsterSetBase gMonsterSetBase;