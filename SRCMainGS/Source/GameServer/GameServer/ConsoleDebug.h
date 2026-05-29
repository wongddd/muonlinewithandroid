#pragma once

enum MSG_TYPE
{
	MCD_SEND = 0x01,
	MCD_RECEIVE,
	MCD_ERROR,
	MCD_NORMAL
};

class CMuConsoleDebug
{
	bool	m_bInit;
public:
	virtual ~CMuConsoleDebug();
	static CMuConsoleDebug* sInstance();

	bool CheckCommand(const std::string& strCommand);
	void LogMessage(int iType, const char* pStr, ...);

protected:
	CMuConsoleDebug();
};

#define pLog		(CMuConsoleDebug::sInstance())