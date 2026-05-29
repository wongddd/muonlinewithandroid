// Android stub for CGMProtect — the original CGMProtect.cpp is heavily Win32-specific
// (file CRC checking, mutex-based single-instance, anti-debug, VM detection, etc.)
// and depends on pre-compiled CCRC32.obj which has no source.
// This provides safe no-op implementations for Android.

#include "stdafx.h"
#include "CGMProtect.h"

// Stub CCRC32 class (original is a pre-compiled .obj with no source)
class CCRC32 {
public:
    void FileCRC(const char*, DWORD*, DWORD) {}
    void FullCRC(const char*, DWORD*) {}
};

CGMProtect::CGMProtect()
{
    m_hMutex = NULL;
    memset(&m_Kernel, 0, sizeof(m_Kernel));
    memset(&m_MainInfo, 0, sizeof(m_MainInfo));
    m_MainInfo.ajust_fps_render = 25;  // default FPS, avoids SIGFPE in Getframe_per_second()
}

CGMProtect::~CGMProtect()
{
}

CGMProtect* CGMProtect::Instance()
{
    static CGMProtect sInstance;
    return &sInstance;
}

WORD CGMProtect::sin_port()
{
    return m_Kernel.sin_port;
}

WORD CGMProtect::GetMaxOfInstance()
{
    return m_MainInfo.m_MaxInstance;
}

char* CGMProtect::GetWindowName()
{
    return m_Kernel.WindowName;
}

char* CGMProtect::GetScreenPath()
{
    return m_Kernel.ScreenShotPath;
}

bool CGMProtect::CreateKeyEnv()
{
	WORD Key = 0;
	for (int n = 0; n < sizeof(m_Kernel.CustomerName); n++)
	{
		Key += (BYTE)(m_Kernel.CustomerName[n] ^ m_Kernel.ClientSerial[(n % sizeof(m_Kernel.ClientSerial))]);
	}

	this->EncDecKey[0] = (BYTE)0xE2;
	this->EncDecKey[1] = (BYTE)0x02;

	this->EncDecKey[0] += LOBYTE(Key);
	this->EncDecKey[1] += HIBYTE(Key);

	return true;
}

void CGMProtect::HookComplete()
{
    extern char* szServerIpAddress;
    extern WORD g_ServerPort;
    extern BYTE Version[SIZE_PROTOCOLVERSION];
    extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

    szServerIpAddress = new char[32];
    strncpy(szServerIpAddress, m_Kernel.IpAddress, 31);
    szServerIpAddress[31] = '\0';

    g_ServerPort = m_Kernel.sin_port;

    Version[0] = (m_Kernel.ClientVersion[0] + 1);
    Version[1] = (m_Kernel.ClientVersion[2] + 2);
    Version[2] = (m_Kernel.ClientVersion[3] + 3);
    Version[3] = (m_Kernel.ClientVersion[5] + 4);
    Version[4] = (m_Kernel.ClientVersion[6] + 5);

    memcpy(Serial, m_Kernel.ClientSerial, sizeof(Serial));
}

bool CGMProtect::ReadMainFile()
{
	// Read Connect.msil to get CustomerName/ClientSerial for EncDecKey calculation
	if (!ReadMainConnect("Data\\Local\\Connect.msil", &m_Kernel, sizeof(MAIN_INFO_ENV)))
	{
		// If file not found, still proceed (EncDecKey defaults to [0xE2, 0x02])
		memset(&m_Kernel, 0, sizeof(m_Kernel));
	}
	CreateKeyEnv();
	return true;
}

bool CGMProtect::ReadMainConnect(std::string filename, void* pEnvStruct, int Size)
{
	BYTE* Buffer = nullptr;
	int MaxLine = PackFileDecrypt(filename, &Buffer, 1, Size, 0x5A18);

	if (MaxLine != 0 && Buffer)
	{
		memcpy(pEnvStruct, Buffer, Size);
		delete[] Buffer;
		return true;
	}
	return false;
}
bool CGMProtect::LoadMainFileInfo(MAIN_FILE_INFO&, const std::string&) { return true; }
bool CGMProtect::ReadTextFile(char*) { return true; }
void CGMProtect::CheckPluginFile() {}
bool CGMProtect::IsInvExt() { return false; }
bool CGMProtect::IsBaulExt() { return false; }
bool CGMProtect::shutdown_oficial_helper() { return false; }
bool CGMProtect::IsInGameShop() { return false; }
bool CGMProtect::IsMasterSkill() { return false; }
bool CGMProtect::CheckWideScreen() { return false; }
bool CGMProtect::ChatEmoticon() { return false; }
bool CGMProtect::PickItemTextType() { return false; }
bool CGMProtect::CastleSkillEnable() { return false; }
int CGMProtect::CharInfoBalloonType() { return 0; }
int CGMProtect::LookAndFeelType() { return 0; }
int CGMProtect::GetAttackTimeForClass(BYTE) { return 0; }
DWORD CGMProtect::GetAttackSpeedForClass(BYTE) { return 0; }
void CGMProtect::LoadItemModel() {}
void CGMProtect::LoadPetsModel() {}
void CGMProtect::LoadClawModel(int, int, int) {}
void CGMProtect::LoadImageTexture() {}
bool CGMProtect::GetItemColor(int, float*) { return false; }
bool CGMProtect::IsPetBug(int) { return false; }
int CGMProtect::GetPetMovement(int) { return 0; }
bool CGMProtect::CheckItemStack(int, int) { return false; }
CUSTOM_PET_STACK* CGMProtect::GetPetAction(int) { return nullptr; }
CUSTOM_ITEM_STACK* CGMProtect::GetItemStack(int, int) { return nullptr; }
CUSTOM_SMOKE_STACK* CGMProtect::GetItemSmoke(int) { return nullptr; }
bool CGMProtect::GetNpcName(int, char*, int, int) { return false; }
bool CGMProtect::IsRenderWave(const char*) { return false; }
void CGMProtect::LoadingProgressive() {}
void CGMProtect::runtime_checked_crc32() {}
void CGMProtect::runtime_open_module_crc32(std::string, DWORD) {}
bool CGMProtect::runtime_create_mutex() { return true; }
void CGMProtect::runtime_delete_mutex() {}
DWORD CGMProtect::GetProcessIDFromMutex(const std::string&) { return 0; }
void CGMProtect::Release() {}
bool CGMProtect::IsWindows7OrLower() { return false; }
bool CGMProtect::IsWindows11() { return false; }
bool CGMProtect::IsRunningInVM() { return false; }

// ============================================================================
// Bridge: initialize EncDecKey from Connect.msil and push to MuNetwork
// Called from MuGameLoop.cpp (which uses stdafx_port.h, can't include CGMProtect.h)
// ============================================================================
#ifdef ANDROID
#include "MuNetwork.h"
#include <android/log.h>
#define LOG_TAG "CGMProtect"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" void CGMProtect_InitEncDecKey() {
	CGMProtect* p = CGMProtect::Instance();
	if (p->ReadMainFile()) {
		LOGI("Connect.msil loaded, EncDecKey=[0x%02X, 0x%02X]", p->EncDecKey[0], p->EncDecKey[1]);
	} else {
		LOGE("Connect.msil not found, using default EncDecKey=[0x%02X, 0x%02X]", p->EncDecKey[0], p->EncDecKey[1]);
	}
    // Override EncDecKey to match server: CustomerName="MuEmuS6", ServerSerial="TbYehR2hFUPBKgZj"
    // Key = sum(CustomerName[n] ^ ServerSerial[n%17]) for n=0..31 = 0x0882
    // EncDecKey[0] = 0xE2 + 0x82 = 0x64, EncDecKey[1] = 0x02 + 0x08 = 0x0A
    p->EncDecKey[0] = 0x64;
    p->EncDecKey[1] = 0x0A;
    MuNetwork::initKeys(p->EncDecKey[0], p->EncDecKey[1]);
}
#endif // ANDROID
