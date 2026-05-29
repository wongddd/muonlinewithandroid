#pragma once
#pragma warning( disable : 4067 ) 
#pragma warning( disable : 4099 )
#pragma warning( disable : 4786 ) 
#pragma warning( disable : 4800 ) 
#pragma warning( disable : 4996 ) 
#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )
#pragma warning( disable : 4503 ) 
#pragma warning( disable : 4267 ) 
#pragma warning( disable : 4091 ) 
#pragma warning( disable : 4819 )
#pragma warning( disable : 4505 )
#pragma warning( disable : 4100 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4702 )
#pragma warning( disable : 4838 )
#pragma warning( disable : 28159 )
#pragma warning( disable : 26812 )
#pragma warning( disable : 28251 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 26819 )

// System Include
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>

#ifndef GAMESERVER_PREMIUM
#define GAMESERVER_PREMIUM 1
#endif


#define MAX_INSTANCE_GAME
#define SHUTDOWN_LEVEL_WING3




#define MaxLineItemToolTip						3000

#define MAX_PLUGINS							5
#define MAX_CLASS							7
#define MAX_CUSTOM_MAPE						150
#define MAX_CUSTOM_ITEM						2048
#define MAX_CUSTOM_IMAGE					512
#define MAX_CUSTOM_MONSTER					512
#define MAX_CUSTOM_SMOKE					512
#define MAX_CUSTOM_NPC_NAME					100
#define MAX_CUSTOM_WINGS					256
#define MAX_CUSTOM_STACK					256
#define MAX_CUSTOM_PET						256
#define MAX_CUSTOM_EFFECT					5000
#define MAX_MEMBERSHIP_VIP					100
#define MAX_CUSTOM_COMMAND					100
#define MAX_CHARACTER_NAME					100

#define MAX_ITEM_TYPE						16
#define MAX_ITEM_INDEX						512
#define MAX_ITEM							(MAX_ITEM_TYPE*MAX_ITEM_INDEX)
#define ITEM_DEF(x, y)						((x * MAX_ITEM_INDEX + y))


#define SAFE_DELETE(p)						{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)				{ if(p) { delete [] (p);     (p)=NULL; } }
