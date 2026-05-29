#include "StdAfx.h"
#include "CGMHardwareId.h"

CGMHardwareId gmHardwareId;

CGMHardwareId::CGMHardwareId()
{
}

CGMHardwareId::~CGMHardwareId()
{
	this->m_HardwareInfo.clear();
}

bool CGMHardwareId::CheckHardwareId(char* HardwareId)
{
	std::map<std::string, HARDWARE_ID_INFO>::iterator it = this->m_HardwareInfo.find(std::string(HardwareId));

	if (it == this->m_HardwareInfo.end())
	{
		return ((MaxHWIDConnection == 0) ? false : true);
	}
	else
	{
		return ((it->second.HardwareIdCount >= MaxHWIDConnection) ? false : true);
	}
}

void CGMHardwareId::InsertHardwareId(char* HardwareId)
{
	std::map<std::string, HARDWARE_ID_INFO>::iterator it = this->m_HardwareInfo.find(std::string(HardwareId));

	if (it != this->m_HardwareInfo.end())
	{
		it->second.HardwareIdCount++;
	}
	else
	{
		HARDWARE_ID_INFO info;

		strcpy_s(info.HardwareId, HardwareId);

		info.HardwareIdCount = 1;

		this->m_HardwareInfo.insert(std::pair<std::string, HARDWARE_ID_INFO>(std::string(HardwareId), info));
	}
}

void CGMHardwareId::RemoveHardwareId(char* HardwareId)
{
	std::map<std::string, HARDWARE_ID_INFO>::iterator it = this->m_HardwareInfo.find(std::string(HardwareId));

	if (it != this->m_HardwareInfo.end())
	{
		if ((--it->second.HardwareIdCount) == 0)
		{
			this->m_HardwareInfo.erase(it);
		}
	}
}
