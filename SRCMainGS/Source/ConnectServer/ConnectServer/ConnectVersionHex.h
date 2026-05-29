#pragma once
class ConnectVersionHex
{
public:
	ConnectVersionHex();
	virtual~ConnectVersionHex();
	int GetVersion();
	size_t GetFileSize();
	std::vector<BYTE> GetFileBuffer();
	int OpenDataVersion(int version, std::string filename);
private:
	int Version;
	std::vector<BYTE> buffer;
};

extern int ServerVersion;
extern std::vector<ConnectVersionHex> VersionHex;


extern void LoadServerVersion();
extern std::vector<BYTE> GetServerVersion(int version);