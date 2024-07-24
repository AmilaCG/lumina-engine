#pragma once

#include "BloomFBO.h"
#include "Shader.h"

class BloomRenderer
{
public:
    BloomRenderer(const unsigned int& windowWidth, const unsigned int& windowHeight);
    ~BloomRenderer();
    void renderBloomTexture(unsigned int srcTexture, float filterRadius);
    unsigned int bloomTexture() const;

private:
    bool init(unsigned int windowWidth, unsigned int windowHeight);
    void renderDownsamples(unsigned int srcTexture);
    void renderUpsamples(float filterRadius);

    bool mInit;
    BloomFBO mFBO;
    unsigned int mQuadVAO;
    glm::ivec2 mSrcViewportSize;
    glm::vec2 mSrcViewportSizeFloat;
    Shader* mDownsampleShader;
    Shader* mUpsampleShader;
};
