#include "Animation.h"

SpriteSheet::SpriteSheet(Texture* texture_, int frameW, int frameH)
    : texture(texture_), frameWidth(frameW), frameHeight(frameH)
{
    texWidth = texture_->GetWidth();
    texHeight = texture_->GetHeight();
    columns = texWidth / frameWidth;
    rows = texHeight / frameHeight;
}

glm::vec2 SpriteSheet::GetUVOffset(int frameIndex) const
{
    int col = frameIndex % columns;
    int row = frameIndex / columns;
    int flippedRow = (rows - 1) - row;
    float u = static_cast<float>(col * frameWidth) / texWidth;
    float v = static_cast<float>(flippedRow * frameHeight) / texHeight;
    return { u, v };
}

glm::vec2 SpriteSheet::GetUVScale() const
{
    return {
        static_cast<float>(frameWidth) / texWidth,
        static_cast<float>(frameHeight) / texHeight
    };
}

int SpriteSheet::GetFrameCount() const
{
    return columns * rows;
}

void SpriteSheet::AddClip(const std::string& name, const std::vector<int>& frames, float frameDuration, bool looping)
{
    SpriteClip clip;
    clip.frameIndices = frames;
    clip.frameDuration = frameDuration;
    clip.looping = looping;

    animationClips[name] = clip;
}

const SpriteClip* SpriteSheet::GetClip(const std::string& name) const
{
    auto it = animationClips.find(name);
    if (it != animationClips.end())
        return &it->second;
    return nullptr;
}


SpriteAnimator::SpriteAnimator(SpriteSheet* sheet_, float frameTime_, bool loop_)
    : sheet(sheet_), frameTime(frameTime_), loop(loop_) {
}

void SpriteAnimator::PlayClip(int start, int end, bool loop_)
{
    playingClip = nullptr;
    startFrame = start;
    endFrame = end;
    loop = loop_;
    currentFrame = start;
    elapsed = 0.0f;
}
void SpriteAnimator::PlayClip(const std::string& clipName)
{
    const auto* clip = sheet->GetClip(clipName);
    if (!clip || clip->frameIndices.empty())
        return;

    playingClip = clip;
    clipFrameIndex = 0;
    elapsed = 0.0f;
    currentFrame = clip->frameIndices[clipFrameIndex];
}

void SpriteAnimator::Update(float dt)
{
    elapsed += dt;

    if (playingClip)
    {
        if (elapsed >= playingClip->frameDuration)
        {
            elapsed -= playingClip->frameDuration;
            ++clipFrameIndex;

            if (clipFrameIndex >= static_cast<int>(playingClip->frameIndices.size()))
            {
                if (playingClip->looping)
                    clipFrameIndex = 0;
                else
                    clipFrameIndex = static_cast<int>(playingClip->frameIndices.size()) - 1;
            }

            currentFrame = playingClip->frameIndices[clipFrameIndex];
        }
    }
    else
    {
        if (elapsed >= frameTime)
        {
            elapsed -= frameTime;
            ++currentFrame;

            if (currentFrame > endFrame)
            {
                if (loop)
                    currentFrame = startFrame;
                else
                    currentFrame = endFrame;
            }
        }
    }
}


glm::vec2 SpriteAnimator::GetUVOffset() const
{
    return sheet ? sheet->GetUVOffset(currentFrame) : glm::vec2(0.0f);
}

glm::vec2 SpriteAnimator::GetUVScale() const
{
    return sheet ? sheet->GetUVScale() : glm::vec2(1.0f);
}
