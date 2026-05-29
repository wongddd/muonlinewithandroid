#include "stdafx.h"
#include "ConnectServerProtocol.h"
#include "ClientManager.h"
#include "Log.h"
#include "Protect.h"
#include "ServerList.h"
#include "SocketManager.h"
#include "Util.h"
#include "IpManager.h"
#include "ConnectVersionHex.h"

void ConnectServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int size) // OK
{
	PROTECT_START

	gClientManager[index].m_PacketTime = GetTickCount();

	switch (head)
	{
	case 0xF4:
		switch (lpMsg[3])
		{
		case 0x03:
			CCServerInfoRecv((PMSG_SERVER_INFO_RECV*)lpMsg, index);
			break;
		case 0x04:
			CCServerHwidRecv((PMSG_SERVER_HWID_RECV*)lpMsg, index);
			break;
		case 0x06:
			CCServerListRecv((PMSG_SERVER_LIST_RECV*)lpMsg, index);
			break;
#ifdef SOCKET_LAUNCHER
		case 0x08:
			CCServerVersionRecv((PMSG_SERVER_VERSION_RECV*)lpMsg, index);
			break;
#endif // SOCKET_LAUNCHER

		}
		break;
	}

	PROTECT_FINAL
}

#include "CGMHardwareId.h"

void CCServerHwidRecv(PMSG_SERVER_HWID_RECV* lpMsg, int index)
{
	if (gServerList.CheckJoinServerState() == 0)
	{
		return;
	}

	CClientManager* lpClientManager = &gClientManager[index];

	if (lpClientManager->CheckState())
	{
		if (gmHardwareId.CheckHardwareId(lpMsg->ComputerHardwareId) == false)
		{
			gSocketManager.Disconnect(index);
		}
		else
		{
			lpClientManager->AddClient(lpMsg->ComputerHardwareId);
		}
	}
}

void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg, int index) // OK
{
	if (gServerList.CheckJoinServerState() == 0)
	{
		return;
	}

	SERVER_LIST_INFO* lpServerListInfo = gServerList.GetServerListInfo(lpMsg->ServerCode);

	if (lpServerListInfo == 0)
	{
		return;
	}

	if (lpServerListInfo->ServerShow == 0 || lpServerListInfo->ServerState == 0)
	{
		return;
	}

	PMSG_SERVER_INFO_SEND pMsg;

	pMsg.header.set(0xF4, 0x03, sizeof(pMsg));

	memcpy(pMsg.ServerAddress, lpServerListInfo->ServerAddress, sizeof(pMsg.ServerAddress));

	pMsg.ServerPort = lpServerListInfo->ServerPort;

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}

void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg, int index) // OK
{
	BYTE send[2048];

	PMSG_SERVER_LIST_SEND pMsg;

	pMsg.header.set(0xF4, 0x06, 0);

	int size = sizeof(pMsg);

	int count = gServerList.GenerateServerList(send, &size);

	pMsg.count[0] = SET_NUMBERHB(count);

	pMsg.count[1] = SET_NUMBERLB(count);

	pMsg.header.size[0] = SET_NUMBERHB(size);

	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);
}

void CCServerInitSend(int index, int result) // OK
{
	PMSG_SERVER_INIT_SEND pMsg;

	pMsg.header.set(0x00, sizeof(pMsg));

	pMsg.result = result;

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}

#ifdef SOCKET_LAUNCHER

bool connection_check_update(int index)
{
	CClientManager* lpClientManager = &gClientManager[index];

	return gIpManager.CurrentUpdateIpAddress(lpClientManager->m_IpAddr, true);
}

void CCServerUpdateSend(int index, WORD result, WORD Version, bool accumulated)
{
	PMSG_SERVER_VERSION_UPDATE_SEND pMsg;

	int size = sizeof(pMsg);

	pMsg.Header.set(0xF4, 0x09, size);

	pMsg.state = result;

	pMsg.Version = Version;

	if (accumulated == false)
	{
		gSocketManager.DataSend(index, (BYTE*)&pMsg, size);
	}
	else
	{
		gSocketManager.accumulated_message(index, (BYTE*)&pMsg, size);
	}
}


void CCServerVersionRecv(PMSG_SERVER_VERSION_RECV* lpMsg, int index)
{
	int ClientVersion = lpMsg->ClientVersion;

	if (ServerVersion <= 0)
	{
		CCServerUpdateSend(index, 3, 0);
	}
	else
	{
		if (ClientVersion == ServerVersion)
		{
			CCServerUpdateSend(index, 0, ServerVersion);
		}
		else
		{
			if (connection_check_update(index) == true)
			{
				if (ClientVersion > ServerVersion)
				{
					ClientVersion = 0;
				}

				CCServerUpdateSend(index, 1, ServerVersion);

				int MaxSize = MAX_MAIN_PACKET_SIZE - sizeof(PMSG_SERVER_FILE_VERSION_SEND);

				for (size_t Version = ClientVersion; Version <= ServerVersion; Version++)
				{
					std::vector<BYTE> data = GetServerVersion(Version);

					if (data.empty())
					{
						continue;
					}

					size_t offset = 0;
					int packetNumber = 1;
					size_t totalSize = data.size();

					while (offset < totalSize)
					{
						size_t chunkSize = min(MaxSize, totalSize - offset);

						BYTE send[MAX_MAIN_PACKET_SIZE];
						memset(send, 0, sizeof(send)); // Asegúrate de inicializar el buffer

						PMSG_SERVER_FILE_VERSION_SEND pMsg;
						pMsg.header.set(0xF4, 0x08, 0);

						pMsg.TotalSize = totalSize;
						pMsg.ChunkSize = chunkSize;
						pMsg.Offset = offset;
						pMsg.Version = Version;

						int size = sizeof(pMsg);
						pMsg.header.setSize(size + chunkSize);

						// Copiar los datos de la porción del archivo al buffer
						memcpy(&send[size], data.data() + offset, chunkSize);
						memcpy(send, &pMsg, sizeof(PMSG_SERVER_FILE_VERSION_SEND));

						// Actualizar los campos de tamaño del header
						size += chunkSize;

						// Enviar el paquete
						gSocketManager.accumulated_message(index, send, size);

						offset += chunkSize;
					}
				}
				CCServerUpdateSend(index, 2, ServerVersion, true);

				gSocketManager.send_queue(index);
			}
			else
			{
				CCServerUpdateSend(index, 4, 0);
			}
		}
	}
}
#endif // SOCKET_LAUNCHER
