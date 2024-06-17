#pragma once
#include <glad/glad.h>
#include <string>
#include <array>

namespace TextureUtils
{
    unsigned int loadTexture(const std::string& filePath, const bool& gammaCorrection);
    unsigned int loadCubemapTexture(const std::array<std::string, 6>& filePaths, const bool& gammaCorrection);
    int evaluateFormats(const int& nrChannels, GLenum& internalFormat, GLenum& dataFormat, const bool& correctGamma);
}
