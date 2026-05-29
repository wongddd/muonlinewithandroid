// ItemStack.h: interface for the CItemStack class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct ITEM_STACK_INFO
{
	int Index;
	int Level;
	int MaxStack;
	int CreateItemIndex;
	int break_up;
};

class CItemStack
{
public:
	CItemStack();
	virtual ~CItemStack();
	void Load(char* path);
	void ExportXML(std::string filename);
	int GetItemMaxStack(int index, int Level);
	int GetCreateItemIndex(int index, int Level);
	int IsBreakUp(int index, int Level = -1);
private:
	ITEM_STACK_INFO* find(int _index, int _level);
	std::vector<ITEM_STACK_INFO> m_ItemStackInfo;
};

extern CItemStack gItemStack;
