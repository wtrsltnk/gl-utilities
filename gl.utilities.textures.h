#ifndef GL_UTILITIES_TEXTURES_H
#define GL_UTILITIES_TEXTURES_H

#include <GL/glextl.h>
#include <string>
#include <iostream>

class Texture
{
    GLuint _textureId;
public:
    Texture() : _textureId(0) { }
    virtual ~Texture() { this->cleanup(); }

    void setup()
    {
        glGenTextures(1, &this->_textureId);
    }

    void use() const
    {
        glBindTexture(GL_TEXTURE_2D, this->_textureId);
    }

    void cleanup()
    {
        if (this->_textureId != 0)
        {
            glDeleteTextures(1, &this->_textureId);
            this->_textureId = 0;
        }
    }
// When STB image is included, we can load images trough this library
#ifdef STBI_INCLUDE_STB_IMAGE_H
    bool load(const std::string& filename)
    {
        int x = 0, y = 0, comp = 3;
        auto imageData = stbi_load(filename.c_str(), &x, &y, &comp, 4);
        if (imageData != nullptr)
        {
            std::cout << "loaded " << filename << std::endl;
            auto format = comp == 4 ? GL_RGBA : GL_RGB;
            glBindTexture(GL_TEXTURE_2D, this->_textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, format, x, y, 0, format, GL_UNSIGNED_BYTE, imageData);
            free(imageData);

            return true;
        }

        std::cout << "Unable to load " << filename << std::endl;
        return false;
    }
#endif // STBI_INCLUDE_STB_IMAGE_H
};

#endif // GL_UTILITIES_TEXTURES_H
