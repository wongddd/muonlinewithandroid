#pragma once
#if(GAMESERVER_UPDATE>=601)
#define MAX_CLASS			15
#else
#define MAX_CLASS			6
#endif

enum eClassNumber
{
	CLASS_DW  = 0,
	CLASS_DK  = 1,
	CLASS_FE  = 2,
	CLASS_MG  = 3,
	CLASS_DL  = 4,
	CLASS_SU  = 5,
	CLASS_RF  = 6,
	CLASS_GL  = 7,
	CLASS_RW  = 8,
	CLASS_SL  = 9,
	CLASS_GC  = 10,
	CLASS_KUN = 11,
	CLASS_LEM = 12,
	CLASS_KIN = 13,
	CLASS_ALQ = 14,
};

enum eDBClassNumber
{
	DB_CLASS_DW    = 0  ,
	DB_CLASS_SM    = 1  ,
	DB_CLASS_GM    = 2  ,
	DB_CLASS_DW_4  = 3  , //-- Soul Wizard
	DB_CLASS_DK    = 16 ,
	DB_CLASS_BK    = 17 ,
	DB_CLASS_BM    = 18 ,
	DB_CLASS_DK_4  = 19 , //-- Dragon Knight
	DB_CLASS_FE    = 32 , //-- Elf
	DB_CLASS_ME    = 33 ,
	DB_CLASS_HE    = 34 ,
	DB_CLASS_FE_4  = 35 ,
	DB_CLASS_MG    = 48 , //-- Magic Gladiator
	DB_CLASS_DM    = 50 ,
	DB_CLASS_MG_4  = 51 ,
	DB_CLASS_DL    = 64 , //-- DarkLord
	DB_CLASS_LE    = 66 ,
	DB_CLASS_DL_4  = 67 ,
	DB_CLASS_SU    = 80 , //-- Summoner
	DB_CLASS_BS    = 81 ,
	DB_CLASS_DS    = 82 ,
	DB_CLASS_SU_4  = 83 ,
	DB_CLASS_RF    = 96 , //-- Rage Fighter
	DB_CLASS_FM    = 98 ,
	DB_CLASS_RF_4  = 99 ,
	DB_CLASS_GL    = 112, //-- Grow Lancer
	DB_CLASS_ML    = 114,
	DB_CLASS_GL_4  = 115,
	DB_CLASS_RW    = 128, //-- Rune Wizard
	DB_CLASS_RS    = 129,
	DB_CLASS_GR    = 130,
	DB_CLASS_RW_4  = 131,
	DB_CLASS_SL    = 144, //-- Slayer
	DB_CLASS_SL_2  = 145,
	DB_CLASS_SL_3  = 146,
	DB_CLASS_SL_4  = 147,
	DB_CLASS_GC    = 160, //-- Gun Crusher
	DB_CLASS_GC_2  = 161,
	DB_CLASS_GC_3  = 162,
	DB_CLASS_GC_4  = 163,
	DB_CLASS_KUM   = 176, //-- Kundun
	DB_CLASS_KUM_2 = 177,
	DB_CLASS_KUM_3 = 178,
	DB_CLASS_KUM_4 = 179,
	DB_CLASS_LEM   = 192, //-- Lemuria
	DB_CLASS_LEM_2 = 193,
	DB_CLASS_LEM_3 = 194,
	DB_CLASS_LEM_4 = 195,
	DB_CLASS_KIN   = 208, //-- Illusion Knight
	DB_CLASS_KIN_2 = 209,
	DB_CLASS_KIN_3 = 210,
	DB_CLASS_KIN_4 = 211,
	DB_CLASS_ALQ   = 224, //-- Alquimista
	DB_CLASS_ALQ_2 = 225,
	DB_CLASS_ALQ_3 = 226,
	DB_CLASS_ALQ_4 = 227,
};

typedef struct
{
	int Class;
	int Strength;
	int Dexterity;
	int Vitality;
	int Energy;
	int Leadership;
	float MaxLife;
	float MaxMana;
	float LevelLife;
	float LevelMana;
	float VitalityToLife;
	float EnergyToMana;
}DEFAULT_CLASS_INFO;

class CDefaultClassInfo
{
public:
	CDefaultClassInfo();
	virtual ~CDefaultClassInfo();
	void Init();
	void Load(char* path);
	void SetInfo(DEFAULT_CLASS_INFO info);
	int GetCharacterDefaultStat(int Class,int stat);
	BYTE GetCharacterClass(BYTE DBClass);
public:
	DEFAULT_CLASS_INFO m_DefaultClassInfo[MAX_CLASS];
};

extern CDefaultClassInfo gCharacterManager;
