#include "StdAfx.h"
#include "Util.h"
#include "ConnectVersionHex.h"

int ServerVersion = 0;

std::vector<ConnectVersionHex> VersionHex;

ConnectVersionHex::ConnectVersionHex()
{
	Version = 0;
}

ConnectVersionHex::~ConnectVersionHex()
{
	buffer.clear();
}

int ConnectVersionHex::GetVersion()
{
	return Version;
}

size_t ConnectVersionHex::GetFileSize()
{
	return buffer.size();
}

std::vector<BYTE> ConnectVersionHex::GetFileBuffer()
{
	return buffer; // Si el buffer está vacío, retornará un vector vacío
}

int ConnectVersionHex::OpenDataVersion(int version, std::string filename)
{
	Version = version;

	FILE* fp = fopen(filename.c_str(), "rb");

	if (fp == NULL)
	{
		return (-1);
	}

	fseek(fp, 0, SEEK_END);

	unsigned int FileSize = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	buffer = std::vector<BYTE>(FileSize, 0);

	fread(buffer.data(), 1u, FileSize, fp);

	fclose(fp);

	return FileSize;
}

void LoadServerVersion()
{
	VersionHex.clear();

	ServerVersion = GetPrivateProfileInt("ConnectServerInfo", "ConnectServerVersion", 0, ".\\Data\\ConnectServer.ini");

	char wildcard_path[MAX_PATH];

	wsprintf(wildcard_path, "%s*", ".\\Data\\Update\\");

	WIN32_FIND_DATA data;

	HANDLE file = FindFirstFile(wildcard_path, &data);

	if (file == INVALID_HANDLE_VALUE)
	{
		return;
	}

	int m_count = 0;

	char Path[MAX_PATH];

	do
	{
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			if (isdigit(data.cFileName[0]) != 0 && isdigit(data.cFileName[1]) != 0 && isdigit(data.cFileName[2]) != 0)
			{
				if (data.cFileName[3] == ' ' && data.cFileName[4] == '-' && data.cFileName[5] == ' ')
				{
					int version = atoi(data.cFileName);

					ConnectVersionHex info;

					wsprintf(Path, ".\\Data\\Update\\%s", data.cFileName);

					size_t totalSize = info.OpenDataVersion(version, Path);

					if (totalSize != 0)
					{
						VersionHex.push_back(info);
					}
				}
			}
		}
	} while (FindNextFile(file, &data) != 0);

	LogAdd(LOG_BLUE, "[ServerVersion] ServerVersion loaded successfully");
}

std::vector<BYTE> GetServerVersion(int version)
{
	std::vector<BYTE> EncData;

	for (size_t i = 0; i < VersionHex.size(); i++)
	{
		if (VersionHex[i].GetVersion() == version)
		{
			// Obtiene el buffer como un vector de bytes
			EncData = VersionHex[i].GetFileBuffer();

			// Retorna el vector si no está vacío
			if (!EncData.empty())
			{
				return EncData;
			}
		}
	}

	// Si no se encuentra la versión, retorna un vector vacío
	return EncData;
}
