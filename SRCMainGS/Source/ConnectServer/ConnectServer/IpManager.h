// IpManager.h: interface for the CIpManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

struct IP_ADDRESS_INFO
{
	char IpAddress[16];
	WORD IpAddressCount;
	bool state_update;
};

class CIpManager
{
public:
	CIpManager();
	virtual ~CIpManager();
	bool CheckIpAddress(char* IpAddress);
	void InsertIpAddress(char* IpAddress);
	void RemoveIpAddress(char* IpAddress);

	bool CheckUpdateIpAddress(char* IpAddress);

	bool CurrentUpdateIpAddress(char* IpAddress, bool state_update);
private:
	std::map<std::string,IP_ADDRESS_INFO> m_IpAddressInfo;
};

extern CIpManager gIpManager;
