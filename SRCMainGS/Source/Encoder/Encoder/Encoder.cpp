#include "stdafx.h"
#include "Util.h"
#include "CCRC32.h"
#include "ThemidaSDK.h"
#include "supportingfeature.h"
#include "ItemToolTip.h"

typedef struct
{
	char CustomerName[32];
	char IpAddress[32];
	WORD IpAddressPort;
	char ClientVersion[8];
	char ClientSerial[17];
	char WindowName[32];
	char ScreenShotPath[50];
}MAIN_INFO_ENV;

typedef struct 
{
#ifdef MAX_INSTANCE_GAME
	DWORD m_MaxInstance;
#endif // MAX_INSTANCE_GAME

	DWORD AntiHackCRC32;
	char AntiHackName[32];
	DWORD ReShaderCRC32;
	char ReShaderName[32];
	UINT HelperActiveAlert;
	UINT HelperActiveLevel;
	UINT MaxAttackSpeed[MAX_CLASS];
	UINT ReconnectTime;
	BYTE RemoveClass;
	BYTE ScreenType;
	BYTE LookAndFeel;
	BYTE ChatEmoticon;
	BYTE PickItemTextType;
	BYTE CastleSkillEnable;
	BYTE CharInfoBalloonType;
	BYTE InventoryExtension;
	BYTE MaxInventoryExtension;
	bool ServerNameDefault;
	bool WareHouseExtension;
	bool CashShopInGame;
	bool MuHelperOficial;
	bool WinMasterSkill;
	bool InvasionInfoTime;
	bool shutdown_tooltipold;
	bool m_RenderItemNameDrop;
	bool m_RenderCharacterName;
	bool Enable_Earring;
	bool Enable_Costumen;
	bool Enable_flag_nat;
	bool shutdown_item_muun;
	bool shutdown_item_eagle;
	//------------------------------------
	BYTE SceneLogin;
	BYTE SceneCharacter;
	BYTE m_RenderMuLogo;
	BYTE RankUserSwitch;
	BYTE PartyHpBarType;
	bool RenderGuildLogo;
	bool RankUserShowOverHead;
	bool RankUserOnlyOnSafeZone;
	bool RankUserShowReset;
	bool RankUserShowMasterReset;
	bool RankUserNeedAltKey;
	bool MenuSwitch;
	BYTE MenuTypeButton;
	bool MenuButtonVipShop;
	bool MenuButtonRankTop;
	bool MenuButtonCommand;
	bool MenuButtonOptions;
	bool MenuButtonEventTime;
	bool WindowsVipShop;
	bool WindowsRankTop;
	bool WindowsCommand;
	bool WindowsEventTime;
	bool WindowsJewelInventory;
	bool WindowsResetInfo;
	BYTE SwitchPersonalShopType;
	bool CustomOffStoreEnable;
	bool CustomStoreEnableJoB;
	bool CustomStoreEnableJoS;
	bool CustomStoreEnableJoC;
	bool CustomStoreEnableCoin1;
	bool CustomStoreEnableCoin2;
	bool CustomStoreEnableCoin3;
	bool remember_account;
	bool trade_system_pro;
	BYTE shutdown_popup;
	BYTE ajust_fps_render;
	BYTE max_item_level_on;
#ifdef SHUTDOWN_LEVEL_WING3
	BYTE shutdown_level_wing3;
#endif // SHUTDOWN_LEVEL_WING3

	//------------------------------------
} MAIN_FILE_INFO;

struct TEXT_FILE_INFO
{
	nInformation m_TRSTooltipData[MaxLineItemToolTip];
	nInformation m_TRSTooltipSetData[MaxLineItemToolTip];
	nText m_TRSTooltipText[MaxLineItemToolTip];
};
void EncoderText()
{
	//===Text Info
	TEXT_FILE_INFO info;
	memset(&info, 0, sizeof(info));

	GInfo.Load("ItemManager\\CustomItemToolTip.txt");
	GInfo.Load2("ItemManager\\CustomTextTooltip.txt");

	memcpy(info.m_TRSTooltipData, GInfo.n_TRSTooltipData, sizeof(info.m_TRSTooltipData));
	memcpy(info.m_TRSTooltipSetData, GInfo.n_TRSTooltipSetData, sizeof(info.m_TRSTooltipSetData));
	memcpy(info.m_TRSTooltipText, GInfo.n_TRSTooltipText, sizeof(info.m_TRSTooltipText));

	for (int n = 0; n < sizeof(TEXT_FILE_INFO); n++)
	{
		((BYTE*)&info)[n] ^= (BYTE)(0xCA ^ LOBYTE(n));
		((BYTE*)&info)[n] += (BYTE)(0x95 ^ HIBYTE(n));
	}


	HANDLE file = CreateFile("..\\Client_2\\Data\\Local\\CBTextInfo.bin", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);

	if (file == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD OutSize = 0;

	if (WriteFile(file, &info, sizeof(TEXT_FILE_INFO), &OutSize, 0) == 0)
	{
		CloseHandle(file);
		return;
	}

	CloseHandle(file);
}
void EncoderEnviron()
{
	MAIN_INFO_ENV info;
	wchar_t wFilename[MAX_PATH];
	wchar_t WindowName[MAX_PATH];

	memset(&info, 0, sizeof(info));

	GetPrivateProfileString("MainInfo", "CustomerName", "", info.CustomerName, sizeof(info.CustomerName), ".\\MainConnect.ini");

	GetPrivateProfileString("MainInfo", "IpAddress", "", info.IpAddress, sizeof(info.IpAddress), ".\\MainConnect.ini");

	info.IpAddressPort = GetPrivateProfileInt("MainInfo", "IpAddressPort", 44405, ".\\MainConnect.ini");

	GetPrivateProfileString("MainInfo", "ClientVersion", "", info.ClientVersion, sizeof(info.ClientVersion), ".\\MainConnect.ini");

	GetPrivateProfileString("MainInfo", "ClientSerial", "", info.ClientSerial, sizeof(info.ClientSerial), ".\\MainConnect.ini");

	GetPrivateProfileString("MainInfo", "ScreenShotPath", "", info.ScreenShotPath, sizeof(info.ScreenShotPath), ".\\MainConnect.ini");

	GetPrivateProfileStringW(L"MainInfo", L"WindowName", L"", WindowName, sizeof(WindowName) / sizeof(wchar_t), L".\\MainConnect.ini");

	GetPrivateProfileStringW(L"MainInfo", L"PathExport", L"", wFilename, sizeof(wFilename) / sizeof(wchar_t), L".\\MainConnect.ini");

	if (info.CustomerName[0] != '\0')
	{
		std::wstring tempPath(wFilename);

		std::wstring tempName(WindowName);

		strncpy_s(info.WindowName, WStringToUTF8(tempName).c_str(), sizeof(info.WindowName));

		if (tempPath.empty())
		{
			CreateDirectory("Release", 0);
			CreateDirectory("Release\\Data", 0);
			CreateDirectory("Release\\Data\\Local", 0);
			PackFileEncrypt("Release\\Data\\Local\\Connect.msil", (BYTE*)&info, 1, sizeof(MAIN_INFO_ENV), 0x5A18u, false);
		}
		else
		{
			RemoveQuotes(tempPath);
			tempPath += L"\\Data\\Local\\Connect.msil";
			PackFileEncrypt(WStringToUTF8(tempPath), (BYTE*)&info, 1, sizeof(MAIN_INFO_ENV), 0x5A18u, false);
		}
	}
}

void SaveMainFileInfo(const MAIN_FILE_INFO& mainInfo, const std::string& filePath)
{
	std::vector<CUSTOM_MAPE_INFO> CustomMap;
	std::vector<CUSTOM_ITEM_INFO> CustomItem;
	std::vector<CUSTOM_JEWEL_INFO> CustomJewelInfo;
	std::vector<CUSTOM_IMAGE_INFO> CustomImagen;
	std::vector<CUSTOM_WING_INFO> CustomWings;
	std::vector<WINGS_PHYSICS_INFO> CustomCapes;
	std::vector<CUSTOM_PET_STACK> CustomPetItem;
	std::vector<CUSTOM_ITEM_STACK> CustomStacks;
	std::vector<CUSTOM_SMOKE_STACK> CustomSmokeItem;
	std::vector<CUSTOM_MONSTER_INFO> CustomMonster;
	std::vector<CUSTOM_NPC_NAME> CustomNpcName;
	std::vector<CUSTOM_EFFECT_STACK> CustomEffectDinamic;
	std::vector<CUSTOM_EFFECT_STACK> CustomEffectStatics;
	std::vector<CUSTOM_EFFECT_STACK> CustomPetEffectDinamic;
	std::vector<CUSTOM_EFFECT_STACK> CustomPetEffectStatics;
	std::vector<RENDER_MESH_EFFECT> CustomPetRenderMesh;//[MAX_CUSTOM_EFFECT];
	std::vector<RENDER_MESH_EFFECT> CustomMonsterRenderMesh;//[MAX_CUSTOM_EFFECT];
	std::vector<TEMPLATE_MEMBERSHIP> CustomBuyVip;
	std::vector<CUSTOM_COMMAND_INFO> CustomCommand;
	std::vector<CHARACTER_NAME_EFFECT> CustomCharNameWave;
	std::vector<RENDER_MESH_EFFECT_DEFAULT> CustomDefaultRenderMesh;

	//------------------------------------------------------------------------------------------------------------
	runtime_unique_document_file("Local\\CustomItem.txt", CustomItem, MAX_CUSTOM_ITEM);

	runtime_unique_document_file("Local\\CustomJewel.txt", CustomJewelInfo, MAX_CUSTOM_STACK);
	

	runtime_unique_document_file("Local\\CustomWorld.txt", CustomMap, MAX_CUSTOM_MAPE);

	runtime_unique_document_file("Local\\CustomImage.txt", CustomImagen, MAX_CUSTOM_IMAGE);

	runtime_unique_document_file("Local\\CustomMonster.txt", CustomMonster, MAX_CUSTOM_MONSTER);

	runtime_unique_document_file("Local\\CustomWing.txt", CustomWings, MAX_CUSTOM_WINGS);

	runtime_unique_document_file("Local\\CustomStack.txt", CustomStacks, MAX_CUSTOM_STACK);

	runtime_unique_document_file("Local\\CustomPet.txt", CustomPetItem, MAX_CUSTOM_PET);

	runtime_unique_document_file("Local\\CustomEffectDynamic.txt", CustomEffectDinamic, MAX_CUSTOM_EFFECT);

	runtime_unique_document_file("Local\\CustomEffectStatic.txt", CustomEffectStatics, MAX_CUSTOM_EFFECT);

	runtime_unique_document_file("Local\\CustomPetEffectDynamic.txt", CustomPetEffectDinamic, MAX_CUSTOM_EFFECT);

	runtime_unique_document_file("Local\\CustomPetEffectStatic.txt", CustomPetEffectStatics, MAX_CUSTOM_EFFECT);

	runtime_unique_document_file("Local\\CustomItemRenderMesh.txt", CustomPetRenderMesh, MAX_CUSTOM_EFFECT, RENDER_MESH_OBJ_ITEM);

	runtime_kapocha_document_file("Local\\CEffectRenderMesh.txt", CustomPetRenderMesh, MAX_CUSTOM_EFFECT);

	runtime_unique_document_file("Local\\CustomMonsterRenderMesh.txt", CustomMonsterRenderMesh, MAX_CUSTOM_EFFECT, RENDER_MESH_OBJ_MONSTER);

	runtime_unique_document_file("Local\\CustomSmokeItem.txt", CustomSmokeItem, MAX_CUSTOM_SMOKE);

	runtime_unique_document_file("Local\\CustomNpcName.txt", CustomNpcName, MAX_CUSTOM_NPC_NAME);

	runtime_unique_document_file("Local\\CustomBuyVip.txt", CustomBuyVip, MAX_MEMBERSHIP_VIP);

	runtime_unique_document_file("Local\\CustomCommand.txt", CustomCommand, MAX_CUSTOM_COMMAND);

	runtime_unique_document_file("Local\\CustomNameWave.txt", CustomCharNameWave, MAX_CHARACTER_NAME);

	runtime_unique_document_file("Local\\CustomRenderEffect.txt", CustomDefaultRenderMesh, MAX_CUSTOM_EFFECT);

	runtime_unique_document_file("Local\\CustomCape.txt", CustomCapes, MAX_CUSTOM_WINGS);
	//------------------------------------------------------------------------------------------------------------

	FILE* fp = fopen(filePath.c_str(), "wb");

	if (fp == nullptr)
	{
		throw std::runtime_error("No se pudo abrir el archivo para lectura.");
	}

	BuxConvert((BYTE*)&mainInfo, sizeof(MAIN_FILE_INFO));

	DWORD dwCheckSum = GenerateCheckSum2((BYTE*)&mainInfo, sizeof(MAIN_FILE_INFO), 0x5A18);

	fwrite(&dwCheckSum, sizeof(DWORD), 1u, fp);
	fwrite(&mainInfo, sizeof(MAIN_FILE_INFO), 1, fp);  // Escribe el tamaño del vector

	// Serializar cada vector
	writeVector(fp, CustomMap);
	writeVector(fp, CustomItem);
	writeVector(fp, CustomJewelInfo);
	writeVector(fp, CustomImagen);
	writeVector(fp, CustomWings);
	writeVector(fp, CustomCapes);
	writeVector(fp, CustomPetItem);
	writeVector(fp, CustomStacks);
	writeVector(fp, CustomSmokeItem);
	writeVector(fp, CustomMonster);
	writeVector(fp, CustomNpcName);
	writeVector(fp, CustomEffectDinamic);
	writeVector(fp, CustomEffectStatics);
	writeVector(fp, CustomPetEffectDinamic);
	writeVector(fp, CustomPetEffectStatics);

	writeVector(fp, CustomPetRenderMesh);
	writeVector(fp, CustomMonsterRenderMesh);

	writeVector(fp, CustomBuyVip);
	writeVector(fp, CustomCommand);
	writeVector(fp, CustomCharNameWave);
	writeVector(fp, CustomDefaultRenderMesh);

	fclose(fp);
}

void EncoderInformation()
{
	MAIN_FILE_INFO info;

	wchar_t wFilename[MAX_PATH];

	memset(&info, 0, sizeof(info));

#ifdef MAX_INSTANCE_GAME
	info.m_MaxInstance = GetPrivateProfileInt("MainInfo", "MaxOfInstance", 2, ".\\MainInfo.ini");
#endif // MAX_INSTANCE_GAME

	GetPrivateProfileString("MainInfo", "PluginAntiHack", "", info.AntiHackName, sizeof(info.AntiHackName), ".\\MainInfo.ini");

	GetPrivateProfileString("MainInfo", "PluginReShader", "", info.ReShaderName, sizeof(info.ReShaderName), ".\\MainInfo.ini");

	GetPrivateProfileStringW(L"MainInfo", L"PathExport", L"", wFilename, sizeof(wFilename) / sizeof(wchar_t), L".\\MainInfo.ini");

	info.HelperActiveAlert = GetPrivateProfileInt("HelperInfo", "HelperActiveAlert", 0, ".\\MainInfo.ini");

	info.HelperActiveLevel = GetPrivateProfileInt("HelperInfo", "HelperActiveLevel", 80, ".\\MainInfo.ini");

	info.ReconnectTime = GetPrivateProfileInt("ReconnectInfo", "ReconnectTime", 0, ".\\MainInfo.ini");

	//[CharacterInfo]
	info.MaxAttackSpeed[0] = GetPrivateProfileInt("CharacterInfo", "DWMaxAttackSpeed", 65000, ".\\MainInfo.ini");

	info.MaxAttackSpeed[1] = GetPrivateProfileInt("CharacterInfo", "DKMaxAttackSpeed", 65000, ".\\MainInfo.ini");

	info.MaxAttackSpeed[2] = GetPrivateProfileInt("CharacterInfo", "FEMaxAttackSpeed", 65000, ".\\MainInfo.ini");

	info.MaxAttackSpeed[3] = GetPrivateProfileInt("CharacterInfo", "MGMaxAttackSpeed", 65000, ".\\MainInfo.ini");

	info.MaxAttackSpeed[4] = GetPrivateProfileInt("CharacterInfo", "DLMaxAttackSpeed", 65000, ".\\MainInfo.ini");

	info.MaxAttackSpeed[5] = GetPrivateProfileInt("CharacterInfo", "SUMaxAttackSpeed", 65000, ".\\MainInfo.ini");

	info.MaxAttackSpeed[6] = GetPrivateProfileInt("CharacterInfo", "RFMaxAttackSpeed", 65000, ".\\MainInfo.ini");

	//[Interface]
	info.ServerNameDefault = GetPrivateProfileInt("Interface", "ServerDefaultName", 1, ".\\MainInfo.ini");

	info.ScreenType = GetPrivateProfileInt("Interface", "ScreenType", 0, ".\\MainInfo.ini");

	info.LookAndFeel = GetPrivateProfileInt("Interface", "LookAndFeel", 0, ".\\MainInfo.ini");

	info.ChatEmoticon = GetPrivateProfileInt("Interface", "ChatEmoticon", 0, ".\\MainInfo.ini");

	info.PickItemTextType = GetPrivateProfileInt("Interface", "PickItemTextType", 0, ".\\MainInfo.ini");

	info.CharInfoBalloonType = GetPrivateProfileInt("Interface", "CharInfo", 0, ".\\MainInfo.ini");

	info.InventoryExtension = GetPrivateProfileInt("Interface", "InventoryExtension", 0, ".\\MainInfo.ini");

	info.MaxInventoryExtension = GetPrivateProfileInt("Interface", "MaxInventoryExtension", 2, ".\\MainInfo.ini");

	info.WareHouseExtension = GetPrivateProfileInt("Interface", "WareHouseExtension", 0, ".\\MainInfo.ini");

	info.CashShopInGame = GetPrivateProfileInt("Interface", "CashShopInGame", 0, ".\\MainInfo.ini");

	info.MuHelperOficial = GetPrivateProfileInt("Interface", "OficialMuHelper", 0, ".\\MainInfo.ini");

	info.WinMasterSkill = GetPrivateProfileInt("Interface", "MasterSkill", 0, ".\\MainInfo.ini");

	info.InvasionInfoTime = GetPrivateProfileInt("Interface", "InvasionInfoTime", 0, ".\\MainInfo.ini");

	info.shutdown_tooltipold = GetPrivateProfileInt("Interface", "RenderItemToolip", 0, ".\\MainInfo.ini");

	info.m_RenderItemNameDrop = GetPrivateProfileInt("Interface", "RenderItemNameDrop", 0, ".\\MainInfo.ini");

	info.m_RenderCharacterName = GetPrivateProfileInt("Interface", "RenderCharacterName", 0, ".\\MainInfo.ini");

	info.Enable_Earring = GetPrivateProfileInt("Interface", "RenderEquipmentEarring", 0, ".\\MainInfo.ini");

	info.Enable_Costumen = GetPrivateProfileInt("Interface", "RenderEquipmentCostume", 0, ".\\MainInfo.ini");

	info.Enable_flag_nat = GetPrivateProfileInt("Interface", "RenderEquipmentFlagNat", 0, ".\\MainInfo.ini");

	info.shutdown_item_muun = GetPrivateProfileInt("Interface", "RenderEquipmentPetMuun", 0, ".\\MainInfo.ini");

	info.shutdown_item_eagle = GetPrivateProfileInt("Interface", "RenderEquipmentPetEagle", 0, ".\\MainInfo.ini");
	//[Custom]
	info.RemoveClass = GetPrivateProfileInt("Custom", "RemoveClass", 0, ".\\MainInfo.ini");

	info.RenderGuildLogo = GetPrivateProfileInt("Custom", "RenderGuildLogo", 0, ".\\MainInfo.ini");

	info.m_RenderMuLogo = GetPrivateProfileInt("Custom", "PrintLogo", 0, ".\\MainInfo.ini");

	info.CastleSkillEnable = GetPrivateProfileInt("Custom", "EnableCsSkillsAllMaps", 0, ".\\MainInfo.ini");

	info.SceneLogin = GetPrivateProfileInt("Custom", "SceneLogin", 0, ".\\MainInfo.ini");

	info.SceneCharacter = GetPrivateProfileInt("Custom", "SceneCharacter", 0, ".\\MainInfo.ini");

	info.ajust_fps_render = GetPrivateProfileInt("Custom", "FpsRenderMax", 30, ".\\MainInfo.ini");

	info.shutdown_popup = GetPrivateProfileInt("Custom", "FpsWindowsOption", 1, ".\\MainInfo.ini");

	//------------------------------------------------------------------------------------------------------------

	info.RankUserSwitch = GetPrivateProfileInt("Custom", "RankUserSwitch", 0, ".\\MainInfo.ini");

	info.RankUserShowOverHead = GetPrivateProfileInt("Custom", "RankUserShowOverHead", 0, ".\\MainInfo.ini");

	info.RankUserOnlyOnSafeZone = GetPrivateProfileInt("Custom", "RankUserOnlyOnSafeZone", 1, ".\\MainInfo.ini");

	info.RankUserShowReset = GetPrivateProfileInt("Custom", "RankUserShowReset", 1, ".\\MainInfo.ini");

	info.RankUserShowMasterReset = GetPrivateProfileInt("Custom", "RankUserShowMasterReset", 1, ".\\MainInfo.ini");

	info.RankUserNeedAltKey = GetPrivateProfileInt("Custom", "RankUserNeedAltKey", 0, ".\\MainInfo.ini");

	info.PartyHpBarType = GetPrivateProfileInt("Custom", "PartyHpBarType", 1, ".\\MainInfo.ini");

	//------------------------------------------------------------------------------------------------------------

	info.MenuSwitch = GetPrivateProfileInt("Custom", "CustomMenuSwitch", 0, ".\\MainInfo.ini");

	info.MenuTypeButton = GetPrivateProfileInt("Custom", "CustomMenuType", 0, ".\\MainInfo.ini");

	info.MenuButtonVipShop = GetPrivateProfileInt("Custom", "EnableVipShopButton", 0, ".\\MainInfo.ini");

	info.MenuButtonRankTop = GetPrivateProfileInt("Custom", "EnableRankingButton", 0, ".\\MainInfo.ini");

	info.MenuButtonCommand = GetPrivateProfileInt("Custom", "EnableCommandButton", 0, ".\\MainInfo.ini");

	info.MenuButtonOptions = GetPrivateProfileInt("Custom", "EnableOptionButton", 0, ".\\MainInfo.ini");

	info.MenuButtonEventTime = GetPrivateProfileInt("Custom", "EnableEventTimeButton", 0, ".\\MainInfo.ini");

	info.WindowsVipShop = GetPrivateProfileInt("Custom", "EnableWindowsVipShop", 0, ".\\MainInfo.ini");

	info.WindowsRankTop = GetPrivateProfileInt("Custom", "EnableWindowsRankTop", 0, ".\\MainInfo.ini");

	info.WindowsCommand = GetPrivateProfileInt("Custom", "EnableWindowsCommand", 0, ".\\MainInfo.ini");

	info.WindowsEventTime = GetPrivateProfileInt("Custom", "EnableWindowsEventTime", 0, ".\\MainInfo.ini");

	info.SwitchPersonalShopType = GetPrivateProfileInt("Custom", "SwitchPersonalShopType", 0, ".\\MainInfo.ini");

	info.CustomOffStoreEnable = GetPrivateProfileInt("Custom", "EnableOffStoreButtom", 0, ".\\MainInfo.ini");

	info.CustomStoreEnableJoB = GetPrivateProfileInt("Custom", "EnableStoreBlessButtom", 0, ".\\MainInfo.ini");

	info.CustomStoreEnableJoS = GetPrivateProfileInt("Custom", "EnableStoreSoulButtom", 0, ".\\MainInfo.ini");

	info.CustomStoreEnableJoC = GetPrivateProfileInt("Custom", "EnableStoreChaosButtom", 0, ".\\MainInfo.ini");

	info.CustomStoreEnableCoin1 = GetPrivateProfileInt("Custom", "EnableStoreCoin1Buttom", 0, ".\\MainInfo.ini");

	info.CustomStoreEnableCoin2 = GetPrivateProfileInt("Custom", "EnableStoreCoin2Buttom", 0, ".\\MainInfo.ini");

	info.CustomStoreEnableCoin3 = GetPrivateProfileInt("Custom", "EnableStoreCoin3Buttom", 0, ".\\MainInfo.ini");

	info.WindowsJewelInventory = GetPrivateProfileInt("Custom", "EnableJewelOfInventory", 0, ".\\MainInfo.ini");

	info.WindowsResetInfo = GetPrivateProfileInt("Custom", "EnableWindowsResetInfo", 0, ".\\MainInfo.ini");

	info.remember_account = GetPrivateProfileInt("Custom", "EnableRememberAccount", 0, ".\\MainInfo.ini");

	info.trade_system_pro = GetPrivateProfileInt("Custom", "EnableTradeCoin", 0, ".\\MainInfo.ini");

	info.max_item_level_on = GetPrivateProfileInt("Custom", "MaxItemLevelOn", 15, ".\\MainInfo.ini");

#ifdef SHUTDOWN_LEVEL_WING3
	info.shutdown_level_wing3 = GetPrivateProfileInt("Custom", "ShutdownLevelWing3", 0, ".\\MainInfo.ini");
#endif // SHUTDOWN_LEVEL_WING3
	//------------------------------------------------------------------------------------------------------------

	CCRC32 CRC32;

	if (CRC32.FileCRC(info.AntiHackName, &info.AntiHackCRC32, 1024) == 0)
	{
		info.AntiHackCRC32 = 0;
	}

	if (CRC32.FileCRC(info.ReShaderName, &info.ReShaderCRC32, 1024) == 0)
	{
		info.ReShaderCRC32 = 0;
	}

	std::wstring tempPath(wFilename);

	RemoveQuotes(tempPath); // Eliminar comillas del string temporal si existen

	if (tempPath.empty())
	{
		CreateDirectory("Release", 0);
		CreateDirectory("Release\\Data", 0);
		SaveMainFileInfo(info, "Release\\Data\\av-code45.pak");
	}
	else
	{
		tempPath += L"\\Data\\av-code45.pak";
		SaveMainFileInfo(info, WStringToUTF8(tempPath));
	}
}

int _tmain(int argc, _TCHAR* argv[]) // OK
{
	CLEAR_START;

	ENCODE_START;

	EncoderEnviron();

	EncoderInformation();

	EncoderText();

	ENCODE_END

	CLEAR_END

	return 0;
}
