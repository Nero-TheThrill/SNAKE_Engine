#pragma once
#include <unordered_map>

#include "vec2.hpp"
#include "Texture.h"

/**
 * @brief Per-frame UVs and metrics for sprites.
 * @details
 * - uvTopLeft / uvBottomRight: normalized UV rectangle for the frame.
 * - pixelSize: frame width/height in pixels.
 * - offset: per-frame pixel offset applied by the caller if needed.
 */
struct SpriteFrame
{
    glm::vec2 uvTopLeft;
    glm::vec2 uvBottomRight;
    glm::ivec2 pixelSize;
    glm::ivec2 offset;
};

/**
 * @brief Named clip describing a sequence of frames.
 * @details
 * - frameIndices: zero-based indices into the sheet.
 * - frameDuration: seconds per frame for this clip.
 * - looping: whether playback wraps when reaching the end.
 */
struct SpriteClip
{
    std::vector<int> frameIndices;
    float frameDuration;
    bool looping;
};

/**
 * @brief Grid-based sprite sheet built from a texture and fixed frame size.
 *
 * @details
 * - The sheet is partitioned into (columns x rows) using frameWidth/Height.
 *   Texture size is read from Texture::GetWidth()/GetHeight().
 * - UV helpers expose per-frame offset and scale for a grid cell.
 * - Clips (name -> SpriteClip) can be registered and queried.
 *
 * @note Texture* is non-owning; the texture must outlive the sheet.
 */
class SpriteSheet
{

public:
    /**
     * @brief Constructs a sheet for @p texture_ using fixed frame size.
     * @param texture_ Texture containing a grid of frames (non-owning).
     * @param frameW Frame width in pixels.
     * @param frameH Frame height in pixels.
     */
    SpriteSheet(Texture* texture_, int frameW, int frameH);

    /**
     * @brief Returns normalized UV offset for a frame index.
     * @param frameIndex Zero-based frame index in the sheet grid.
     */
    [[nodiscard]] glm::vec2 GetUVOffset(int frameIndex) const;

    /**
     * @brief Returns normalized UV scale of a single frame cell.
     * @details Derived from (frameW, frameH) and the underlying texture size.
     */
    [[nodiscard]] glm::vec2 GetUVScale() const;

    /** @brief Underlying texture (non-owning). */
    [[nodiscard]] Texture* GetTexture() const { return texture; }

    /**
     * @brief Total number of frames in the grid.
     * @details Computed from texture size and frame size.
     */
    [[nodiscard]] int GetFrameCount() const;

    /**
     * @brief Registers a named clip.
     * @param name Clip name (unique key).
     * @param frames Frame indices to play in order.
     * @param frameDuration Seconds per frame for this clip.
     * @param looping Whether the clip loops (default: true).
     */
    void AddClip(const std::string& name, const std::vector<int>& frames, float frameDuration, bool looping = true);

    /**
     * @brief Looks up a clip by name.
     * @return Pointer to the clip if found; nullptr otherwise.
     */
    [[nodiscard]] const SpriteClip* GetClip(const std::string& name) const;

private:
    std::unordered_map<std::string, SpriteClip> animationClips;
    Texture* texture;
    int frameWidth, frameHeight;
    int columns, rows;
    int texWidth = 0, texHeight = 0;

    bool flipUV_X = false;
    bool flipUV_Y = false;
};

/**
 * @brief Simple frame animator operating on a SpriteSheet.
 *
 * @details
 * - Supports playing a direct frame range or a named clip from the sheet.
 * - Update(dt) advances time and steps the current frame according to the
 *   active range's frameTime or the clip's frameDuration/looping.
 */
class SpriteAnimator
{
public:
    /**
     * @brief Constructs an animator bound to a sheet.
     * @param sheet_ Target sheet (non-owning).
     * @param frameTime_ Default seconds-per-frame for direct range playback.
     * @param loop_ Default looping flag for direct range playback.
     */
    SpriteAnimator(SpriteSheet* sheet_, float frameTime_, bool loop_ = true);

    /**
     * @brief Plays a direct frame range on the sheet.
     * @param start Start frame index.
     * @param end   End frame index.
     * @param loop_ Whether to loop the range.
     */
    void PlayClip(int start, int end, bool loop_ = true);

    /**
     * @brief Plays a named clip registered in the sheet.
     * @param clipName Name passed to SpriteSheet::GetClip().
     */
    void PlayClip(const std::string& clipName);

    /**
     * @brief Advances the animation and updates the current frame.
     * @param dt Delta time in seconds.
     */
    void Update(float dt);

    /** @brief Current frame's normalized UV offset. */
    [[nodiscard]] glm::vec2 GetUVOffset() const;
    /** @brief Current frame's normalized UV scale. */
    [[nodiscard]] glm::vec2 GetUVScale() const;

    /** @brief Convenience: returns the sheet's texture (non-owning). */
    [[nodiscard]] Texture* GetTexture() { return sheet->GetTexture(); }

    /** @brief Forces the current frame index. */
    void SetFrame(int frame) { currentFrame = frame; }
    /** @brief Returns the current frame index. */
    [[nodiscard]] int GetCurrentFrame() const { return currentFrame; }

    /** @brief Access to the bound sheet (non-owning). */
    SpriteSheet* GetSpriteSheet() const { return sheet; }

private:
    SpriteSheet* sheet;
    float frameTime;
    float elapsed = 0.0f;
    int currentFrame = 0;
    int startFrame = 0;
    int endFrame = 0;
    bool loop = true;
    const SpriteClip* playingClip = nullptr;
    int clipFrameIndex = 0;
};
