#include "stdafx.h"
#include "Util.h"
#include "ConsoleDebug.h"
#include "WindowsConsole.h"

CMuConsoleDebug::CMuConsoleDebug() : m_bInit(false)
{
#ifdef CONSOLE
	if (leaf::OpenConsoleWindow("Mu Debug Console Window"))
	{
		leaf::ActivateCloseButton(false);
		leaf::ShowConsole(true);
		m_bInit = true;
	}
#endif
}

CMuConsoleDebug::~CMuConsoleDebug()
{
#ifdef CONSOLE
	leaf::CloseConsoleWindow();
#endif
}

CMuConsoleDebug* CMuConsoleDebug::sInstance()
{
	static CMuConsoleDebug pInstance;
	return &pInstance;
}

bool CMuConsoleDebug::CheckCommand(const std::string& strCommand)
{
	if (!m_bInit)
		return false;
#ifdef CONSOLE
	if (strCommand.compare("$open") == NULL)
	{
		leaf::ShowConsole(true);
		return true;
	}
	else if (strCommand.compare("$close") == NULL)
	{
		leaf::ShowConsole(false);
		return true;
	}
	else if (strCommand.compare("$clear") == NULL)
	{
		leaf::SetConsoleTextColor();
		leaf::ClearConsoleScreen();
		return true;
	}
	else if (strCommand.compare("$type_test") == NULL)
	{
		LogMessage(MCD_SEND, "MCD_SEND");
		LogMessage(MCD_RECEIVE, "MCD_RECEIVE");
		LogMessage(MCD_ERROR, "MCD_ERROR");
		LogMessage(MCD_NORMAL, "MCD_NORMAL");
		return true;
	}
	else if (strCommand.compare("$texture_info") == NULL)
	{
		//LogMessage(MCD_NORMAL, "Texture Number : %d", Bitmaps.GetNumberOfTexture());
		//LogMessage(MCD_NORMAL, "Texture Memory : %dKB", Bitmaps.GetUsedTextureMemory() / 1024);
		return true;
	}
	else if (strCommand.compare("$color_test") == NULL)
	{
		leaf::SetConsoleTextColor(leaf::COLOR_DARKRED);
		std::cout << "color test: dark red" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_DARKGREEN);
		std::cout << "color test: dark green" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_DARKBLUE);
		std::cout << "color test: dark blue" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_RED);
		std::cout << "color test: red" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_GREEN);
		std::cout << "color test: green" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_BLUE);
		std::cout << "color test: blue" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_OLIVE);
		std::cout << "color test: olive" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_PURPLE);
		std::cout << "color test: purple" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_TEAL);
		std::cout << "color test: teal" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_GRAY);
		std::cout << "color test: gray" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_AQUA);
		std::cout << "color test: aqua" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_FUCHSIA);
		std::cout << "color test: fuchsia" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_YELLOW);
		std::cout << "color test: yellow" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_WHITE);
		std::cout << "color test: white" << std::endl;
		return true;
	}
#endif
	return false;
}

void CMuConsoleDebug::LogMessage(int iType, const char* pStr, ...)
{
#ifdef CONSOLE
	if (m_bInit)
	{
		switch (iType)
		{
		case MCD_SEND:
			leaf::SetConsoleTextColor(leaf::COLOR_OLIVE);
			break;
		case MCD_RECEIVE:
			leaf::SetConsoleTextColor(leaf::COLOR_DARKGREEN);
			break;
		case MCD_ERROR:
			leaf::SetConsoleTextColor(leaf::COLOR_WHITE, leaf::COLOR_DARKRED);
			break;
		case MCD_NORMAL:
			leaf::SetConsoleTextColor(leaf::COLOR_GRAY);
			break;
		default:
			leaf::SetConsoleTextColor(leaf::COLOR_YELLOW);
			break;
		}
		char szBuffer[256] = "";
		va_list	pArguments;

		va_start(pArguments, pStr);
		vsprintf(szBuffer, pStr, pArguments);
		va_end(pArguments);
		std::cout << szBuffer << std::endl;
	}
#endif
}