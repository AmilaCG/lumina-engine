#pragma once

#include <glm/vec2.hpp>
#include <vector>

struct BloomMip
{
    glm::vec2 size;
    glm::ivec2 intSize;
    unsigned int texture;
};

class BloomFBO
{
public:
    BloomFBO();
    ~BloomFBO();
    bool init(unsigned int windowWidth, unsigned int windowHeight, unsigned int mipChainLength);
    void destroy();
    void bindForWriting() const;
    const std::vector<BloomMip>& mipChain() const;

private:
    bool mInit;
    unsigned int mFBO;
    std::vector<BloomMip> mMipChain;
};
