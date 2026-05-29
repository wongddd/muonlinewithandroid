#pragma once
class CMultiLanguage
{

private:
	

	BYTE byLanguage;
	int iCodePage;
	int iNumByteForOneCharUTF8;

public:
	CMultiLanguage()
	{
		byLanguage = 0;
		iCodePage = CP_UTF8;
		iNumByteForOneCharUTF8 = 2;
	};
	~CMultiLanguage()
	{
	};

	BYTE GetLanguage();				// Getters
	int GetCodePage();
	int GetNumByteForOneCharUTF8();

	BOOL IsCharUTF8(const char* pszText);
	int	 ConvertCharToWideStr(std::wstring& wstrDest, LPCSTR lpString);
	int  ConvertWideCharToStr(std::string& strDest, LPCWSTR lpwString, int iConversionType = CP_UTF8);
	void ConvertANSIToUTF8OrViceVersa(std::string& strDest, LPCSTR lpString);
	int	 GetClosestBlankFromCenter(const std::wstring wstrTarget);

	WPARAM ConvertFulltoHalfWidthChar(DWORD wParam);
	BOOL _GetTextExtentPoint32(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize);
	BOOL _GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize);

	BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString);
	BOOL _TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString);

	static CMultiLanguage* GetSingletonPtr()
	{
		static CMultiLanguage ms_Singleton;
		return &ms_Singleton;
	};
};


#define g_pMultiLanguage			(CMultiLanguage::GetSingletonPtr())