#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "glm.hpp"
#include "ft2build.h"
#include FT_FREETYPE_H

#include "Texture.h"
#include "Material.h"

class Camera2D;


struct EngineContext;

/** @brief Horizontal text alignment used when building meshes. */
enum class TextAlignH
{
    Left,   ///< Align line start at x = 0 (default).
    Center, ///< Center each line around x = 0.
    Right   ///< Align line end at x = 0.
};

/** @brief Vertical text alignment used when building meshes. */
enum class TextAlignV
{
    Top,    ///< First line starts at top; y offset decreases by font size per line.
    Middle, ///< Block centered vertically around y = 0.
    Bottom  ///< Last line ends at y = 0.
};

/**
 * @brief Glyph metrics and atlas UVs.
 * @details
 * - size: bitmap width/height in pixels.
 * - bearing: (left, top) offset from baseline in pixels.
 * - advance: 26.6 fixed-point advance (use advance >> 6 for pixels).
 * - uvTopLeft / uvBottomRight: normalized UVs in the font atlas.
 */
struct Glyph
{
    glm::ivec2 size;
    glm::ivec2 bearing;
    uint32_t advance;
    glm::vec2 uvTopLeft;
    glm::vec2 uvBottomRight;
};

/**
 * @brief FreeType-based bitmap font with dynamic atlas and text mesh generation.
 *
 * @details
 * - Constructor loads a TTF via FreeType and creates a single-channel (R) atlas texture,
 *   then initializes a Material using shader tag "[EngineShader]internal_text".
 *   The material binds the atlas as "u_FontTexture" and sets default "u_Color" = (1,1,1,1).
 * - Atlas starts at 128x128 and expands by doubling; all baked glyphs are re-packed on expand.
 * - Text input is parsed as UTF-8; per-glyph advances are used (no kerning).
 * - Newlines ('\n') start a new line; overall size = (max line width, fontSize x lineCount).
 */
class Font
{
public:
    /**
     * @brief Loads the font and builds the initial atlas/material.
     * @param engineContext RenderManager reference (used to fetch the internal text shader).
     * @param ttfPath Path to a .ttf file.
     * @param fontSize Pixel size passed to FreeType (FT_Set_Pixel_Sizes).
     */
    Font(RenderManager& engineContext, const std::string& ttfPath, uint32_t fontSize);

    /**
     * @brief Releases FreeType resources (FT_Face / FT_Library).
     */
    ~Font();

    /**
     * @brief Material used for rendering this font.
     * @details
     * Binds "u_FontTexture" to the atlas texture and exposes "u_Color".
     * Returned pointer is non-owning.
     */
    [[nodiscard]] Material* GetMaterial() const { return material.get(); }

    /**
     * @brief Computes text block size for UTF-8 input.
     * @details
     * - Splits on '\n' to count lines.
     * - Width is the maximum sum of (advance >> 6) per line.
     * - Height = fontSize x line count.
     * @return (width, height) in pixels.
     */
    [[nodiscard]] glm::vec2 GetTextSize(const std::string& text) const;

    /**
     * @brief Builds a Mesh containing quads for the given UTF-8 text.
     *
     * @details
     * - Pre-bakes required glyphs into the atlas on demand.
     * - Horizontal alignment per line:
     *   Left -> x=0, Center -> x=?lineWidth/2, Right -> x=?lineWidth.
     * - Vertical alignment of the block:
     *   Top -> yStart -= fontSize, Middle -> yStart += totalHeight*0.5 ? lineSpacing,
     *   Bottom -> yStart += totalHeight ? lineSpacing, with lineSpacing = fontSize.
     * - Vertex positions use glyph bearing/size; UVs come from the atlas.
     *
     * @param text UTF-8 text.
     * @param alignH Horizontal alignment per line.
     * @param alignV Vertical alignment of the whole block.
     * @return Newly allocated Mesh*.
     */
    [[nodiscard]] Mesh* GenerateTextMesh(const std::string& text, TextAlignH alignH = TextAlignH::Left, TextAlignV alignV = TextAlignV::Top);

    /**
     * @brief Monotonically increasing atlas version.
     * @details Increments whenever the atlas expands/rebakes. Useful to detect when meshes need rebuild.
     */
    int GetTextAtlasVersion() { return atlasVersion; }

private:
    /**
     * @brief INTERNAL: Initializes FreeType (FT_Init_FreeType / FT_New_Face / FT_Set_Pixel_Sizes).
     */
    void LoadFont(const std::string& path, uint32_t fontSize);

    /**
     * @brief INTERNAL: Creates the initial 128x128 R-channel atlas and the text Material.
     * @details Binds "u_FontTexture" and sets "u_Color" to white.
     */
    void BakeAtlas(RenderManager& renderManager);

    /**
     * @brief INTERNAL: Gets a baked glyph or a fallback.
     * @details Returns '?' if the glyph is missing; otherwise returns an empty static Glyph.
     */
    [[nodiscard]] const Glyph& GetGlyph(char32_t c) const;

    /**
     * @brief INTERNAL: Bakes a single codepoint into the atlas if not present.
     * @details
     * - Uses FT_Load_Char(..., FT_LOAD_RENDER).
     * - Reserves 1-pixel padding; writes bitmap via glTextureSubImage2D.
     * - Expands the atlas and retries if it doesn't fit.
     * @return true on success (or already baked); false on load failure.
     */
    [[nodiscard]] bool TryBakeGlyph(char32_t c);

    /**
     * @brief INTERNAL: Doubles atlas size and re-bakes all previously baked glyphs.
     * @details Updates the material's "u_FontTexture" and increments atlasVersion.
     */
    void ExpandAtlas();

    FT_Library ft;
    FT_Face face;

    uint32_t fontSize;

    std::unordered_map<char32_t, Glyph> glyphs;
    std::unique_ptr<Texture> atlasTexture;
    std::unique_ptr<Material> material;

    int nextX = 0;
    int nextY = 0;
    int maxRowHeight = 0;

    int atlasVersion = 0;
};
