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

enum class TextAlignH
{
    Left,
    Center,
    Right
};

enum class TextAlignV
{
    Top,
    Middle,
    Bottom
};

struct Glyph
{
    glm::ivec2 size;
    glm::ivec2 bearing;
    uint32_t advance;
    glm::vec2 uvTopLeft;
    glm::vec2 uvBottomRight;
};


/**
 * @brief A font rendering class that supports UTF-8 text and dynamic glyph atlas baking.
 *
 * @details
 * The Font class encapsulates font loading, glyph caching, and mesh generation for text rendering.
 * It uses FreeType to rasterize glyphs on demand and dynamically packs them into a single texture atlas.
 *
 * Each Font instance maintains its own texture atlas and material. It also supports horizontal and vertical
 * alignment for multi-line text layout. Text meshes are typically generated internally by TextObject or
 * TextInstance, not by user code.
 *
 * This class is not constructed directly by users. Use RenderManager::RegisterFont() to register a font,
 * and RenderManager::GetFontByTag() to retrieve it.
 *
 * The font supports both ASCII and full Unicode input (including Hangul and CJK characters), and
 * dynamically expands its atlas if necessary via ExpandAtlas().
 *
 * @note
 * In most cases, you do not need to call GenerateTextMesh() directly -
 * TextObject and TextInstance internally manage mesh generation and caching.
 *
 * @code
 * // Typical usage pattern:
 * renderManager.RegisterFont("default_kr", "fonts/NotoSansKR-Regular.ttf", 32);
 * Font* font = renderManager.GetFontByTag("default_kr");
 * auto* textObj = new TextObject(&font, u8"Unicode input here", TextAlignH::Center, TextAlignV::Middle);
 * @endcode
 */
class Font
{
public:
    /**
     * @brief Constructs a Font instance by loading a TTF file and baking an initial atlas.
     *
     * @details
     * This constructor loads the specified TrueType font file using FreeType and prepares
     * an empty texture atlas for glyph storage. It also creates a default material for rendering.
     *
     * This constructor should only be called via RenderManager::RegisterFont().
     *
     * @param renderManager Reference to the RenderManager that will own and register this font.
     * @param ttfPath Path to the .ttf font file.
     * @param fontSize Pixel size of the font to render.
     */
    Font(RenderManager& renderManager, const std::string& ttfPath, uint32_t fontSize);

    /**
     * @brief Destroys the Font and releases FreeType resources.
     *
     * @details
     * Frees the FreeType face and library, and releases the internal texture and material.
     */
    ~Font();

    /**
     * @brief Returns the material used for rendering text with this font.
     *
     * @details
     * The material includes the font's atlas texture and a default shader for alpha-blended glyph rendering.
     * This material is typically used internally by TextObject during draw calls.
     *
     * @return Pointer to the font's internal Material object.
     */
    [[nodiscard]] Material* GetMaterial() const { return material.get(); }

    /**
     * @brief Measures the pixel size of a given UTF-8 string when rendered.
     *
     * @details
     * Computes the width and height of the text as it would appear with this font,
     * taking into account line breaks and glyph advances. Useful for alignment or layout purposes.
     *
     * Glyphs that have not yet been baked will be processed during this call.
     *
     * @param text UTF-8 encoded string to measure.
     * @return Size in pixels as a 2D vector (width, height).
     *
     * @note Use this to calculate bounding boxes or anchor alignment.
     */
    [[nodiscard]] glm::vec2 GetTextSize(const std::string& text) const;

    /**
     * @brief Generates a mesh for rendering a given UTF-8 string with specified alignment.
     *
     * @details
     * This function takes a UTF-8 encoded string and constructs a quad-based mesh that visually represents
     * the text using the current font's atlas. Each character is rendered as a pair of triangles (a quad),
     * using the glyph's UV coordinates and pixel size.
     *
     * Multi-line support:
     * - Line breaks ('\n') are respected and processed.
     * - Each line is measured individually, and text is aligned based on the specified 'TextAlignH' (horizontal) and
     *   'TextAlignV' (vertical) modes.
     *
     * Glyph handling:
     * - For each character in the input, the glyph is first checked in the atlas.
     * - If the glyph is missing, it is baked on-demand using TryBakeGlyph().
     * - Glyphs with no bitmap or advance are skipped or replaced with a fallback ('?') glyph.
     *
     * The resulting mesh includes vertices with positions and UVs, and indices forming triangles for each character.
     * It supports batching, alignment, and is compatible with the engine's instanced or non-instanced rendering.
     *
     * @param text UTF-8 encoded input string (e.g., u8"Hello\nWorld").
     * @param alignH Horizontal alignment mode (Left, Center, Right).
     * @param alignV Vertical alignment mode (Top, Middle, Bottom).
     * @return A newly allocated Mesh containing vertex and index data for rendering the text.
     *
     * @note
     * This function is used internally by TextObject and TextInstance.
     * You do not need to call it directly, and the returned Mesh is automatically managed
     * via unique_ptr inside a font-specific mesh cache.
     * - This function implicitly triggers glyph baking and atlas expansion when needed.
     */
    [[nodiscard]] Mesh* GenerateTextMesh(const std::string& text, TextAlignH alignH = TextAlignH::Left, TextAlignV alignV = TextAlignV::Top);

private:
    /**
     * @brief Loads a TrueType or OpenType font file using FreeType.
     *
     * @details
     * Initializes the FreeType library and loads the font face from the specified file path.
     * This function also sets the target pixel size (font height) for all glyphs in this font.
     *
     * It must be called before baking the atlas or generating glyphs.
     * If loading fails, an exception is thrown.
     *
     * @param path File path to the .ttf font file.
     * @param fontSize Font pixel height in screen space.
     *
     * @throws std::runtime_error if FreeType fails to initialize or load the font.
     */
    void LoadFont(const std::string& path, uint32_t fontSize);

    /**
     * @brief Initializes an empty texture atlas and material for this font.
     *
     * @details
     * Allocates a blank grayscale texture to be used as the initial glyph atlas.
     * It also creates a default Material and assigns the internal text shader ("internal_text").
     * This function should be called once before using the font for rendering.
     *
     * The atlas will be dynamically populated later through TryBakeGlyph().
     *
     * @param renderManager Reference to the engine's RenderManager, used for material setup.
     */
    void BakeAtlas(RenderManager& renderManager);

    /**
     * @brief Returns a reference to the baked glyph for a given Unicode codepoint.
     *
     * @details
     * Looks up the glyph in the internal glyph map. If the glyph is not found,
     * the fallback glyph ('?') is returned if available, or an empty glyph otherwise.
     *
     * This function does not bake any glyphs - use TryBakeGlyph() if needed.
     *
     * @param c Unicode codepoint to query (e.g., U'UTF-8 Character')
     * @return Reference to a cached Glyph instance.
     */
    [[nodiscard]] const Glyph& GetGlyph(char32_t c) const;

    /**
     * @brief Attempts to bake a glyph into the atlas if it is not already present.
     *
     * @details
     * This function loads the given codepoint using FreeType, rasterizes it into a bitmap,
     * and inserts it into the current atlas texture at the next available position.
     * If the atlas is full, it triggers ExpandAtlas() and retries.
     *
     * Some glyphs (e.g., spaces or control codes) may not have bitmaps but are still stored
     * if they have valid advance values.
     *
     * @param c Unicode codepoint to bake.
     * @return true if the glyph was successfully baked or already exists; false if unsupported.
     */
    [[nodiscard]] bool TryBakeGlyph(char32_t c);

    /**
     * @brief Doubles the size of the atlas texture to accommodate more glyphs.
     *
     * @details
     * Allocates a larger texture (2x current width and height), and rebinds it to the font's material.
     * Previously baked glyphs remain intact in the glyph map, but the bitmap content is not migrated.
     * After expansion, new glyphs can be baked into the new space.
     *
     * Called automatically by TryBakeGlyph() when atlas runs out of space.
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
};
