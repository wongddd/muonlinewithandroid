#pragma once
#include "protocol.h"

struct PMSG_STATS_ADVANCE
{
	PSBMSG_HEAD Header;

	DWORD m_PowerATK;
	DWORD m_KillCount;
	DWORD m_DeadCount;
	DWORD m_DoubleDmgRate;
	DWORD m_TripleDmgRate;
	DWORD m_CriticalDmgRate;
	DWORD m_ExcellentDmgRate;
	DWORD m_IncIgnoreShield;
	DWORD m_IncIgnoreDefense;
	DWORD AttackSuccessRate;
	//--
	DWORD m_ReflectDmg;
	DWORD m_ReflectDmgRate;
	DWORD m_ResIgnoreShield;
	DWORD m_ResIgnoreDefense;
	DWORD m_DmgReductionRate;
	DWORD m_ResDoubleDmgRate;
	DWORD m_ResAttackStunRate;
	DWORD m_ResCriticalDmgRate;
	DWORD m_ResExcellentDmgRate;
	DWORD DefenseSuccessRate;


	DWORD DefFullHPRestoreRate;
	DWORD DefFullMPRestoreRate;
	DWORD DefFullSDRestoreRate;
	DWORD DefFullBPRestoreRate;
	DWORD OffFullHPRestoreRate;
	DWORD OffFullMPRestoreRate;
	DWORD OffFullSDRestoreRate;
	DWORD OffFullBPRestoreRate;
};


class CCharacterAdvance
{
public:
	CCharacterAdvance();
	virtual~CCharacterAdvance();
	static CCharacterAdvance* Instance();


	void UpdateInformation(LPOBJ lpObj);
};

#define CharacterAdvance				(CCharacterAdvance::Instance())