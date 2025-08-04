#pragma once
#include <unordered_map>

#include "vec2.hpp"
#include "Texture.h"
struct SpriteFrame
{
    glm::vec2 uvTopLeft;
    glm::vec2 uvBottomRight;
    glm::ivec2 pixelSize;
    glm::ivec2 offset;
};

struct SpriteClip
{
    std::vector<int> frameIndices;
    float frameDuration;
    bool looping;
};

/**
 * @brief Represents a texture atlas subdivided into animation frames.
 *
 * @details
 * SpriteSheet stores frame layout information for a texture that is
 * subdivided into equal-sized animation frames (grid-based).
 * It computes per-frame UV coordinates and supports the definition of named animation clips.
 *
 * This class is responsible for providing UV offset and scale for each frame,
 * which are consumed by SpriteAnimator during playback.
 * Clips can be defined using AddClip() and retrieved by name.
 *
 * @note Used primarily for 2D sprite animation via instancing or mesh-based rendering.
 */
class SpriteSheet
{

public:
    /**
     * @brief Constructs a sprite sheet from a texture by dividing it into grid cells.
     *
     * @details
     * Given a texture and frame width/height, this constructor divides the texture
     * into equally-sized cells (tiles), and precomputes the number of rows and columns.
     * These are later used to compute UV coordinates per frame.
     *
     * @param texture_ Pointer to the texture to use as a sprite sheet.
     * @param frameW Width in pixels of each frame.
     * @param frameH Height in pixels of each frame.
     */
    SpriteSheet(Texture* texture_, int frameW, int frameH);

    /**
     * @brief Returns the UV offset (top-left) of a specific frame.
     *
     * @details
     * Calculates the UV starting coordinate of the given frame index,
     * based on the row/column layout of the sprite sheet.
     *
     * If the index exceeds available frames, behavior is undefined.
     *
     * @param frameIndex Index of the frame to retrieve.
     * @return UV offset as glm::vec2 (top-left corner).
     */
    [[nodiscard]] glm::vec2 GetUVOffset(int frameIndex) const;

    /**
     * @brief Returns the uniform UV scale for all frames in the sprite sheet.
     *
     * @details
     * Since all frames are same-sized tiles, the scale is constant and
     * computed as (frameWidth / textureWidth, frameHeight / textureHeight).
     *
     * @return UV scale (width and height) as glm::vec2.
     */
    [[nodiscard]] glm::vec2 GetUVScale() const;

    /**
     * @brief Returns the uniform UV scale for all frames in the sprite sheet.
     *
     * @details
     * Since all frames are same-sized tiles, the scale is constant and
     * computed as (frameWidth / textureWidth, frameHeight / textureHeight).
     *
     * @return UV scale (width and height) as glm::vec2.
     */
    [[nodiscard]] Texture* GetTexture() const { return texture; }

    /**
     * @brief Returns the total number of frames in the sprite sheet.
     *
     * @details
     * Computed as (columns * rows), based on frame size and texture size.
     *
     * @return Total frame count.
     */
    [[nodiscard]] int GetFrameCount() const;

    /**
     * @brief Registers a named animation clip for later playback.
     *
     * @details
     * Adds a new SpriteClip entry mapped to the given name.
     * Each clip consists of an ordered list of frame indices, frame duration, and loop flag.
     * If a clip with the same name already exists, it will be overwritten.
     *
     * @param name Unique name to identify the clip.
     * @param frames Ordered list of frame indices used by this clip.
     * @param frameDuration Time per frame (in seconds).
     * @param looping Whether the clip should loop automatically.
     */
    void AddClip(const std::string& name, const std::vector<int>& frames, float frameDuration, bool looping=true);

    /**
     * @brief Retrieves a previously added named animation clip.
     *
     * @details
     * Looks up the animationClips map by name and returns a pointer to the SpriteClip.
     * If the name does not exist, returns nullptr.
     *
     * @param name The name of the clip to retrieve.
     * @return Pointer to the SpriteClip, or nullptr if not found.
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
 * @brief Controls playback of sprite animations using a SpriteSheet.
 *
 * @details
 * SpriteAnimator handles frame-by-frame animation using either:
 * - A direct frame range (via PlayClip(start, end))
 * - A named clip from the SpriteSheet (via PlayClip("clipName"))
 *
 * On each Update(dt), it advances the animation timer and determines the current frame.
 * The UV offset and scale are updated per frame and can be queried for rendering.
 *
 * This class does not directly render anything; it simply manages which part
 * of the sprite sheet should be shown and for how long.
 *
 * @note Usually attached to animated objects (e.g. GameObject or TextObject).
 */
class SpriteAnimator
{
public:
    /**
     * @brief Constructs a sprite animator with a sheet and frame timing.
     *
     * @details
     * Initializes animation playback using the provided SpriteSheet and per-frame time.
     * By default, playback loops between frames unless loop is disabled.
     *
     * No clip or range is active until PlayClip() is explicitly called.
     *
     * @param sheet_ Pointer to the SpriteSheet used for frame UV data.
     * @param frameTime_ Duration (in seconds) to display each frame.
     * @param loop_ Whether to loop playback after reaching the end.
     */
    SpriteAnimator(SpriteSheet* sheet_, float frameTime_, bool loop_ = true);

    /**
     * @brief Plays a custom animation clip defined by frame range.
     *
     * @details
     * This method allows you to manually specify a start and end frame index.
     * The animator will cycle from startFrame to endFrame at the configured frame rate.
     *
     * Looping behavior is determined by the loop_ parameter.
     * This method overrides any currently playing named clip.
     *
     * @param start Starting frame index (inclusive).
     * @param end Ending frame index (inclusive).
     * @param loop_ Whether to loop the animation after reaching the end.
     */
    void PlayClip(int start, int end, bool loop_ = true);

    /**
     * @brief Plays a named animation clip from the SpriteSheet.
     *
     * @details
     * Looks up the specified clip in the SpriteSheet's registered clips.
     * If found, the animator begins playback using the clip's frame list and duration.
     * Looping is determined by the clip's 'looping' flag.
     *
     * If the clip is not found, no change is made.
     *
     * @param clipName The name of the registered clip to play.
     */
    void PlayClip(const std::string& clipName);

    /**
     * @brief Advances the animation timer and updates the current frame.
     *
     * @details
     * Increments the internal animation timer by dt.
     * If the elapsed time exceeds the configured frame duration,
     * the current frame is advanced by one.
     *
     * - If using a named clip: advances through clip.frameIndices[]
     * - If using direct range: increments from startFrame to endFrame
     *
     * When the end is reached:
     * - If looping: wraps to start
     * - If not: remains on last frame
     *
     * @param dt Time in seconds since last update call.
     */
    void Update(float dt);

    /**
     * @brief Returns the UV offset of the currently active frame.
     *
     * @details
     * Queries the SpriteSheet for the UV offset of the currently playing frame index.
     * Used during rendering to determine where to sample the texture.
     *
     * @return UV top-left offset as glm::vec2.
     */
    [[nodiscard]] glm::vec2 GetUVOffset() const;

    /**
     * @brief Returns the uniform UV scale used for all frames.
     *
     * @details
     * Each frame in the sprite sheet has the same width and height,
     * so the UV scale remains constant and is derived from frame size vs texture size.
     *
     * @return UV scale as glm::vec2.
     */
    [[nodiscard]] glm::vec2 GetUVScale() const;

    /**
     * @brief Returns the texture associated with the underlying SpriteSheet.
     *
     * @return Pointer to the texture object used for sprite rendering.
     */
    [[nodiscard]] Texture* GetTexture() { return sheet->GetTexture();}

    /**
     * @brief Directly sets the current animation frame index.
     *
     * @param frame The new frame index to use.
     */
    void SetFrame(int frame) { currentFrame = frame; }

    /**
     * @brief Returns the current animation frame index.
     *
     * @return Index of the current frame being displayed.
     */
    [[nodiscard]] int GetCurrentFrame() const { return currentFrame; }

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
