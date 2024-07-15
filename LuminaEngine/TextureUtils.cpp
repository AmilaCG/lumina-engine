#include "TextureUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

namespace TextureUtils
{
    unsigned int loadTexture(const std::string& filePath, const bool& gammaCorrection)
    {
        unsigned int textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        int width, height, nrChannels;
        unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum internalFormat;
            GLenum dataFormat;
            if (!evaluateFormats(nrChannels, internalFormat, dataFormat, gammaCorrection))
            {
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
            }
            else
            {
                std::cout << "Texture load failed! Undefined image channels: " << filePath << std::endl;
                stbi_image_free(data);
                return 0;
            }
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << filePath << std::endl;
            return 0;
        }

        return textureID;
    }

    unsigned int loadCubemapTexture(const std::array<std::string, 6>& filePaths, const bool& gammaCorrection)
    {
        unsigned int textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (int i = 0; i < filePaths.size() ; i++)
        {
            unsigned char* data = stbi_load(filePaths[i].c_str(), &width, &height, &nrChannels, 0);
            if (!data)
            {
                std::cout << "Texture failed to load at path: " << filePaths[i] << std::endl;
                return 0;
            }

            GLenum internalFormat;
            GLenum dataFormat;
            if (!evaluateFormats(nrChannels, internalFormat, dataFormat, gammaCorrection))
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Texture load failed! Undefined image channels: " << filePaths[i] << std::endl;
                stbi_image_free(data);
                return 0;
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        return textureID;
    }

    unsigned int loadHdrImage(const std::string& filePath)
    {
        unsigned int textureID = 0;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        stbi_set_flip_vertically_on_load(true);
        int width, height, nrChannels;
        float* data = stbi_loadf(filePath.c_str(), &width, &height, &nrChannels, 0);
        if (!data)
        {
            std::cout << "Texture failed to load at path: " << filePath << std::endl;
            return 0;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        stbi_image_free(data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        return textureID;
    }

    int evaluateFormats(
        const int& nrChannels,
        GLenum& internalFormat,
        GLenum& dataFormat,
        const bool& correctGamma = false)
    {
        switch (nrChannels)
        {
            case 1:
                internalFormat = dataFormat = GL_RED;
                return 0;
            case 3:
                internalFormat = correctGamma ? GL_SRGB : GL_RGB;
                dataFormat = GL_RGB;
                return 0;
            case 4:
                internalFormat = correctGamma ? GL_SRGB_ALPHA : GL_RGBA;
                dataFormat = GL_RGBA;
                return 0;
            default:
                return -1;
        }
    }
}
