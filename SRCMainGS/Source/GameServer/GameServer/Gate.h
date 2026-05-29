// Gate.h: interface for the CGate class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"
#define MAX_GATES			1024

struct GATE_INFO
{
	int Index;
	int Flag;
	int Map;
	int X;
	int Y;
	int TX;
	int TY;
	int TargetGate;
	int Dir;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int AccountLevel;
};

typedef struct GATEATTRIBUTE
{
	BYTE Flag;
	BYTE iMap;
	BYTE X;
	BYTE Y;
	BYTE TX;
	BYTE TY;
	WORD Target;
	BYTE Angle;
	WORD MINLevel;
	WORD MAXLevel;

	GATEATTRIBUTE() : X(0), Y(0), TX(0), TY(0), MINLevel(0), MAXLevel(0)
	{
		Flag = 0;
		iMap = 0;
		Angle = 0;
		Target = 0;
	}
} GATE_ATTRIBUTE;

typedef std::map<int, GATE_INFO> type_map_gate;


class CGate
{
public:
	CGate();
	virtual ~CGate();
	void Load(char* path);
	void ExportXML(std::string filename);
	void ExportBMD(std::string filename);

	bool GetInfo(int index,GATE_INFO* lpInfo);
	int GetGateMap(int index);
	int GetMoveLevel(LPOBJ lpObj,int map,int level);
	bool IsGate(int index);
	bool IsInGate(LPOBJ lpObj,int index);
	bool GetGate(int index,int* gate,int* map,int* x,int* y,int* dir,int* level);
private:
	type_map_gate m_GateInfo;
};

extern CGate gGate;
