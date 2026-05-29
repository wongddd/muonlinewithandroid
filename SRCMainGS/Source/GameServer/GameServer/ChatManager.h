#pragma once
#include <ctime>
#include <unordered_map>

#define messageLimit	3
#define timeLimit		10 // Time limit between messages in seconds


class CChatManager
{
	struct Message
	{
		std::string content;
		std::time_t timestamp;
	};
public:
	static CChatManager* Instance();
		bool InvokeMessage(const std::string& user, const std::string& message);
	void ClearHistory(const std::string& user);
private:
	std::unordered_map<std::string, std::deque<Message>> users;

	bool InvokeSpamming(const std::string& user, const std::string& message, std::time_t now);
	void RemoveOldMessages(const std::string& user, std::time_t now);
};

#define GMAntiSpam				(CChatManager::Instance())