#include "StdAfx.h"
#include "Util.h"
#include "User.h"
#include "CharacterAdvance.h"

CCharacterAdvance::CCharacterAdvance()
{
}

CCharacterAdvance::~CCharacterAdvance()
{
}

CCharacterAdvance* CCharacterAdvance::Instance()
{
	static CCharacterAdvance sInstance;
	return &sInstance;
}

void CCharacterAdvance::UpdateInformation(LPOBJ lpObj)
{
	PMSG_STATS_ADVANCE pMsg;

	pMsg.Header.set(0xF3, 0xF3, sizeof(pMsg));

	pMsg.m_PowerATK = lpObj->PowerATTK;
	pMsg.m_KillCount = lpObj->Kills;
	pMsg.m_DeadCount = lpObj->Deads;
	pMsg.AttackSuccessRate  = lpObj->AttackSuccessRate;
	pMsg.DefenseSuccessRate = lpObj->DefenseSuccessRate;

	pMsg.m_DoubleDmgRate      = lpObj->DoubleDamageRate;
	pMsg.m_TripleDmgRate      = lpObj->TripleDamageRate;
	pMsg.m_CriticalDmgRate    = lpObj->CriticalDamageRate + lpObj->EffectOption.AddCriticalDamageRate;
	pMsg.m_ExcellentDmgRate   = lpObj->ExcellentDamageRate + lpObj->EffectOption.AddExcellentDamageRate;
	pMsg.m_IncIgnoreShield    = lpObj->IgnoreShieldGaugeRate + lpObj->DecreaseShieldGaugeRate;
	pMsg.m_IncIgnoreDefense   = lpObj->IgnoreDefenseRate + lpObj->EffectOption.AddIgnoreDefenseRate;
	pMsg.OffFullHPRestoreRate = lpObj->OffensiveFullHPRestoreRate;
	pMsg.OffFullMPRestoreRate = lpObj->OffensiveFullMPRestoreRate;
	pMsg.OffFullSDRestoreRate = lpObj->OffensiveFullSDRestoreRate;
	pMsg.OffFullBPRestoreRate = lpObj->OffensiveFullBPRestoreRate;
	//--
	pMsg.m_ReflectDmg          = lpObj->DamageReflect + lpObj->EffectOption.AddDamageReflect;
	pMsg.m_ReflectDmgRate      = lpObj->FullDamageReflectRate;
	pMsg.m_ResIgnoreShield     = lpObj->ResistIgnoreShieldGaugeRate;
	pMsg.m_ResIgnoreDefense    = lpObj->ResistIgnoreDefenseRate;
	pMsg.m_ResDoubleDmgRate    = lpObj->ResistDoubleDamageRate;
	pMsg.m_ResAttackStunRate   = lpObj->ResistStunRate;
	pMsg.m_ResCriticalDmgRate  = lpObj->ResistCriticalDamageRate;
	pMsg.m_ResExcellentDmgRate = lpObj->ResistExcellentDamageRate;
	pMsg.DefFullHPRestoreRate = lpObj->DefensiveFullHPRestoreRate;
	pMsg.DefFullMPRestoreRate = lpObj->DefensiveFullMPRestoreRate;
	pMsg.DefFullSDRestoreRate = lpObj->DefensiveFullSDRestoreRate;
	pMsg.DefFullBPRestoreRate = lpObj->DefensiveFullBPRestoreRate;

	pMsg.m_DmgReductionRate = 0;
	for (int n = 0; n < MAX_DAMAGE_REDUCTION; n++)
	{
		pMsg.m_DmgReductionRate += lpObj->DamageReduction[n];
	}

	DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.Header.size);
}
