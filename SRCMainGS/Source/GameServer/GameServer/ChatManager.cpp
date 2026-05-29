#include "StdAfx.h"
#include "ServerInfo.h"
#include "ChatManager.h"

CChatManager* CChatManager::Instance()
{
	static CChatManager sInstance;
	return &sInstance;
}

bool CChatManager::InvokeMessage(const std::string& user, const std::string& message)
{
	bool success;
	std::time_t now = std::time(nullptr);

	// Check for spam
	if (InvokeSpamming(user, message, now))
	{
		success = false;
		//std::cout << "User " << user << " is spamming. Message blocked." << std::endl;
	}
	else
	{
		success = true;
		// Register message
		Message info;
		info.content = message;
		info.timestamp = now;

		users[user].push_back(info);
	}

	// Remove old messages
	RemoveOldMessages(user, now);

	return success;
}

void CChatManager::ClearHistory(const std::string& user)
{
	users.erase(user);
}

bool CChatManager::InvokeSpamming(const std::string& user, const std::string& message, std::time_t now)
{
	auto& messages = users[user];

	// Check for repeated messages
	int repeatedCount = 0;
	for (auto it = messages.rbegin(); it != messages.rend(); ++it)
	{
		if (it->content == message && (now - it->timestamp) < gServerInfo.m_ChatAntiSpamTimeCheck)
		{
			++repeatedCount;
		}
		else
		{
			break;
		}
		if (repeatedCount >= gServerInfo.m_ChatAntiSpamMaxMessage - 1)
		{
			return true;
		}
	}

	return false;
}

void CChatManager::RemoveOldMessages(const std::string& user, std::time_t now)
{
	auto& messages = users[user];

	while (!messages.empty() && (now - messages.front().timestamp) > gServerInfo.m_ChatAntiSpamTimeCheck)
	{
		messages.pop_front();
	}
}
