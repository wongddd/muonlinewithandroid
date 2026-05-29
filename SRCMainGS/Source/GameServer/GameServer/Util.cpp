#include "stdafx.h"
#include "Util.h"
#include "GameMain.h"
#include "HackCheck.h"
#include "ItemManager.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "ThemidaSDK.h"
#include "Viewport.h"

std::mt19937 seed;
std::uniform_int_distribution<int> dist;
short RoadPathTable[MAX_ROAD_PATH_TABLE] = { -1,-1,0,-1,1,-1,1,0,1,1,0,1,-1,1,-1,0 };

extern HWND hWnd;

std::string ito_string(int number)
{
	std::ostringstream buff;
	buff << number;
	return buff.str();
}

std::string fto_string(float number)
{
	std::ostringstream buff;
	buff << number;
	return buff.str();
}

int SafeGetItem(int index) // OK
{
	return CHECK_ITEM(index);
}

flt GetRoundValue(float value) // OK
{
	float integral;

	if (modf(value, &integral) > 0.5f)
	{
		return ceil(value);
	}

	return floor(value);
}

BYTE GetNewOptionCount(BYTE NewOption) // OK
{
	BYTE count = 0;

	for (int n = 0; n < MAX_EXC_OPTION; n++)
	{
		if ((NewOption & (1 << n)) != 0)
		{
			count++;
		}
	}

	return count;
}

BYTE GetSocketOptionCount(BYTE SocketOption[5]) // OK
{
	BYTE count = 0;

	for (int n = 0; n < MAX_SOCKET_OPTION; n++)
	{
		if (SocketOption[n] != 0xFF)
		{
			count++;
		}
	}

	return count;
}

BYTE GetPathPacketDirPos(int px, int py) // OK
{
	if (px <= -1 && py <= -1)
	{
		return 0;
	}
	else if (px <= -1 && py == 0)
	{
		return 7;
	}
	else if (px <= -1 && py >= 1)
	{
		return 6;
	}
	else if (px == 0 && py <= -1)
	{
		return 1;
	}
	else if (px == 0 && py >= 1)
	{
		return 5;
	}
	else if (px >= 1 && py <= -1)
	{
		return 2;
	}
	else if (px >= 1 && py == 0)
	{
		return 3;
	}
	else if (px >= 1 && py >= 1)
	{
		return 4;
	}

	return 0;
}

void PacketArgumentDecrypt(char* out_buff, char* in_buff, int size) // OK
{
	BYTE XorTable[3] = { 0xFC,0xCF,0xAB };

	for (int n = 0; n < size; n++)
	{
		out_buff[n] = in_buff[n] ^ XorTable[n % 3];
	}
}

void ErrorMessageBox(char* message, ...) // OK
{
	VM_START;

	char buff[256];

	memset(buff, 0, sizeof(buff));

	va_list arg;
	va_start(arg, message);
	vsprintf_s(buff, message, arg);
	va_end(arg);

	MessageBox(hWnd, buff, "Error", MB_OK | MB_ICONERROR);
	VM_END;

	ExitProcess(0);
}

void LogAdd(eLogColor color, char* text, ...) // OK
{
	tm today;
	time_t ltime;
	time(&ltime);

	if (localtime_s(&today, &ltime) != 0)
	{
		return;
	}

	char time[32];

	if (asctime_s(time, sizeof(time), &today) != 0)
	{
		return;
	}

	char temp[1024];

	va_list arg;
	va_start(arg, text);
	vsprintf_s(temp, text, arg);
	va_end(arg);

	char log[1024];

	wsprintf(log, "%.8s %s", &time[11], temp);

	gServerDisplayer.LogAddText(color, log, strlen(log));
}

void LogAddConnect(eLogColor color, char* text, ...) // OK
{
	tm today;
	time_t ltime;
	time(&ltime);

	if (localtime_s(&today, &ltime) != 0)
	{
		return;
	}

	char time[32];

	if (asctime_s(time, sizeof(time), &today) != 0)
	{
		return;
	}

	char temp[1024];

	va_list arg;
	va_start(arg, text);
	vsprintf_s(temp, text, arg);
	va_end(arg);

	char log[1024];

	wsprintf(log, "%.8s %s", &time[11], temp);

	gServerDisplayer.LogAddTextConnect(color, log, strlen(log));
}

bool DataSend(int aIndex, BYTE* lpMsg, DWORD size) // OK
{
	return gSocketManager.DataSend(aIndex, lpMsg, size);
}

void DataSendAll(BYTE* lpMsg, int size) // OK
{
	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnected(n) != 0)
		{
			DataSend(n, lpMsg, size);
		}
	}
}

bool DataSendSocket(SOCKET socket, BYTE* lpMsg, DWORD size) // OK
{
	if (socket == INVALID_SOCKET)
	{
		return 0;
	}

#if(ENCRYPT_STATE==1)

	EncryptData(lpMsg, size);

#endif

	int count = 0, result = 0;

	while (size > 0)
	{
		if ((result = send(socket, (char*)&lpMsg[count], size, 0)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				return 0;
			}
		}
		else
		{
			count += result;
			size -= result;
		}
	}

	return 1;
}

void MsgSendV2(LPOBJ lpObj, BYTE* lpMsg, int size) // OK
{
	for (int n = 0; n < MAX_VIEWPORT; n++)
	{
		if (lpObj->VpPlayer2[n].state != VIEWPORT_NONE && lpObj->VpPlayer2[n].type == OBJECT_USER && OBJECT_BOTS)
		{
			DataSend(lpObj->VpPlayer2[n].index, lpMsg, size);
		}
	}
}

void CloseClient(int aIndex) // OK
{
	gSocketManager.Disconnect(aIndex);
}

void PostMessage1(char* name, char* message, char* text) // OK
{
#if(GAMESERVER_UPDATE>=701)

	char buff[256] = { 0 };

	wsprintf(buff, message, name, text);

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n], buff);
		}
	}

#else

	char buff[256] = { 0 };

	wsprintf(buff, message, text);

	int size = strlen(buff);

	size = ((size > MAX_CHAT_MESSAGE_SIZE-1) ? MAX_CHAT_MESSAGE_SIZE-1 : size);

	PMSG_CHAT_WHISPER_SEND pMsg;

	pMsg.header.set(0x02, (sizeof(pMsg) - (sizeof(pMsg.message) - (size + 1))));

	memcpy(pMsg.name, name, sizeof(pMsg.name));

	memcpy(pMsg.message, buff, size);

	pMsg.message[size] = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}

#endif
}

void PostMessage2(char* name, char* message, char* text) // OK
{
#if(GAMESERVER_UPDATE>=701)

	char buff[256] = { 0 };

	wsprintf(buff, message, name, text);

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n], buff);
		}
	}

#else

	char buff[256] = { '~' };

	wsprintf(&buff[1], message, text);

	int size = strlen(buff);

	size = ((size > MAX_CHAT_MESSAGE_SIZE-1) ? MAX_CHAT_MESSAGE_SIZE-1 : size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00, (sizeof(pMsg) - (sizeof(pMsg.message) - (size + 1))));

	memcpy(pMsg.name, name, sizeof(pMsg.name));

	memcpy(pMsg.message, buff, size);

	pMsg.message[size] = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}

#endif
}

void PostMessage3(char* name, char* message, char* text) // OK
{
#if(GAMESERVER_UPDATE>=701)

	char buff[256] = { 0 };

	wsprintf(buff, message, name, text);

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n], buff);
		}
	}

#else

	char buff[256] = { '@' };

	wsprintf(&buff[1], message, text);

	int size = strlen(buff);

	size = ((size > MAX_CHAT_MESSAGE_SIZE-1) ? MAX_CHAT_MESSAGE_SIZE-1 : size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00, (sizeof(pMsg) - (sizeof(pMsg.message) - (size + 1))));

	memcpy(pMsg.name, name, sizeof(pMsg.name));

	memcpy(pMsg.message, buff, size);

	pMsg.message[size] = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}

#endif
}

void PostMessage4(char* name, char* message, char* text) // OK
{
#if(GAMESERVER_UPDATE>=701)

	char buff[256] = { 0 };

	wsprintf(buff, message, name, text);

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n], buff);
		}
	}

#else

	char buff[256] = { '$' };

	wsprintf(&buff[1], message, text);

	int size = strlen(buff);

	size = ((size > MAX_CHAT_MESSAGE_SIZE-1) ? MAX_CHAT_MESSAGE_SIZE-1 : size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00, (sizeof(pMsg) - (sizeof(pMsg.message) - (size + 1))));

	memcpy(pMsg.name, name, sizeof(pMsg.name));

	memcpy(pMsg.message, buff, size);

	pMsg.message[size] = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}

#endif
}

void PostMessagePK(char* name, char* message, char* text) // OK
{
#if(GAMESERVER_UPDATE>=701)

	char buff[256] = { 0 };

	wsprintf(buff, message, name, text);

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n], buff);
		}
	}

#else

	char buff[256] = { '~' };

	wsprintf(&buff[1], message, name, text);

	int size = strlen(buff);

	size = ((size > MAX_CHAT_MESSAGE_SIZE-1) ? MAX_CHAT_MESSAGE_SIZE-1 : size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00, (sizeof(pMsg) - (sizeof(pMsg.message) - (size + 1))));

	memcpy(pMsg.name, "[PK]", sizeof(pMsg.name));

	memcpy(pMsg.message, buff, size);

	pMsg.message[size] = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}

#endif
}

void PostMessageUserON(char* name, char* message) // OK
{
#if(GAMESERVER_UPDATE>=701)

	char buff[256] = { 0 };

	wsprintf(buff, message, name);

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n], buff);
		}
	}

#else

	char buff[256] = { '@' };

	wsprintf(&buff[1], message, name);

	int size = strlen(buff);

	size = ((size > MAX_CHAT_MESSAGE_SIZE-1) ? MAX_CHAT_MESSAGE_SIZE-1 : size);

	PMSG_CHAT_SEND pMsg;

	pMsg.header.set(0x00, (sizeof(pMsg) - (sizeof(pMsg.message) - (size + 1))));

	memcpy(pMsg.name, "[UserON]", sizeof(pMsg.name));

	memcpy(pMsg.message, buff, size);

	pMsg.message[size] = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}

#endif
}

void PostMessageNew(char* name, char* message, char* text) // OK
{
#if(GAMESERVER_UPDATE>=701)

	char buff[256] = { 0 };

	wsprintf(buff, message, name, text);

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			GCNewMessageSend(&gObj[n], buff);
		}
	}

#else

	char buff[256] = { 0 };

	wsprintf(buff, message, name, text);

	int size = strlen(buff);

	size = ((size > MAX_CHAT_MESSAGE_SIZE-1) ? MAX_CHAT_MESSAGE_SIZE-1 : size);

	PMSG_CHAT_WHISPER_SEND pMsg;

	pMsg.header.set(0x02, (sizeof(pMsg) - (sizeof(pMsg.message) - (size + 1))));

	memcpy(pMsg.name, "[POST]", sizeof(pMsg.name));

	memcpy(pMsg.message, buff, size);

	pMsg.message[size] = 0;

	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObjIsConnectedGP(n) != 0)
		{
			DataSend(n, (BYTE*)&pMsg, pMsg.header.size);
		}
	}
#endif
}

void SetLargeRand() // OK
{
	seed = std::mt19937(std::random_device());
	dist = std::uniform_int_distribution<int>(0, 2147483647);
}

long GetLargeRand() // OK
{
	return dist(seed);
}

void writeFile(FILE* fp, int Number, int columnWidth, bool Break)
{
	char buffer[50];

	if (Break)
	{
		if (columnWidth != 0)
			sprintf_s(buffer, "%-*d\n", columnWidth, Number);
		else
			sprintf_s(buffer, "%d\n", Number);
	}
	else
	{
		if (columnWidth != 0)
			sprintf_s(buffer, "%-*d", columnWidth, Number);
		else
			sprintf_s(buffer, "%d", Number);
	}

	fwrite(buffer, 1, strlen(buffer), fp);
}

void writeFile(FILE* fp, const char* textvalue, int columnWidth, bool Break, bool as)
{
	char buffer[MAX_PATH];
	char buffTemp[MAX_PATH];

	if (as)
		sprintf_s(buffTemp, "\"%s\"", textvalue);
	else
		sprintf_s(buffTemp, "%s", textvalue);


	if (Break)
	{
		if (columnWidth != 0)
			sprintf_s(buffer, "%-*s\n", columnWidth, buffTemp);
		else
			sprintf_s(buffer, "%s\n", buffTemp);
	}
	else
	{
		if (columnWidth != 0)
			sprintf_s(buffer, "%-*s", columnWidth, buffTemp);
		else
			sprintf_s(buffer, "%s", buffTemp);
	}

	fwrite(buffer, 1, strlen(buffer), fp);
}

void BuxConvert(BYTE* pbyBuffer, int Size)
{
	BYTE bBuxCode[3] = { 0xFC, 0xCF, 0xAB };

	for (int i = 0; i < Size; i++)
	{
		pbyBuffer[i] ^= bBuxCode[i % 3];
	}
}

DWORD GenerateCheckSum2(BYTE* pbyBuffer, int dwSize, WORD Key)
{
	int dwKey = Key;
	int dwResult = Key << 9;

	for (int dwChecked = 0; dwChecked <= dwSize - 4; dwChecked += 4)
	{
		DWORD dwTemp;

		memcpy(&dwTemp, pbyBuffer + dwChecked, 4);

		DWORD v4 = (Key + (dwChecked >> 2)) % 2;
		switch (v4)
		{
		case 0:
			dwResult ^= dwTemp;
			break;
		case 1:
			dwResult += dwTemp;
			break;
		}

		if (!(dwChecked % 0x10))
		{
			dwResult ^= (unsigned int)(dwResult + dwKey) >> ((dwChecked >> 2) % 8 + 1);
		}
	}
	return dwResult;
}

void PackFileEncrypt(const char* filename, BYTE* pbyBuffer, int MaxLine, int Size)
{
	FILE* fp = fopen(filename, "wb");

	if (fp != NULL)
	{
		DWORD MAX_BUFFER = (MaxLine * Size);

		BuxConvert(pbyBuffer, MAX_BUFFER);
		fwrite(pbyBuffer, MAX_BUFFER, 1u, fp);
		fclose(fp);
	}
}

void PackFileEncrypt(const char* filename, BYTE* pbyBuffer, int MaxLine, int Size, DWORD Key, bool WriteMax, bool CheckSum)
{
	FILE* fp = fopen(filename, "wb");

	if (fp != NULL)
	{
		DWORD MAX_BUFFER = (MaxLine * Size);

		BYTE* Buffer = new BYTE[MAX_BUFFER];

		if (WriteMax == true)
		{
			fwrite(&MaxLine, 4u, 1u, fp);
		}

		for (int i = 0; i < MaxLine; ++i)
		{
			BuxConvert(pbyBuffer, Size);

			memcpy((char*)(Buffer + Size * i), pbyBuffer, Size);

			fwrite((char*)(Buffer + Size * i), Size, 1u, fp);

			pbyBuffer += Size;
		}

		if (CheckSum)
		{
			DWORD dwCheckSum = GenerateCheckSum2(Buffer, MAX_BUFFER, Key);

			fwrite(&dwCheckSum, 4u, 1u, fp);
		}

		fclose(fp);

		delete[] Buffer;
	}
}

// Conversión entre wchar_t* y char*
std::string ConvertWStringToString(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(wstr);
}

std::wstring ConvertStringToWString(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.from_bytes(str);
}

bool isArrayZeroed(const BYTE* array, int Size)
{
	BYTE zeroArray[16];
	memset(zeroArray, 0, 16);
	return memcmp(array, zeroArray, Size) == 0;
}

BOOL IsCharUTF8(const char* pszText)
{
	if (pszText == NULL || strlen(pszText) <= 0)
		return TRUE;

	const char* pbyCurr = pszText;
	BOOL bUTF8 = TRUE;

	while ((*pbyCurr != 0x00) && bUTF8)
	{
		// U+0000 ~ U+007F (0 ~ 127)
		if ((*pbyCurr & 0x80) == 0x00)
		{
			pbyCurr++;
		} // U+0080 ~ U+07FF (128 ~ 2,047)
		else if (((*pbyCurr & 0xE0) == 0xC0) && ((*(pbyCurr + 1) & 0xC0) == 0x80))
		{
			/*if ((*pbyCurr & 0xFE) == 0xC0)
			{ // Overlong check
				bUTF8 = FALSE;
			}*/
			pbyCurr += 2;
		} // U+0800 ~ U+FFFF (2,048 ~ 65,535)
		else if (((*pbyCurr & 0xF0) == 0xE0) && ((*(pbyCurr + 1) & 0xC0) == 0x80) && ((*(pbyCurr + 2) & 0xC0) == 0x80))
		{
			/*if ((*pbyCurr == 0xE0 && (*(pbyCurr + 1) & 0xE0) == 0x80) ||  // Overlong check
				(*pbyCurr == 0xED && (*(pbyCurr + 1) & 0xE0) == 0xA0))
			{ // Surrogate range
				bUTF8 = FALSE;
			}*/
			pbyCurr += 3;
		} // U+10000 ~ U+10FFFF (65,536 ~ 1,114,111)
		else if (((*pbyCurr & 0xF8) == 0xF0) &&
			((*(pbyCurr + 1) & 0xC0) == 0x80) && ((*(pbyCurr + 2) & 0xC0) == 0x80) && ((*(pbyCurr + 3) & 0xC0) == 0x80))
		{
			/*if ((*pbyCurr == 0xF0 && (*(pbyCurr + 1) & 0xF0) == 0x80) || // Overlong check
				(*pbyCurr > 0xF4 || (*(pbyCurr + 3) & 0xF8) != 0xF0))
			{   // Out of range
				bUTF8 = FALSE;
			}*/
			pbyCurr += 4;
		} // not UTF-8
		else
		{
			bUTF8 = FALSE;
		}
	}
	return bUTF8;
}

int ConvertCharToWideStr(std::wstring& wstrDest, LPCSTR lpString)
{
	wstrDest = L"";

	if (lpString == NULL || strlen(lpString) <= 0)
		return 0;

	int nLenOfWideCharStr;
	int iConversionType;

#ifdef CHINESE_LANGUAGE
	iConversionType = CHINESE_CP;
#else
	if (strcmp(lpString, "¸ÖÆ¼ ·Î±ä ¹öÁ¯") == 0)
		iConversionType = (IsCharUTF8(lpString)) ? CP_UTF8 : 949;
	else
		iConversionType = (IsCharUTF8(lpString)) ? CP_UTF8 : 1252;
#endif
	//iConversionType = (IsCharUTF8(lpString)) ? CP_UTF8 : iCodePage;

	// calculate the number of characters needed to hold the wide-character version of the string.
	nLenOfWideCharStr = MultiByteToWideChar(iConversionType, 0, lpString, -1, NULL, 0);
	// memory allocation
	wchar_t* pwszStr = new wchar_t[nLenOfWideCharStr];

	// convert the multi-byte string to a wide-character string.
	MultiByteToWideChar(iConversionType, 0, lpString, -1, pwszStr, nLenOfWideCharStr);

	//assign
	wstrDest += pwszStr;

	// release the allocated memory.
	delete[] pwszStr;

	return nLenOfWideCharStr - 1;
}

BOOL WriteUnicode(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
{
	std::wstring wstrText = L"";
	ConvertCharToWideStr(wstrText, lpString);

	return TextOutW(hdc, nXStart, nYStart, wstrText.c_str(), wstrText.length());
}

bool CheckChuoiKyTuDacBiet(const std::string& str)
{
	for (int i = 0; i < str.size(); i++)
	{
		if (!isdigit(str[i]) && !(str[i] >= 'a' && str[i] <= 'z') && !(str[i] >= 'A' && str[i] <= 'Z'))
		{
			return false;
		}
	}
	return true;
}