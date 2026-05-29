// MemScript.cpp: implementation of the CMemScript class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemScript.h"
#include <sstream>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemScript::CMemScript() // OK
{
	this->m_buff = 0;
	this->m_size = 0;

	memset(this->m_path,0,sizeof(this->m_path));

	this->SetLastError(4);
}

CMemScript::~CMemScript() // OK
{
	if(this->m_buff != 0)
	{
		delete[] this->m_buff;
		this->m_buff = 0;
	}

	this->m_size = 0;
}

bool CMemScript::SetBuffer(char* path) // OK
{
	strcpy_s(this->m_path,path);

	HANDLE file = CreateFile(this->m_path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,0);

	if(file == INVALID_HANDLE_VALUE)
	{
		this->SetLastError(0);
		return 0;
	}

	this->m_size = GetFileSize(file,0);

	if(this->m_buff != 0)
	{
		delete[] this->m_buff;
		this->m_buff = 0;
	}

	this->m_buff = new char[this->m_size];

	if(this->m_buff == 0)
	{
		this->SetLastError(1);
		CloseHandle(file);
		return 0;
	}

	DWORD OutSize = 0;

	if(ReadFile(file,this->m_buff,this->m_size,&OutSize,0) == 0)
	{
		this->SetLastError(2);
		CloseHandle(file);
		return 0;
	}

	CloseHandle(file);

	this->m_count = 0;

	this->m_tick = GetTickCount();

	return 1;
}

bool CMemScript::SetBuffer(std::string path)
{
	strcpy_s(this->m_path, path.c_str());

	HANDLE file = CreateFile(this->m_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0);

	if (file == INVALID_HANDLE_VALUE)
	{
		this->SetLastError(0);
		return 0;
	}

	this->m_size = GetFileSize(file, 0);

	if (this->m_buff != 0)
	{
		delete[] this->m_buff;
		this->m_buff = 0;
	}

	this->m_buff = new char[this->m_size];

	if (this->m_buff == 0)
	{
		this->SetLastError(1);
		CloseHandle(file);
		return 0;
	}

	DWORD OutSize = 0;

	if (ReadFile(file, this->m_buff, this->m_size, &OutSize, 0) == 0)
	{
		this->SetLastError(2);
		CloseHandle(file);
		return 0;
	}

	CloseHandle(file);

	this->m_count = 0;

	this->m_tick = GetTickCount();

	return 1;
}

bool CMemScript::GetBuffer(char* buff,DWORD* size) // OK
{
	if(this->m_buff == 0)
	{
		this->SetLastError(3);
		return 0;
	}

	memcpy(buff,this->m_buff,this->m_size);
	(*size) = this->m_size;
	return 1;
}

char CMemScript::GetChar() // OK
{
	if(this->m_count >= this->m_size)
	{
		return -1;
	}

	return this->m_buff[this->m_count++];
}

void CMemScript::UnGetChar(char ch) // OK
{
	if(this->m_count == 0)
	{
		return;
	}

	this->m_buff[--this->m_count] = ch;
}

char CMemScript::CheckComment(char ch) // OK
{
	if(ch != '/' || (ch = this->GetChar()) != '/')
	{
		return ch;
	}

	while(true)
	{
		if((ch=this->GetChar()) == -1)
		{
			return ch;
		}

		if(ch == '\n')
		{
			return ch;
		}
	}

	return ch;
}

eTokenResult CMemScript::GetToken() // OK
{
	if((GetTickCount()-this->m_tick) > 1000)
	{
		this->SetLastError(4);
		throw 1;
	}

	this->m_number = 0;

	memset(this->m_string,0,sizeof(this->m_string));

	char ch;

	while(true)
	{
		if((ch=this->GetChar()) == -1)
		{
			return TOKEN_END;
		}

		if(isspace(ch) != 0)
		{
			continue;
		}

		if((ch=this->CheckComment(ch)) == -1)
		{
			return TOKEN_END;
		}
		else if(ch != '\n')
		{
			break;
		}
	}

	if(ch == '-' || ch == '.' || ch == '*' || isdigit(ch))
	{
		return this->GetTokenNumber(ch);
	}

	if(ch == '"')
	{
		return this->GetTokenString(ch);
	}

	return this->GetTokenCommon(ch);
}

eTokenResult CMemScript::GetTokenNumber(char ch) // OK
{
	int count = 0;

	this->UnGetChar(ch);

	while((ch=this->GetChar()) != -1 && (ch == '-' || ch == '.' || ch == '*' || isdigit(ch) != 0))
	{
		this->m_string[count++] = ch;
	}

	if(strcmp(this->m_string,"*") == 0)
	{
		this->m_number = -1;
	}
	else
	{
		this->m_number = (float)atof(this->m_string);
	}

	this->m_string[count] = 0;

	return TOKEN_NUMBER;
}

eTokenResult CMemScript::GetTokenString(char ch) // OK
{
	int count = 0;

	while((ch=this->GetChar()) != -1 && ch != '"')
	{
		this->m_string[count++] = ch;
	}

	if(ch != '"')
	{
		this->UnGetChar(ch);
	}

	this->m_string[count] = 0;

	return TOKEN_STRING;
}

eTokenResult CMemScript::GetTokenCommon(char ch) // OK
{
	if(isalpha(ch) == 0)
	{
		return TOKEN_ERROR;
	}

	int count = 0;

	this->m_string[count++] = ch;

	while((ch=this->GetChar()) != -1 && (ch == '.' || ch == '_' || isalnum(ch) != 0))
	{
		this->m_string[count++] = ch;
	}

	this->UnGetChar(ch);

	this->m_string[count] = 0;

	return TOKEN_STRING;
}

void CMemScript::SetLastError(int error) // OK
{
	switch(error)
	{
		case 0:
			wsprintf(this->m_LastError,MEM_SCRIPT_ERROR_CODE0,this->m_path);
			break;
		case 1:
			wsprintf(this->m_LastError,MEM_SCRIPT_ERROR_CODE1,this->m_path);
			break;
		case 2:
			wsprintf(this->m_LastError,MEM_SCRIPT_ERROR_CODE2,this->m_path);
			break;
		case 3:
			wsprintf(this->m_LastError,MEM_SCRIPT_ERROR_CODE3,this->m_path);
			break;
		case 4:
			wsprintf(this->m_LastError,MEM_SCRIPT_ERROR_CODE4,this->m_path);
			break;
		default:
			wsprintf(this->m_LastError,MEM_SCRIPT_ERROR_CODEX,this->m_path,error);
			break;
	}
}

char* CMemScript::GetLastError() // OK
{
	return this->m_LastError;
}

int CMemScript::GetNumber() // OK
{
	return (int)this->m_number;
}

int CMemScript::GetAsNumber() // OK
{
	this->GetToken();

	return (int)this->m_number;
}

DWORD CMemScript::GetAsCLRREF()
{
	BYTE r = (BYTE)this->GetAsNumber();

	BYTE g = (BYTE)this->GetAsNumber();

	BYTE b = (BYTE)this->GetAsNumber();

	BYTE a = 0xff;

	return (DWORD)((a << 24) | (r) | (g << 8) | (b << 16));
}

DWORD CMemScript::GetAsCLRREFF()
{
	BYTE r = static_cast<BYTE>(this->GetAsFloatNumber() * 255.f);

	BYTE g = static_cast<BYTE>(this->GetAsFloatNumber() * 255.f);

	BYTE b = static_cast<BYTE>(this->GetAsFloatNumber() * 255.f);

	BYTE a = 0xff;

	return (DWORD)((a << 24) | (r) | (g << 8) | (b << 16));
}

DWORD CMemScript::GetSTRINGRGB()
{
	std::string rgb = this->GetAsString(); // OK

	if (rgb.front() == '(' && rgb.back() == ')')
	{
		return -1;
	}
	else
	{
		rgb = rgb.substr(1, rgb.size() - 2);

		std::string temp;
		std::stringstream ss(rgb);

		// Leer y convertir los valores
		std::getline(ss, temp, ',');
		float red = std::stof(temp);

		std::getline(ss, temp, ',');
		float green = std::stof(temp);

		std::getline(ss, temp, ',');
		float blue = std::stof(temp);

		BYTE r = static_cast<BYTE>(red * 255.f);

		BYTE g = static_cast<BYTE>(green * 255.f);

		BYTE b = static_cast<BYTE>(blue * 255.f);

		return (DWORD)((r) | (g << 8) | (b << 16));
	}
}

float CMemScript::GetFloatNumber() // OK
{
	return this->m_number;
}

float CMemScript::GetAsFloatNumber() // OK
{
	this->GetToken();

	return this->m_number;
}

char* CMemScript::GetString() // OK
{
	return this->m_string;
}

char* CMemScript::GetAsString() // OK
{
	this->GetToken();

	return this->m_string;
}
