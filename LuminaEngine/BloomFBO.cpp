#include "BloomFBO.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

BloomFBO::BloomFBO() : mInit(false), mFBO(0) {}

BloomFBO::~BloomFBO() = default;

bool BloomFBO::init(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength)
{
    if (mInit) return true;

    glGenFramebuffers(1, &mFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    glm::vec2 mipSize(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    glm::ivec2 mipIntSize(static_cast<int>(windowWidth), static_cast<int>(windowHeight));

    if (windowWidth > static_cast<unsigned int>(INT_MAX) ||
        windowHeight > static_cast<unsigned int>(INT_MAX))
    {
        std::cerr << "Window size conversion overflow - cannot build bloom FBO!\n";
        return false;
    }

    for (unsigned int i = 0; i < mipChainLength; i++)
    {
        BloomMip mip {};

        mipSize *= 0.5f;
        mipIntSize /= 2;
        mip.size = mipSize;
        mip.intSize = mipIntSize;

        glGenTextures(1, &mip.texture);
        glBindTexture(GL_TEXTURE_2D, mip.texture);
        // we are downscaling an HDR color buffer, so we need a float texture format
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, static_cast<int>(mipSize.x),
            static_cast<int>(mipSize.y), 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        mMipChain.emplace_back(mip);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, mMipChain[0].texture, 0);

    // setup attachments
    constexpr unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    // check completion status
    const int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("gbuffer FBO error, status: 0x\%x\n", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    mInit = true;
    return true;
}

void BloomFBO::destroy()
{
    for (auto &[size, intSize, texture] : mMipChain)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }

    glDeleteFramebuffers(1, &mFBO);
    mFBO = 0;
    mInit = false;
}

void BloomFBO::bindForWriting() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
}

const std::vector<BloomMip>& BloomFBO::mipChain() const
{
    return mMipChain;
}
