// ============================================================================
// NewUIStubs.cpp — Stubs for NewUI classes + Lua + audio + globals
// ============================================================================

#include "stdafx_port.h"
#include "_struct.h"
#include "WSclient.h"

namespace leaf { class xstreambuf; }

struct OBJECT;
struct CHARACTER;
struct tagITEM;

// g_fScreenRate_x/y — defined in UIControls.cpp, used by SEASON3B NewUI code
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

namespace SEASON3B {

// Forward declarations — real implementations are in NewUI*.cpp (M1-M6)
enum eLUCKYITEM { eLuckyItem_None = 0, eLuckyItem_Move, eLuckyItem_Act, eLuckyITem_Result, eLuckyItem_End };
enum BUTTON_STATE : int;

class CNewUISystem;
class CNewUIOptionWindow;
class CNewUICamWebzen;
class CNewUIMiniMap;
class CNewUIInventoryCtrl;
class CNewUIInventoryCore;
class CNewUICursedTempleSystem;
class CNewUIManager;
class CNewUIChatLogWindow;
class CNewUIMainFrameWindow;
class CNewUIFriendWindow;
class CNewUISlideWindow;
class CNewUICatapultWindow;
class CNewUIReconnect;
class CNewUIMyInventory;
class CNewUICommandWindow;
class CNewUIQuickCommandWindow;
class CNewUINPCShop;
class CNewUISkillList;
class CNewUIKanturu2ndEnterNpc;
class CNewUIChatInputBox;
class CNewUIPartyListWindow;
class CNewUIInGameShop;
class CNewUIInvenExpansion;
class CNewUINameWindow;
class CNewUIItemTooltip;
class CNewUIItemScript;
class CNewUILuckyCoinRegistration;
class CNewUICursedTempleEnter;
class CNewUIMyShopInventory;
class CNewUIMasterSkillTree;
class CNewUICommandFilter;
class CNewUIGensRanking;
class CNewUIMixInventory;
class CNewUIStorageInventory;
class CNewUITrade;
class CNewUIPurchaseShopInventory;
class CNewUIMacroGaugeBar;
class CNewUIBloodCastle;
class CNewUIChaosCastleTime;
class CNewUIItemMng;
class CNewUI3DRenderMng;
class CNewUIQuestProgressByEtc;
class CNewUIQuestProgress;
class CNewUIMyQuestInfoWindow;
class CNewUICryWolf;
class CNewUIDoppelGangerFrame;
class CNewUIGuardWindow;
class CNewUIStorageExpansion;
class CNewUIHotKey;
class CNewUISiegeWarfare;
class CNewUIGuildInfoWindow;
class CNewUIEnterBloodCastle;
class CNewUIDevilSquareEnter;
class CNewUICharacterInfoWindow;
class CNewUICursedTempleResult;
class CNewUIKanturuInfoWindow;
class CNewUIMoveCommandWindow;
class CNewUINPCDialogue;
class CNewUIHelpWindow;
class CNewUIItemEnduranceInfo;
class CNewUIMixExpansion;
class CNewUIRankingTop;
class CNewUIDoppelGangerWindow;
class CNewUIEmpireGuardianTimer;
class CNewUIExchangeLuckyCoin;
class CNewUIInventoryJewel;
class CNewUILuckyItemWnd;
class CNewUIPartyInfoWindow;

// ==================== Stubs for items NOT in any NewUI*.cpp file ====================

int SelectedCharacter = -1;

// CNewUIInGameShop — not in any NewUI*.cpp, class defined in InGameShop.h/GameShopSystem.cpp
class CNewUIInGameShop {
public:
    CNewUIInGameShop();
    void Create(CNewUIManager*, int, int);
    bool IsInGameShop();
    bool IsInGameShopOpen();
    bool IsInGameShopRect(float, float);
    void OpeningProcess();
    void ClosingProcess();
    void SetConvertInvenCoord(unsigned short, float, float);
    void SetRateScale(int);
    int GetCurrentStorageCode();
    void InitStorage(int, int, int, int);
    void AddStorageItem(int, int, int, int, int, int, char, char*, char*);
    void UpdateStorageItemList();
    void InitBanner(char*, char*);
};
CNewUIInGameShop::CNewUIInGameShop() {}
void CNewUIInGameShop::Create(CNewUIManager*, int, int) {}
bool CNewUIInGameShop::IsInGameShop() { return false; }
bool CNewUIInGameShop::IsInGameShopOpen() { return false; }
bool CNewUIInGameShop::IsInGameShopRect(float, float) { return false; }
void CNewUIInGameShop::OpeningProcess() {}
void CNewUIInGameShop::ClosingProcess() {}
void CNewUIInGameShop::SetConvertInvenCoord(unsigned short, float, float) {}
void CNewUIInGameShop::SetRateScale(int) {}
int CNewUIInGameShop::GetCurrentStorageCode() { return 0; }
void CNewUIInGameShop::InitStorage(int, int, int, int) {}
void CNewUIInGameShop::AddStorageItem(int, int, int, int, int, int, char, char*, char*) {}
void CNewUIInGameShop::UpdateStorageItemList() {}
void CNewUIInGameShop::InitBanner(char*, char*) {}

// Globals not defined in any M6 source file
void* g_pUIJewelHarmonyinfo = nullptr;
int g_iLengthAuthorityCode = 0;
int g_iCancelSkillTarget = 0;

// Message box classes — some are in M6 files, keep only those not in M6
class CNewUIMessageBoxBase { public: CNewUIMessageBoxBase(); virtual ~CNewUIMessageBoxBase(); };
class CNewUIMessageBoxMng { public: CNewUIMessageBoxMng(); ~CNewUIMessageBoxMng(); static CNewUIMessageBoxMng* GetInstance(); bool IsEmpty(); void PopAllMessageBoxes(); };

class CNewUIManager { public: static void ResetActiveUIObj(); void ShowInterface(unsigned int, bool); void EnableInterface(unsigned int, bool); };
class CNewKeyInput { public: static CNewKeyInput* GetInstance(); void ScanAsyncKeyState(); };

} // namespace SEASON3B

// ==================== SEASON4A namespace stubs ====================
namespace SEASON4A {
float g_iLimitAttackTime = 0.0f;
float IntensityTransform = 0.0f;
} // namespace SEASON4A

// ==================== CWin (global scope, not SEASON3B) ====================
class CButton {
public:
    virtual ~CButton() {}
};
enum CHANGE_PRAM : int { X = 1, Y, XY };

class CWin {
public:
    CWin();
    virtual ~CWin();
    void Create(int, int, int, bool);
    void RegisterButton(CButton*);
    void Show(bool);
    int CursorInWin(int);
    void RenderButtons();
    void Release();
    void Render();
    void SetPosition(int, int);
    void SetSize(int, int, int); // overload: all int
    void SetSize(int, int, CHANGE_PRAM); // overload: CHANGE_PRAM
    void ActiveBtns(bool);
    void Update(double);
};
CWin::CWin() {}
CWin::~CWin() {}
void CWin::Create(int, int, int, bool) {}
void CWin::RegisterButton(CButton*) {}
void CWin::Show(bool) {}
int CWin::CursorInWin(int) { return 0; }
void CWin::RenderButtons() {}
void CWin::Release() {}
void CWin::Render() {}
void CWin::SetPosition(int, int) {}
void CWin::SetSize(int, int, int) {}
void CWin::SetSize(int, int, CHANGE_PRAM) {}
void CWin::ActiveBtns(bool) {}
void CWin::Update(double) {}

// SImgInfo forward decl for CWinEx
struct SImgInfo;

class CWinEx {
public:
    CWinEx();
    virtual ~CWinEx();
    void Create(SImgInfo*, int, int);
    void SetLine(int);
    void Release();
    void SetPosition(int, int);
    void Show(bool);
    void Render();
    int CursorInWin(int);
    bool CheckAdditionalState();
};
CWinEx::CWinEx() {}
CWinEx::~CWinEx() {}
void CWinEx::Create(SImgInfo*, int, int) {}
void CWinEx::SetLine(int) {}
void CWinEx::Release() {}
void CWinEx::SetPosition(int, int) {}
void CWinEx::Show(bool) {}
void CWinEx::Render() {}
int CWinEx::CursorInWin(int) { return 0; }
bool CWinEx::CheckAdditionalState() { return false; }

// g_hFixFont global
void* g_hFixFont = nullptr;

// ==================== Audio ====================
void PlayBuffer(int, OBJECT*, int) {}
void LoadWaveFile(int, char*, int, bool) {}
// ==================== CGMFerea ====================
// Stub removed: real implementation compiled from GMFerea.cpp (P2-6)

// ==================== TEXCOORD ====================
void TEXCOORD(float* c, float u, float v) {
    c[0] = u;
    c[1] = v;
}

// ==================== Font globals ====================
void* g_hFontBold = nullptr;
void* g_hFont = nullptr;

// ==================== Winmain.cpp globals (excluded from build) ====================
int RandomTable[100] = {};
char m_ID[6][11] = {};
char m_ExeVersion[11] = {};

// ==================== Additional globals ====================
void* g_hFontBig = nullptr;
void* g_hDC = nullptr;

// ==================== ReleaseBuffer (DSPlaySound) ====================
void ReleaseBuffer(int) {}

int g_strSelectedML = 0;

// ==================== CUITextInputBox pointers ====================
class CUITextInputBox;
CUITextInputBox* g_pSinglePasswdInputBox = nullptr;
CUITextInputBox* g_pSingleTextInputBox = nullptr;
// g_pUIManager, g_pUIMapName, g_pTimer moved to MuGlobalState.cpp (single definition)

// ==================== Audio stubs (DSPlaySound excluded) ====================
void StopBuffer(int, BOOL) {}
void SetMasterVolume(long) {}
void Set3DSoundPosition() {}

// ==================== TERRAIN helpers (needs non-inline definition) ====================
int TERRAIN_INDEX_REPEAT(int x, int y) { return (y & 0xFF) * 256 + (x & 0xFF); }
int TERRAIN_INDEX(int x, int y) { return y * 256 + x; }
unsigned short TERRAIN_ATTRIBUTE(float, float) { return 0; }

// ==================== CInput stub (BlackWin.cpp, excluded from build) ====================
class CInput {
public:
    virtual ~CInput();
    static CInput& Instance();
    bool IsKeyDown(int);
    int GetScreenWidth();
    int GetScreenHeight();
    void Update();
    bool Create(HWND, long, long);
};
CInput::~CInput() {}
CInput& CInput::Instance() { static CInput inst; return inst; }
bool CInput::IsKeyDown(int) { return false; }
int CInput::GetScreenWidth() { return 1920; }
int CInput::GetScreenHeight() { return 1080; }
void CInput::Update() {}
bool CInput::Create(HWND, long, long) { return true; }

// ==================== ConnectVersionHex stub (ConnectVersionHex.cpp, excluded from build) ====================
class ConnectVersionHex {
public:
    static ConnectVersionHex* Instance();
    void Initialize();
    int GetClientVersion();
    bool GetUpdateVersion();
    void SetUpdateVersion(bool);
    void RenderInterface(void*);
    void data_end();
    void SetClientVersion(int);
    void write_received_file();
    void execute_launcher();
    void download_information(unsigned short, unsigned int, unsigned int, float);
    void write_data_current(unsigned short, unsigned int, unsigned int, unsigned int, unsigned char*);
};
ConnectVersionHex* ConnectVersionHex::Instance() { static ConnectVersionHex inst; return &inst; }
void ConnectVersionHex::Initialize() {}
int ConnectVersionHex::GetClientVersion() { return 0; }
bool ConnectVersionHex::GetUpdateVersion() { return false; }
void ConnectVersionHex::SetUpdateVersion(bool) {}
void ConnectVersionHex::RenderInterface(void*) {}
void ConnectVersionHex::data_end() {}
void ConnectVersionHex::SetClientVersion(int) {}
void ConnectVersionHex::write_received_file() {}
void ConnectVersionHex::execute_launcher() {}
void ConnectVersionHex::download_information(unsigned short, unsigned int, unsigned int, float) {}
void ConnectVersionHex::write_data_current(unsigned short, unsigned int, unsigned int, unsigned int, unsigned char*) {}

// ==================== Lua C API stubs ====================
extern "C" {

typedef struct lua_State lua_State;

void* luaL_newstate(void) { return nullptr; }
void  lua_close(void*) {}
int   lua_gettop(void*) { return 0; }

// Push/pop/manipulate
void  lua_pushvalue(void*, int) {}
void  lua_settop(void*, int) {}
void  lua_pushnil(void*) {}
void  lua_pushboolean(void*, int) {}
void  lua_pushinteger(void*, int) {}
void  lua_pushnumber(void*, double) {}
const char* lua_pushlstring(void*, const char*, size_t) { return ""; }
const char* lua_pushstring(void*, const char*) { return ""; }
void  lua_pushlightuserdata(void*, void*) {}
void  lua_pushcclosure(void*, void*, int) {}
void* lua_newuserdatauv(void*, size_t, int) { return nullptr; }
void  lua_createtable(void*, int, int) {}
void  lua_setglobal(void*, const char*) {}
void  lua_setfield(void*, int, const char*) {}
void  lua_seti(void*, int, int) {}
void  lua_settable(void*, int) {}
void  lua_setmetatable(void*, int) {}
void  lua_rawset(void*, int) {}
void  lua_rawgeti(void*, int, int) {}
void  lua_rotate(void*, int, int) {}
void  lua_xmove(void*, void*, int) {}

// Get/query
int   lua_getglobal(void*, const char*) { return 0; }
int   lua_getfield(void*, int, const char*) { return 0; }
int   lua_getmetatable(void*, int) { return 0; }
int   lua_geti(void*, int, int) { return 0; }
int   lua_type(void*, int) { return 0; }
int   lua_isinteger(void*, int) { return 0; }
int   lua_rawequal(void*, int, int) { return 0; }
int   lua_rawget(void*, int) { return 0; }
int   lua_toboolean(void*, int) { return 0; }
const char* lua_tolstring(void*, int, size_t*) { return ""; }
int   lua_tointegerx(void*, int, int*) { return 0; }
double lua_tonumberx(void*, int, int*) { return 0.0; }
void* lua_touserdata(void*, int) { return nullptr; }
const void* lua_topointer(void*, int) { return nullptr; }
int   lua_next(void*, int) { return 0; }
int   lua_compare(void*, int, int, int) { return 0; }
int   lua_absindex(void*, int) { return 0; }

// Call/error
void  lua_callk(void*, int, int, int, void*) {}
int   lua_pcallk(void*, int, int, int, int, void*) { return 0; }
int   lua_error(void*) { return 0; }
int   luaL_error(void*, const char*, ...) { return 0; }

// Load/require
int   luaL_loadbufferx(void*, const char*, size_t, const char*, const char*) { return 0; }
void  luaL_requiref(void*, const char*, void*, int) {}

// Ref/unref
int   luaL_ref(void*, int) { return 0; }
void  luaL_unref(void*, int, int) {}

// Metatables
int   luaL_newmetatable(void*, const char*) { return 0; }

// Setfuncs/traceback
void  luaL_setfuncs(void*, const void*, int) {}
void  luaL_traceback(void*, void*, const char*, int) {}

// Panic
void* lua_atpanic(void*, void*) { return nullptr; }

// Open libs
void  luaL_openlibs(void*) {}
void  luaopen_base(void*) {}
void  luaopen_package(void*) {}
void  luaopen_coroutine(void*) {}
void  luaopen_string(void*) {}
void  luaopen_os(void*) {}
void  luaopen_math(void*) {}
void  luaopen_debug(void*) {}
void  luaopen_io(void*) {}
void  luaopen_table(void*) {}
void  luaopen_utf8(void*) {}

} // extern "C"

// Account globals
char m_Psz[6][11] = {};
int m_SaveAccount = 0;

// ==================== Global-scope NewUI classes ====================
// CNewUIItemTooltip — implemented by real NewUIItemTooltip.cpp (M6)

// ============================================================================
// BATCH 2: Comprehensive stubs for all remaining undefined symbols (279 total)
// ============================================================================

// ==================== Global scope: CSimpleModulus (Win original) ====================
// Class declared in Main5.2/source/SimpleModulus.h — define out-of-line methods only
CSimpleModulus::CSimpleModulus() {}
CSimpleModulus::~CSimpleModulus() {}
int CSimpleModulus::Encrypt(void*, void*, int) { return 0; }
int CSimpleModulus::Decrypt(void*, void*, int) { return 0; }

// ==================== Global scope: CInGameShopSystem ====================
class CInGameShopSystem {
public:
    static CInGameShopSystem* GetInstance();
    void InitEventPackage(int);
    void InsertEventPackage(int*);
    bool IsShopOpen();
    void SetBannerVersion(int, int, int);
    void SetCashCreditCard(double);
    void SetCashPrepaid(double);
    void SetIsRequestShopOpenning(bool);
    void SetScriptVersion(int, int, int);
    void SetTotalCash(double);
    void SetTotalMileage(double);
    void SetTotalPoint(double);
    void ShopOpenUnLock();
    char* GetBannerFileName();
    char* GetBannerURL();
    bool GetIsRequestShopOpenning();
    bool IsScriptDownload();
    bool ScriptDownload();
    bool IsBannerDownload();
    bool BannerDownload();
};
CInGameShopSystem* CInGameShopSystem::GetInstance() { static CInGameShopSystem inst; return &inst; }
void CInGameShopSystem::InitEventPackage(int) {}
void CInGameShopSystem::InsertEventPackage(int*) {}
bool CInGameShopSystem::IsShopOpen() { return false; }
void CInGameShopSystem::SetBannerVersion(int, int, int) {}
void CInGameShopSystem::SetCashCreditCard(double) {}
void CInGameShopSystem::SetCashPrepaid(double) {}
void CInGameShopSystem::SetIsRequestShopOpenning(bool) {}
void CInGameShopSystem::SetScriptVersion(int, int, int) {}
void CInGameShopSystem::SetTotalCash(double) {}
void CInGameShopSystem::SetTotalMileage(double) {}
void CInGameShopSystem::SetTotalPoint(double) {}
void CInGameShopSystem::ShopOpenUnLock() {}
char* CInGameShopSystem::GetBannerFileName() { return nullptr; }
char* CInGameShopSystem::GetBannerURL() { return nullptr; }
bool CInGameShopSystem::GetIsRequestShopOpenning() { return false; }
bool CInGameShopSystem::IsScriptDownload() { return false; }
bool CInGameShopSystem::ScriptDownload() { return false; }
bool CInGameShopSystem::IsBannerDownload() { return false; }
bool CInGameShopSystem::BannerDownload() { return false; }

// ==================== Global scope: CMsgBoxIGSCommon ====================
class CMsgBoxIGSCommonLayout {
public:
    virtual ~CMsgBoxIGSCommonLayout();
};
CMsgBoxIGSCommonLayout::~CMsgBoxIGSCommonLayout() {}

class CMsgBoxIGSCommon {
public:
    CMsgBoxIGSCommon();
    void Initialize(char const*, char const*);
};
CMsgBoxIGSCommon::CMsgBoxIGSCommon() {}
void CMsgBoxIGSCommon::Initialize(char const*, char const*) {}

// ==================== Global scope: Free functions ====================
void AllStopSound() {}
void CheckHack() {}
unsigned short GetCheckSum(unsigned short v) { return v; }
bool g_bEnterPressed = false;
int g_iChatInputType = 0;

// ============================================================================
// SEASON3B namespace — new classes and methods
// ============================================================================
namespace SEASON3B {

// g_fScreenRate_x/y — references to global scope variables (defined in UIControls.cpp)
float& g_fScreenRate_x = ::g_fScreenRate_x;
float& g_fScreenRate_y = ::g_fScreenRate_y;

// Forward declarations — all NewUI classes M1-M6
class CNewUIPartyInfoWindow;

} // namespace SEASON3B

// Global scope stubs

