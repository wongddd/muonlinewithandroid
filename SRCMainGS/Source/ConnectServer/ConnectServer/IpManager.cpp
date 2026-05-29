// IpManager.cpp: implementation of the CIpManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IpManager.h"

CIpManager gIpManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIpManager::CIpManager() // OK
{

}

CIpManager::~CIpManager() // OK
{
}

bool CIpManager::CheckIpAddress(char* IpAddress) // OK
{
	std::map<std::string,IP_ADDRESS_INFO>::iterator it = this->m_IpAddressInfo.find(std::string(IpAddress));

	if(it == this->m_IpAddressInfo.end())
	{
		return ((MaxIpConnection == 0) ? false : true);
	}
	else
	{
		return ((it->second.IpAddressCount >= MaxIpConnection) ? false : true);
	}
}

void CIpManager::InsertIpAddress(char* IpAddress) // OK
{
	IP_ADDRESS_INFO info;

	strcpy_s(info.IpAddress,IpAddress);

	info.IpAddressCount = 1;

#ifdef SOCKET_LAUNCHER
	info.state_update = FALSE;
#endif // SOCKET_LAUNCHER

	std::map<std::string,IP_ADDRESS_INFO>::iterator it = this->m_IpAddressInfo.find(std::string(IpAddress));

	if(it == this->m_IpAddressInfo.end())
	{
		this->m_IpAddressInfo.insert(std::pair<std::string,IP_ADDRESS_INFO>(std::string(IpAddress),info));
	}
	else
	{
		it->second.IpAddressCount++;
	}
}

void CIpManager::RemoveIpAddress(char* IpAddress) // OK
{
	std::map<std::string,IP_ADDRESS_INFO>::iterator it = this->m_IpAddressInfo.find(std::string(IpAddress));

	if(it != this->m_IpAddressInfo.end())
	{
		if((--it->second.IpAddressCount) == 0)
		{
			this->m_IpAddressInfo.erase(it);
		}
	}
}

#ifdef SOCKET_LAUNCHER

bool CIpManager::CheckUpdateIpAddress(char* IpAddress)
{
	std::map<std::string, IP_ADDRESS_INFO>::iterator it = this->m_IpAddressInfo.find(std::string(IpAddress));

	if (it != this->m_IpAddressInfo.end())
	{
		if (it->second.IpAddressCount > 1 && it->second.state_update == true)
		{
			return true;
		}
	}

	return false;
}

bool CIpManager::CurrentUpdateIpAddress(char* IpAddress, bool state_update)
{
	std::map<std::string, IP_ADDRESS_INFO>::iterator it = this->m_IpAddressInfo.find(std::string(IpAddress));

	if (it != this->m_IpAddressInfo.end())
	{
		if (it->second.IpAddressCount > 1 && it->second.state_update == true)
		{
			return false;
		}

		it->second.state_update = state_update;
	}

	return true;
}

#endif // SOCKET_LAUNCHER
