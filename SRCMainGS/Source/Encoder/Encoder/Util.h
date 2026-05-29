#pragma once

extern void BuxConvert(BYTE* pbyBuffer, int Size);
extern DWORD GenerateCheckSum2(BYTE* pbyBuffer, int dwSize, WORD Key);
extern void PackFileEncrypt(std::string filename, BYTE* pbyBuffer, int MAX_LINE, int Size);
extern void PackFileEncrypt(std::string filename, BYTE* pbyBuffer, int MAX_LINE, int Size, DWORD Key, bool WriteMax = true, bool CheckSum = true);
extern int PackFileDecrypt(std::string filename, BYTE** pbyBuffer, int MAX_LINE, int Size, DWORD Key = 0);


extern void RemoveQuotes(std::wstring& str);
extern std::string WStringToUTF8(const std::wstring& wstr);




template<typename T>
inline void writeVector(FILE* fp, const std::vector<T>& f)
{
	DWORD MaxLine = f.size();
	fwrite(&MaxLine, sizeof(MaxLine), 1, fp);  // Escribe el tamaño (0 si está vacío)

	if (!f.empty())
	{
		BuxConvert((BYTE*)f.data(), (MaxLine * sizeof(T)));
		fwrite(f.data(), sizeof(T), MaxLine, fp);  // Solo escribe si hay datos
	}
}