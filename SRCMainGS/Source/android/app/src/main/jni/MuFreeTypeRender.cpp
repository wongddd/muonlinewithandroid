#include "MuFreeTypeRender.h"

#ifdef ANDROID

#include <android/log.h>
#include <cstring>
#include <cmath>

#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "GlobalBitmap.h"
#include "_TextureIndex.h"

#define LOG_TAG "MuFreeTypeRender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Default font path on Android system — can be overridden
static const char* DEFAULT_FONT_PATH = "/system/fonts/DroidSans.ttf";

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

// ============================================================================
// CUIRenderTextFT construction / destruction
// ============================================================================

CUIRenderTextFT::CUIRenderTextFT()
    : m_ftLibrary(nullptr)
    , m_ftFace(nullptr)
    , m_dwTextColor(0xFFFFFFFF)
    , m_dwBackColor(0x00000000)
    , m_fontSize(14)
    , m_textureId(0)
    , m_texWidth(0)
    , m_texHeight(0)
{
}

CUIRenderTextFT::~CUIRenderTextFT()
{
    Release();
}

// ============================================================================
// Create / Release
// ============================================================================

bool CUIRenderTextFT::Create(HDC hDC)
{
    if (FT_Init_FreeType(&m_ftLibrary)) {
        LOGE("FT_Init_FreeType failed");
        return false;
    }

    if (FT_New_Face(m_ftLibrary, DEFAULT_FONT_PATH, 0, &m_ftFace)) {
        // Try bold variant
        if (FT_New_Face(m_ftLibrary, "/system/fonts/DroidSans-Bold.ttf", 0, &m_ftFace)) {
            LOGE("FT_New_Face failed for %s", DEFAULT_FONT_PATH);
            FT_Done_FreeType(m_ftLibrary);
            m_ftLibrary = nullptr;
            return false;
        }
    }

    FT_Set_Pixel_Sizes(m_ftFace, 0, m_fontSize);

    LOGI("FreeType renderer created: font=%s, size=%d", DEFAULT_FONT_PATH, m_fontSize);
    return true;
}

void CUIRenderTextFT::Release()
{
    if (m_ftFace) {
        FT_Done_Face(m_ftFace);
        m_ftFace = nullptr;
    }
    if (m_ftLibrary) {
        FT_Done_FreeType(m_ftLibrary);
        m_ftLibrary = nullptr;
    }
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
}

// ============================================================================
// Color setters
// ============================================================================

void CUIRenderTextFT::SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    m_dwTextColor = (byAlpha << 24) | (byBlue << 16) | (byGreen << 8) | byRed;
}

void CUIRenderTextFT::SetTextColor(DWORD dwColor)
{
    m_dwTextColor = dwColor;
}

void CUIRenderTextFT::SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    m_dwBackColor = (byAlpha << 24) | (byBlue << 16) | (byGreen << 8) | byRed;
}

void CUIRenderTextFT::SetBgColor(DWORD dwColor)
{
    m_dwBackColor = dwColor;
}

void CUIRenderTextFT::SetFont(HFONT hFont)
{
    // On Android, HFONT is nullptr/stub. We use a single FreeType face.
    // Bold could be handled by setting FT_Set_Pixel_Sizes with a different face.
    (void)hFont;
}

// ============================================================================
// Texture helpers
// ============================================================================

void CUIRenderTextFT::ensureTexture(int w, int h)
{
    if (w <= m_texWidth && h <= m_texHeight) return;

    int nw = std::max(w, m_texWidth);
    int nh = std::max(h, m_texHeight);
    // Grow to next power of two
    nw = 1 << static_cast<int>(std::ceil(std::log2(nw)));
    nh = 1 << static_cast<int>(std::ceil(std::log2(nh)));
    nw = std::max(nw, 64);
    nh = std::max(nh, 64);

    m_texBuffer.resize(nw * nh * 4, 0);
    m_texWidth  = nw;
    m_texHeight = nh;

    if (m_textureId == 0) {
        glGenTextures(1, &m_textureId);
    }
}

void CUIRenderTextFT::renderGlyphToBuffer(FT_Bitmap* bitmap, int penX, int penY, int bufW, int bufH, uint8_t* buf)
{
    uint8_t r = static_cast<uint8_t>(m_dwTextColor & 0xFF);
    uint8_t g = static_cast<uint8_t>((m_dwTextColor >> 8) & 0xFF);
    uint8_t b = static_cast<uint8_t>((m_dwTextColor >> 16) & 0xFF);

    uint8_t bgr = static_cast<uint8_t>(m_dwBackColor & 0xFF);
    uint8_t bgg = static_cast<uint8_t>((m_dwBackColor >> 8) & 0xFF);
    uint8_t bgb = static_cast<uint8_t>((m_dwBackColor >> 16) & 0xFF);

    for (unsigned int row = 0; row < bitmap->rows; row++) {
        int dstY = penY + row;
        if (dstY < 0 || dstY >= bufH) continue;

        for (unsigned int col = 0; col < bitmap->width; col++) {
            int dstX = penX + col;
            if (dstX < 0 || dstX >= bufW) continue;

            unsigned char alpha = 0;
            if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
                alpha = bitmap->buffer[row * bitmap->pitch + col];
            } else if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
                alpha = (bitmap->buffer[row * bitmap->pitch + (col >> 3)] & (0x80 >> (col & 7))) ? 255 : 0;
            }

            int idx = (dstY * bufW + dstX) * 4;
            // Blend: glyph alpha over background
            float fa = alpha / 255.0f;
            buf[idx + 0] = static_cast<uint8_t>(r * fa + bgr * (1.0f - fa));
            buf[idx + 1] = static_cast<uint8_t>(g * fa + bgg * (1.0f - fa));
            buf[idx + 2] = static_cast<uint8_t>(b * fa + bgb * (1.0f - fa));
            buf[idx + 3] = alpha; // Use alpha based on glyph coverage
        }
    }
}

// ============================================================================
// Measure text dimensions in pixels using FreeType
// ============================================================================

static void measureTextFT(FT_Face face, const char* text, int* outWidth, int* outHeight)
{
    int w = 0, h = 0;
    int penX = 0;
    FT_UInt prevIdx = 0;
    bool useKerning = FT_HAS_KERNING(face);

    for (const char* p = text; *p; p++) {
        FT_UInt glyphIdx = FT_Get_Char_Index(face, static_cast<FT_ULong>(static_cast<unsigned char>(*p)));

        if (useKerning && prevIdx && glyphIdx) {
            FT_Vector delta;
            FT_Get_Kerning(face, prevIdx, glyphIdx, FT_KERNING_DEFAULT, &delta);
            penX += delta.x >> 6;
        }

        if (!FT_Load_Glyph(face, glyphIdx, FT_LOAD_DEFAULT)) {
            FT_GlyphSlot slot = face->glyph;
            w = penX + slot->bitmap_left + static_cast<int>(slot->bitmap.width);
            int glyphH = static_cast<int>(slot->bitmap.rows);
            if (glyphH > h) h = glyphH;
            penX += slot->advance.x >> 6;
        } else {
            penX += face->size->metrics.x_ppem / 2; // fallback for missing glyphs
        }

        prevIdx = glyphIdx;
    }

    *outWidth  = w;
    *outHeight = h ? h : face->size->metrics.y_ppem;
}

// ============================================================================
// RenderText — render string with FreeType → OpenGL texture → RenderBitmap
// ============================================================================

void CUIRenderTextFT::RenderText(int iPos_x, int iPos_y, const char* pszText,
    int iBoxWidth, int iBoxHeight, int iSort, OUT SIZE* lpTextSize, bool black_out)
{
    if (!pszText || !pszText[0] || !m_ftFace) return;

    // Measure text
    int textW, textH;
    measureTextFT(m_ftFace, pszText, &textW, &textH);

    if (lpTextSize) {
        lpTextSize->cx = static_cast<long>(textW / g_fScreenRate_x);
        lpTextSize->cy = static_cast<long>(textH / g_fScreenRate_y);
    }

    // Apply screen rate scaling
    int posX = static_cast<int>(iPos_x * g_fScreenRate_x);
    int posY = static_cast<int>(iPos_y * g_fScreenRate_y);
    int boxW = static_cast<int>(iBoxWidth * g_fScreenRate_x);
    int boxH = static_cast<int>(iBoxHeight * g_fScreenRate_y);

    if (boxW == 0) boxW = textW;
    if (boxH == 0) boxH = textH;

    // Calculate alignment offset
    int alignX = 0;
    if (iSort == RT3_SORT_CENTER) {
        alignX = (boxW - textW) / 2;
    } else if (iSort == RT3_SORT_RIGHT) {
        alignX = boxW - textW;
    }

    // Ensure texture buffer
    ensureTexture(textW, textH);

    // Clear buffer with background color
    uint8_t bgr = static_cast<uint8_t>(m_dwBackColor & 0xFF);
    uint8_t bgg = static_cast<uint8_t>((m_dwBackColor >> 8) & 0xFF);
    uint8_t bgb = static_cast<uint8_t>((m_dwBackColor >> 16) & 0xFF);
    uint8_t bga = static_cast<uint8_t>((m_dwBackColor >> 24) & 0xFF);

    int bufSize = m_texWidth * m_texHeight * 4;
    for (int i = 0; i < bufSize; i += 4) {
        m_texBuffer[i + 0] = bgr;
        m_texBuffer[i + 1] = bgg;
        m_texBuffer[i + 2] = bgb;
        m_texBuffer[i + 3] = bga;
    }

    // Render each glyph into buffer
    int penX = 0, penY = 0;
    FT_UInt prevIdx = 0;
    bool useKerning = FT_HAS_KERNING(m_ftFace);

    for (const char* p = pszText; *p; p++) {
        FT_UInt glyphIdx = FT_Get_Char_Index(m_ftFace, static_cast<FT_ULong>(static_cast<unsigned char>(*p)));

        if (useKerning && prevIdx && glyphIdx) {
            FT_Vector delta;
            FT_Get_Kerning(m_ftFace, prevIdx, glyphIdx, FT_KERNING_DEFAULT, &delta);
            penX += delta.x >> 6;
        }

        if (!FT_Load_Glyph(m_ftFace, glyphIdx, FT_LOAD_DEFAULT)) {
            FT_Render_Glyph(m_ftFace->glyph, FT_RENDER_MODE_NORMAL);
            FT_Bitmap* bitmap = &m_ftFace->glyph->bitmap;
            FT_GlyphSlot slot = m_ftFace->glyph;

            int baselineY = m_ftFace->size->metrics.ascender >> 6;
            int glyphY = baselineY - slot->bitmap_top;

            renderGlyphToBuffer(bitmap, penX + slot->bitmap_left, glyphY,
                m_texWidth, m_texHeight, m_texBuffer.data());

            penX += slot->advance.x >> 6;
        } else {
            penX += m_ftFace->size->metrics.x_ppem / 2;
        }

        prevIdx = glyphIdx;
    }

    // Upload to OpenGL and render
    BITMAP_t* b = &Bitmaps[BITMAP_FONT];
    if (b && b->TextureNumber) {
        glBindTexture(GL_TEXTURE_2D, b->TextureNumber);
        // Only update the portion we used
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texWidth, m_texHeight,
            0, GL_RGBA, GL_UNSIGNED_BYTE, m_texBuffer.data());

        float uW = static_cast<float>(textW) / m_texWidth;
        float vH = static_cast<float>(textH) / m_texHeight;

        // Clamp to screen bounds
        int renderW = textW;
        int renderX = posX + alignX;
        float texU = 0.f;

        if (renderX < 0) {
            texU = static_cast<float>(-renderX) / m_texWidth;
            renderW += renderX;
            renderX = 0;
        }
        if (renderX + renderW > WindowWidth) {
            renderW = WindowWidth - renderX;
        }

        int renderH = textH;
        int renderY = posY;
        float texV = 0.f;

        if (renderY < 0) {
            texV = static_cast<float>(-renderY) / m_texHeight;
            renderH += renderY;
            renderY = 0;
        }
        if (renderY + renderH > WindowHeight) {
            renderH = WindowHeight - renderY;
        }

        if (renderW > 0 && renderH > 0) {
            if (black_out) {
                GLfloat curColor[4];
                glGetFloatv(GL_CURRENT_COLOR, curColor);
                glColor4f(0.f, 0.f, 0.f, 1.f);
                RenderBitmap(BITMAP_FONT, static_cast<float>(renderX) + 1.f,
                    static_cast<float>(renderY) + 1.f,
                    static_cast<float>(renderW), static_cast<float>(renderH),
                    texU, texV, texU + uW, texV + vH, false, false);
                glColor4fv(curColor);
            }
            RenderBitmap(BITMAP_FONT, static_cast<float>(renderX), static_cast<float>(renderY),
                static_cast<float>(renderW), static_cast<float>(renderH),
                texU, texV, texU + uW, texV + vH, false, false);
        }
    }
}

// ============================================================================
// RenderFont — same as RenderText but uses the current font
// ============================================================================

void CUIRenderTextFT::RenderFont(int iPos_x, int iPos_y, const char* pszText,
    int iBoxWidth, int iBoxHeight, int iSort, OUT SIZE* lpTextSize)
{
    RenderText(iPos_x, iPos_y, pszText, iBoxWidth, iBoxHeight, iSort, lpTextSize, false);
}

// ============================================================================
// RenderTextClipped — render with scissor clipping
// ============================================================================

void CUIRenderTextFT::RenderTextClipped(float iPos_x, float iPos_y, const char* pszText,
    int iBoxWidth, int iBoxHeight, int iSort, OUT SIZE* lpTextSize)
{
    if (!pszText || !pszText[0] || !m_ftFace) return;

    // Apply scissor for clipping
    int clipX = static_cast<int>(iPos_x * g_fScreenRate_x);
    int clipY = static_cast<int>(iPos_y * g_fScreenRate_y);
    int clipW = static_cast<int>(iBoxWidth * g_fScreenRate_x);
    int clipH = static_cast<int>(iBoxHeight * g_fScreenRate_y);

    if (clipW <= 0 || clipH <= 0) return;

    // Convert to OpenGL window coordinates (Y-flip)
    GLint glY = WindowHeight - (clipY + clipH);

    glEnable(GL_SCISSOR_TEST);
    glScissor(clipX, glY, clipW, clipH);

    RenderText(static_cast<int>(iPos_x), static_cast<int>(iPos_y),
        pszText, iBoxWidth, iBoxHeight, iSort, lpTextSize, false);

    glDisable(GL_SCISSOR_TEST);
}

#endif // ANDROID
