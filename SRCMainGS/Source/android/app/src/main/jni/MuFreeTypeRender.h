#pragma once

#include "stdafx_port.h"

#include <cstdint>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "UIControls.h"

#ifdef ANDROID

class CUIRenderTextFT : public IUIRenderText
{
    FT_Library  m_ftLibrary;
    FT_Face     m_ftFace;
    DWORD       m_dwTextColor;
    DWORD       m_dwBackColor;
    int         m_fontSize;

    GLuint      m_textureId;
    int         m_texWidth;
    int         m_texHeight;
    std::vector<uint8_t> m_texBuffer;

    void ensureTexture(int w, int h);
    void renderGlyphToBuffer(FT_Bitmap* bitmap, int penX, int penY, int bufW, int bufH, uint8_t* buf);

public:
    CUIRenderTextFT();
    virtual ~CUIRenderTextFT();

    bool Create(HDC hDC);
    void Release();

    HDC        GetFontDC() const    { return nullptr; }
    BYTE*      GetFontBuffer() const { return nullptr; }
    DWORD      GetTextColor() const  { return m_dwTextColor; }
    DWORD      GetBgColor() const    { return m_dwBackColor; }

    void SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetTextColor(DWORD dwColor);
    void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetBgColor(DWORD dwColor);

    void SetFont(HFONT hFont);

    void RenderText(int iPos_x, int iPos_y, const char* pszText,
        int iBoxWidth = 0, int iBoxHeight = 0, int iSort = RT3_SORT_LEFT,
        OUT SIZE* lpTextSize = NULL, bool black_out = false);
    void RenderFont(int iPos_x, int iPos_y, const char* pszText,
        int iBoxWidth = 0, int iBoxHeight = 0, int iSort = RT3_SORT_LEFT,
        OUT SIZE* lpTextSize = NULL);
    void RenderTextClipped(float iPos_x, float iPos_y, const char* pszText,
        int iBoxWidth = 0, int iBoxHeight = 0, int iSort = RT3_SORT_LEFT,
        OUT SIZE* lpTextSize = NULL);
};

#endif // ANDROID
