// ItemMove.h: interface for the CItemMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct ITEM_MOVE_INFO
{
	int Index;
	int AllowDrop;
	int AllowSell;
	int AllowTrade;
	int AllowVault;
};

typedef std::map<int, ITEM_MOVE_INFO> type_move_item;

class CItemMove
{
public:
	CItemMove();
	virtual ~CItemMove();
	void Load(char* path);
	void ExportXML(std::string filename);
	bool CheckItemMoveAllowDrop(int index);
	bool CheckItemMoveAllowSell(int index);
	bool CheckItemMoveAllowTrade(int index);
	bool CheckItemMoveAllowVault(int index);
private:
	type_move_item m_ItemMoveInfo;
};

extern CItemMove gItemMove;
