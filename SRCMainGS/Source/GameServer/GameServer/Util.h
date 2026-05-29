#pragma once

#include "ServerDisplayer.h"
#include "User.h"

std::string ito_string(int number);
std::string fto_string(float number);

int SafeGetItem(int index);
flt GetRoundValue(float value);
BYTE GetNewOptionCount(BYTE NewOption);
BYTE GetSocketOptionCount(BYTE SocketOption[5]);
BYTE GetPathPacketDirPos(int px,int py);
void PacketArgumentDecrypt(char* out_buff,char* in_buff,int size);
void ErrorMessageBox(char* message,...);
void LogAdd(eLogColor color,char* text,...);
void LogAddConnect(eLogColor color,char* text,...);
bool DataSend(int aIndex,BYTE* lpMsg,DWORD size);
void DataSendAll(BYTE* lpMsg,int size);
bool DataSendSocket(SOCKET socket,BYTE* lpMsg,DWORD size);
void MsgSendV2(LPOBJ lpObj,BYTE* lpMsg,int size);
void CloseClient(int aIndex);
void PostMessage1(char* name,char* message,char* text);
void PostMessage2(char* name,char* message,char* text);
void PostMessage3(char* name,char* message,char* text);
void PostMessage4(char* name,char* message,char* text);
void PostMessagePK(char* name,char* message,char* text);
void PostMessageUserON(char* name,char* message);
void PostMessageNew(char* name,char* message,char* text);
void SetLargeRand();
long GetLargeRand();

void ConvertSecondsToTime(int index, int total_seconds, int type = 0);
void writeFile(FILE* fp, int Number, int columnWidth, bool Break);
void writeFile(FILE* fp, const char* textvalue, int columnWidth, bool Break, bool as = false);

extern void PackFileEncrypt(const char* filename, BYTE* pbyBuffer, int MaxLine, int Size);
extern void PackFileEncrypt(const char* filename, BYTE* pbyBuffer, int MaxLine, int Size, DWORD Key, bool WriteMax = true, bool CheckSum = true);
extern std::string ConvertWStringToString(const std::wstring& wstr);
extern std::wstring ConvertStringToWString(const std::string& str);
extern bool isArrayZeroed(const BYTE* array, int Size = 16);
extern BOOL WriteUnicode(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString);
extern short RoadPathTable[MAX_ROAD_PATH_TABLE];
bool CheckChuoiKyTuDacBiet(const std::string& str);