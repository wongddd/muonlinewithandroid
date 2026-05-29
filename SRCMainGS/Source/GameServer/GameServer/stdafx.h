#pragma once
#pragma warning( disable : 4005 )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4482 )
#pragma warning( disable : 4800 )
#pragma warning( disable : 4996 )
#pragma warning( disable : 4091 )
#pragma warning( disable : 4094 )
#pragma warning( disable : 4172 )
#pragma warning( disable : 4101 )
#pragma warning( disable : 4102 )
#pragma warning( disable : 4309 )
#pragma warning( disable : 4067 )
#pragma warning( disable : 4805 )
#pragma warning( disable : 4154 )
#pragma warning( disable : 4733 )
#pragma warning( disable : 4620 )

#define WIN32_LEAN_AND_MEAN

#define _WIN32_WINNT _WIN32_WINNT_WIN7


#define GAMESERVER_CLIENT "MGM EMULATOR"

#define GAMESERVER_CLIENTE_UPDATE 16
#define GAMESERVER_CLIENTE_PREMIUM 0
#define GAMESERVER_EDITH_EXPORT


#if(GAMESERVER_TYPE==0)
#define GAMESERVER_VERSION "GS"
#else
#define GAMESERVER_VERSION "GSCS"
#endif

#ifndef GAMESERVER_TYPE
#define GAMESERVER_TYPE 0
#endif

#ifndef GAMESERVER_EXTRA
#define GAMESERVER_EXTRA 1
#endif

#ifndef GAMESERVER_UPDATE
#define GAMESERVER_UPDATE 803
#endif

#if(GAMESERVER_UPDATE==401)
#define GAMESERVER_SEASON "SEASON 4"
#endif

#if(GAMESERVER_UPDATE==603)
#define GAMESERVER_SEASON "SEASON 6"
#endif

#if(GAMESERVER_UPDATE==803)
#define GAMESERVER_SEASON "SEASON 8"
#endif

#ifndef GAMESERVER_LANGUAGE
#define GAMESERVER_LANGUAGE 1
#endif

#ifndef PROTECT_STATE
#define PROTECT_STATE 1
#endif

#ifndef ENCRYPT_STATE
#define ENCRYPT_STATE 1
#endif

#if(PROTECT_STATE==0)
#define GAMESERVER_NAME "Free"
#else
#define GAMESERVER_NAME "Premium"
#endif

#define CHARACTER_EQUIPEMENT_EXT

//MC bot

//----------------------------
//	 ZG-Defines			     -
//	 1:ON  (CPP AND HEADER!) -
//	 2:OFF (CPP AND HEADER!) -
//----------------------------

#ifndef POINT_CUSTOM 
#define POINT_CUSTOM 1 // ShopPointEx
#endif

#define PC_POINT					2 //Agregar algun npc o bot con PCP
#define HAPPY_HOURS					2 //[80%]->Falta Arreglar Los Vip!
#define MASTER_HOURS				2
#define ACHERON_GUARDIAN			2
#define BOT_BUFFER					1
#define ARCA_WAR					2	// Mix nomas!!
#define GOLDEN_ARCHER				2	//
#define MAX_ADD_OPTION_INFO			4
//-- Need All Types
#define PG_CUSTOM					1
#define	ALLBOTSSTRUC				1
//MC bot

// System Include
#include <windows.h>
#include <winsock2.h>
#include <mswSock.h>
#include <commctrl.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <random>
#include <Rpc.h>
#include <algorithm>
#include <string>
#include <atltime.h>
#include <dbghelp.h>
#include <Psapi.h>
#include "pugixml.hpp"
#include <WinGdi.h> // Para AlphaBlend

#include <sstream>
#include <locale>
#include <codecvt>

#include <Mmsystem.h>

#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib, "Msimg32.lib") // Enlazar con la biblioteca Msimg32.lib

#if(GAMESERVER_UPDATE>=701)
#if(NDEBUG==0)
#pragma comment(lib,"..\\..\\Util\\cryptopp\\Debug\\cryptlib.lib")
#else
#pragma comment(lib,"..\\..\\Util\\cryptopp\\Release\\cryptlib.lib")
#endif
#pragma comment(lib,"..\\..\\Util\\mapm\\mapm.lib")
#endif

typedef char chr;

typedef float flt;

typedef short shrt;

typedef unsigned __int64 QWORD;

typedef int int32;

typedef unsigned int uint32;

//#define CONSOLE

#include "ConsoleDebug.h"
#include "define_global.h"