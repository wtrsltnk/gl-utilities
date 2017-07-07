#ifndef GL_UTILITIES_TEXTURES_H
#define GL_UTILITIES_TEXTURES_H

#include <GL/glextl.h>
#include <string>
#include <iostream>

class Texture
{
    friend class TextureLoader;
    GLuint _textureId;
    int _width;
    int _height;
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
    
    int width() const { return this->_width; }
    int height() const { return this->_height; }
};

#endif // GL_UTILITIES_TEXTURES_H
