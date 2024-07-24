#include "BloomRenderer.h"

#include <iostream>
#include <glad/glad.h>

BloomRenderer::BloomRenderer(const unsigned int& windowWidth, const unsigned int& windowHeight) :
    mInit(false),
    mQuadVAO(0),
    mSrcViewportSize(),
    mSrcViewportSizeFloat(),
    mDownsampleShader(nullptr),
    mUpsampleShader(nullptr)
{
    constexpr float QUAD_VERTICIES[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    unsigned int quadVBO = 0;
    // Setup quad VAO
    glGenVertexArrays(1, &mQuadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(mQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICIES), &QUAD_VERTICIES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    init(windowWidth, windowHeight);
}

BloomRenderer::~BloomRenderer()
{
    mFBO.destroy();
    delete mDownsampleShader;
    delete mUpsampleShader;
    mInit = false;
}

bool BloomRenderer::init(const unsigned int windowWidth, const unsigned int windowHeight)
{
    if (mInit) { return true; }

    mSrcViewportSize = glm::ivec2(windowWidth, windowHeight);
    mSrcViewportSizeFloat = glm::vec2(static_cast<float>(windowWidth), static_cast<float>(windowHeight));

    // Framebuffer
    constexpr unsigned int NUM_BLOOM_MIPS = 5; // Experiment with this value
    const bool status = mFBO.init(windowWidth, windowHeight, NUM_BLOOM_MIPS);
    if (!status) {
        std::cerr << "Failed to initialize bloom FBO - cannot create bloom renderer!\n";
        return false;
    }

    const char* SCR_V_SHADER_PATH = "Assets/Shaders/shader_screen.vert";
    const char* UPSAMPLE_F_SHADER_PATH = "Assets/Shaders/shader_upsample.frag";
    const char* DOWNSAMPLE_F_SHADER_PATH = "Assets/Shaders/shader_downsample.frag";

    // Shaders
    mDownsampleShader = new Shader(SCR_V_SHADER_PATH, DOWNSAMPLE_F_SHADER_PATH);
    mUpsampleShader = new Shader(SCR_V_SHADER_PATH, UPSAMPLE_F_SHADER_PATH);

    // Downsample
    mDownsampleShader->use();
    mDownsampleShader->setInt("srcTexture", 0);

    // Upsample
    mUpsampleShader->use();
    mUpsampleShader->setInt("srcTexture", 0);

    mInit = true;
    return true;
}

void BloomRenderer::renderBloomTexture(unsigned int srcTexture, float filterRadius)
{
    mFBO.bindForWriting();

    renderDownsamples(srcTexture);
    renderUpsamples(filterRadius);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Restore viewport
    glViewport(0, 0, mSrcViewportSize.x, mSrcViewportSize.y);
}

GLuint BloomRenderer::bloomTexture() const
{
    return mFBO.mipChain()[0].texture;
}

void BloomRenderer::renderDownsamples(unsigned int srcTexture)
{
    const std::vector<BloomMip>& mipChain = mFBO.mipChain();

    mDownsampleShader->use();
    mDownsampleShader->setVec2("srcResolution", mSrcViewportSizeFloat);

    // Bind srcTexture (HDR color buffer) as initial texture input
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, srcTexture);

    // Progressively downsample through the mip chain
    for (int i = 0; i < mipChain.size(); i++)
    {
        const BloomMip& mip = mipChain[i];
        glViewport(0, 0, mip.size.x, mip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, mip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        glBindVertexArray(mQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        // Set current mip resolution as srcResolution for next iteration
        mDownsampleShader->setVec2("srcResolution", mip.size);
        // Set current mip as texture input for next iteration
        glBindTexture(GL_TEXTURE_2D, mip.texture);
    }
}

void BloomRenderer::renderUpsamples(float filterRadius)
{
    const std::vector<BloomMip>& mipChain = mFBO.mipChain();

    mUpsampleShader->use();
    mUpsampleShader->setFloat("filterRadius", filterRadius);

    // Enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);

    for (int i = mipChain.size() - 1; i > 0; i--)
    {
        const BloomMip& mip = mipChain[i];
        const BloomMip& nextMip = mipChain[i-1];

        // Bind viewport and texture from where to read
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        // Set framebuffer render target (we write to this texture)
        glViewport(0, 0, nextMip.size.x, nextMip.size.y);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, nextMip.texture, 0);

        // Render screen-filled quad of resolution of current mip
        glBindVertexArray(mQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    // Disable additive blending
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Restore if this was default
    glDisable(GL_BLEND);
}
