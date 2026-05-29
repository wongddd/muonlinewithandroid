#pragma once

struct HARDWARE_ID_INFO
{
	char HardwareId[36];
	WORD HardwareIdCount;
};

class CGMHardwareId
{
public:
	CGMHardwareId();
	virtual~CGMHardwareId();

	bool CheckHardwareId(char* HardwareId);
	void InsertHardwareId(char* HardwareId);
	void RemoveHardwareId(char* HardwareId);
private:
	std::map<std::string, HARDWARE_ID_INFO> m_HardwareInfo;
};

extern CGMHardwareId gmHardwareId;