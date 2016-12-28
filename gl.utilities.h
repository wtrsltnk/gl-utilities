#ifndef GL_UTILITIES_H
#define GL_UTILITIES_H

#include <GL/glextl.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <streambuf>

// Vertex
template <class...> class Vertex;

template <class PositionType, class ColorType>
class Vertex<PositionType, ColorType>
{
public:
    PositionType pos;
    ColorType col;
};

template <class PositionType, class NormalType, class TexcoordType>
class Vertex<PositionType, NormalType, TexcoordType>
{
public:
    PositionType pos;
    NormalType normal;
    TexcoordType uv;
};

template <class PositionType, class NormalType, class TexcoordType, class ColorType>
class Vertex<PositionType, NormalType, TexcoordType, ColorType>
{
public:
    PositionType pos;
    NormalType normal;
    TexcoordType uv;
    ColorType color;
};



// Shaders
class CompiledShader
{
protected:
    GLuint _shaderId;
public:
    CompiledShader() : _shaderId(0) { }
    virtual ~CompiledShader() { }

    virtual bool compileFromFile(const std::string& vertShaderFile, const std::string& fragShaderFile)
    {
        std::ifstream vertShaderFileStream(vertShaderFile.c_str());
        std::string vertShaderStr((std::istreambuf_iterator<char>(vertShaderFileStream)),
                         std::istreambuf_iterator<char>());

        std::ifstream fragShaderFileStream(fragShaderFile.c_str());
        std::string fragShaderStr((std::istreambuf_iterator<char>(fragShaderFileStream)),
                         std::istreambuf_iterator<char>());

        return compile(vertShaderStr, fragShaderStr);
    }

    virtual bool compile(const std::string& vertShaderStr, const std::string& fragShaderStr)
    {
        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *vertShaderSrc = vertShaderStr.c_str();
        const char *fragShaderSrc = fragShaderStr.c_str();

        GLint result = GL_FALSE;
        GLint logLength;

        // Compile vertex shader
        glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
        glCompileShader(vertShader);

        // Check vertex shader
        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<GLchar> vertShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
            glGetShaderInfoLog(vertShader, logLength, NULL, &vertShaderError[0]);
            std::cout << &vertShaderError[0] << std::endl;

            return false;
        }

        // Compile fragment shader
        glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
        glCompileShader(fragShader);

        // Check fragment shader
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<GLchar> fragShaderError(static_cast<size_t>((logLength > 1) ? logLength : 1));
            glGetShaderInfoLog(fragShader, logLength, NULL, &fragShaderError[0]);
            std::cout << &fragShaderError[0] << std::endl;

            return false;
        }

        this->_shaderId = glCreateProgram();
        glAttachShader(this->_shaderId, vertShader);
        glAttachShader(this->_shaderId, fragShader);
        glLinkProgram(this->_shaderId);

        glGetProgramiv(this->_shaderId, GL_LINK_STATUS, &result);
        if (result == GL_FALSE)
        {
            glGetProgramiv(this->_shaderId, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<GLchar> programError(static_cast<size_t>((logLength > 1) ? logLength : 1));
            glGetProgramInfoLog(this->_shaderId, logLength, NULL, &programError[0]);
            std::cout << &programError[0] << std::endl;

            return false;
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return true;
    }

    void use() const
    {
        glUseProgram(this->_shaderId);
    }
};

// Shaders with Projection, View and Model uniforms
class PVMShader : public CompiledShader
{
    GLuint _projectionUniformId;
    GLuint _viewUniformId;
    GLuint _modelUniformId;

public:
    PVMShader()
        : CompiledShader(), _projectionUniformId(0), _viewUniformId(0), _modelUniformId(0),
          _projectionUniformName("u_projection"), _viewUniformName("u_view"), _modelUniformName("u_model")
    { }
    virtual ~PVMShader() { }

    std::string _projectionUniformName;
    std::string _viewUniformName;
    std::string _modelUniformName;

    virtual bool compile(const std::string& vertShaderStr, const std::string& fragShaderStr)
    {
        if (!CompiledShader::compile(vertShaderStr, fragShaderStr))
            return false;

        this->_projectionUniformId = glGetUniformLocation(this->_shaderId, this->_projectionUniformName.c_str());
        this->_viewUniformId = glGetUniformLocation(this->_shaderId, this->_viewUniformName.c_str());
        this->_modelUniformId = glGetUniformLocation(this->_shaderId, this->_modelUniformName.c_str());

        return true;
    }

    void setupMatrices(const float projection[], const float view[], const float model[])
    {
        this->use();

        glUniformMatrix4fv(this->_projectionUniformId, 1, false, projection);
        glUniformMatrix4fv(this->_viewUniformId, 1, false, view);
        glUniformMatrix4fv(this->_modelUniformId, 1, false, model);
    }
};

template <class...> class Shader;

template <class PositionType, class ColorType>
class Shader<PositionType, ColorType> : public PVMShader
{
public:
    Shader()
        : _vertexAttributeName("vertex"), _colorAttributeName("color")
    { }

    virtual ~Shader() { }

    std::string _vertexAttributeName;
    std::string _colorAttributeName;

    void setupAttributes() const
    {
        auto vertexSize = sizeof(PositionType) + sizeof(ColorType);

        auto vertexAttrib = glGetAttribLocation(this->_shaderId, this->_vertexAttributeName.c_str());
        glVertexAttribPointer(GLuint(vertexAttrib), sizeof(PositionType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, 0);
        glEnableVertexAttribArray(GLuint(vertexAttrib));

        auto colorAttrib = glGetAttribLocation(this->_shaderId, this->_colorAttributeName.c_str());
        glVertexAttribPointer(GLuint(colorAttrib), sizeof(ColorType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const GLvoid*>(sizeof(PositionType)));
        glEnableVertexAttribArray(GLuint(colorAttrib));
    }
};

template <class PositionType, class NormalType, class TexcoordType>
class Shader<PositionType, NormalType, TexcoordType> : public PVMShader
{
    GLuint _textureUniformId;
public:
    Shader()
        : _vertexAttributeName("vertex"), _normalAttributeName("normal"), _texcoordAttributeName("texcoord"),
          _textureUniformName("texture")
    { }

    virtual ~Shader() { }

    std::string _vertexAttributeName;
    std::string _normalAttributeName;
    std::string _texcoordAttributeName;
    std::string _textureUniformName;

    void setupAttributes() const
    {
        auto vertexSize = sizeof(PositionType) + sizeof(NormalType) + sizeof(TexcoordType);

        GLuint vertexAttrib = glGetAttribLocation(this->_shaderId, this->_vertexAttributeName.c_str());
        glVertexAttribPointer(vertexAttrib, sizeof(PositionType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, 0);
        glEnableVertexAttribArray(vertexAttrib);

        GLuint normalAttrib = glGetAttribLocation(this->_shaderId, this->_normalAttributeName.c_str());
        glVertexAttribPointer(normalAttrib, sizeof(NormalType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const GLvoid*>(sizeof(PositionType)));
        glEnableVertexAttribArray(normalAttrib);

        GLuint texcoordAttrib = glGetAttribLocation(this->_shaderId, this->_texcoordAttributeName.c_str());
        glVertexAttribPointer(texcoordAttrib, sizeof(TexcoordType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const GLvoid*>(sizeof(PositionType) + sizeof(NormalType)));
        glEnableVertexAttribArray(texcoordAttrib);
    }

    virtual bool compile(const std::string& vertShaderStr, const std::string& fragShaderStr)
    {
        if (!PVMShader::compile(vertShaderStr, fragShaderStr))
            return false;

        this->_textureUniformId = glGetUniformLocation(this->_shaderId, this->_textureUniformName.c_str());
        glUniform1i(this->_textureUniformId, 0);

        return true;
    }
};

template <class PositionType, class NormalType, class TexcoordType, class ColorType>
class Shader<PositionType, NormalType, TexcoordType, ColorType> : public PVMShader
{
    GLuint _textureUniformId;
public:
    Shader()
        : _vertexAttributeName("vertex"), _normalAttributeName("normal"), _texcoordAttributeName("texcoord"), _colorAttributeName("color"),
          _textureUniformName("texture")
    { }

    virtual ~Shader() { }

    std::string _vertexAttributeName;
    std::string _normalAttributeName;
    std::string _texcoordAttributeName;
    std::string _colorAttributeName;
    std::string _textureUniformName;

    void setupAttributes() const
    {
        auto vertexSize = sizeof(PositionType) + sizeof(NormalType) + sizeof(TexcoordType) + sizeof(ColorType);

        GLuint vertexAttrib = glGetAttribLocation(this->_shaderId, this->_vertexAttributeName.c_str());
        glVertexAttribPointer(vertexAttrib, sizeof(PositionType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, 0);
        glEnableVertexAttribArray(vertexAttrib);

        GLuint normalAttrib = glGetAttribLocation(this->_shaderId, this->_normalAttributeName.c_str());
        glVertexAttribPointer(normalAttrib, sizeof(NormalType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const GLvoid*>(sizeof(PositionType)));
        glEnableVertexAttribArray(normalAttrib);

        GLuint texcoordAttrib = glGetAttribLocation(this->_shaderId, this->_texcoordAttributeName.c_str());
        glVertexAttribPointer(texcoordAttrib, sizeof(TexcoordType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const GLvoid*>(sizeof(PositionType) + sizeof(NormalType)));
        glEnableVertexAttribArray(texcoordAttrib);

        GLuint colorAttrib = glGetAttribLocation(this->_shaderId, this->_colorAttributeName.c_str());
        glVertexAttribPointer(colorAttrib, sizeof(ColorType) / sizeof(float), GL_FLOAT, GL_FALSE, vertexSize, reinterpret_cast<const GLvoid*>(sizeof(PositionType) + sizeof(NormalType) + sizeof(TexcoordType)));
        glEnableVertexAttribArray(colorAttrib);
    }

    virtual bool compile(const std::string& vertShaderStr, const std::string& fragShaderStr)
    {
        if (!PVMShader::compile(vertShaderStr, fragShaderStr))
            return false;

        this->_textureUniformId = glGetUniformLocation(this->_shaderId, this->_textureUniformName.c_str());
        glUniform1i(this->_textureUniformId, 0);

        return true;
    }
};


// Vertex buffers
class RenderableBuffer
{
protected:
    unsigned int _vertexArrayId;
    unsigned int _vertexBufferId;
    int _vertexCount;

    bool setupRenderableBuffer(int vertexCount)
    {
        this->_vertexCount = vertexCount;

        glGenVertexArrays(1, &this->_vertexArrayId);
        glGenBuffers(1, &this->_vertexBufferId);

        return true;
    }

public:
    RenderableBuffer() : _vertexArrayId(0), _vertexBufferId(0), _vertexCount(0) { }
    virtual ~RenderableBuffer() { }

    void render()
    {
        glBindVertexArray(this->_vertexArrayId);
        glDrawArrays(GL_QUADS, 0, this->_vertexCount);
        glBindVertexArray(0);
    }

    void cleanup()
    {
        if (this->_vertexBufferId != 0)
        {
            glDeleteBuffers(1, &this->_vertexBufferId);
            this->_vertexBufferId = 0;
        }
        if (this->_vertexArrayId != 0)
        {
            glDeleteVertexArrays(1, &this->_vertexArrayId);
            this->_vertexArrayId = 0;
        }
    }
};

template <class...> class VertexBuffer;

template <class PositionType, class ColorType>
class VertexBuffer<PositionType, ColorType> : public RenderableBuffer
{
    const Shader<PositionType, ColorType>& _shader;
    std::vector<Vertex<PositionType, ColorType>> _verts;
    ColorType _nextColor;

public:
    VertexBuffer(const Shader<PositionType, ColorType>& shader) : _shader(shader) { }
    virtual ~VertexBuffer() { }

    VertexBuffer<PositionType, ColorType>& operator << (const Vertex<PositionType, ColorType>& vertex)
    {
        this->_verts.push_back(vertex);

        return *this;
    }

    bool setup()
    {
        if (!this->setupRenderableBuffer(this->_verts.size()))
            return false;

        glBindVertexArray(this->_vertexArrayId);
        glBindBuffer(GL_ARRAY_BUFFER, this->_vertexBufferId);

        glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, ColorType>)), 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, ColorType>)), reinterpret_cast<const GLvoid*>(&this->_verts[0]));

        this->_shader.setupAttributes();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        this->_verts.clear();

        return true;
    }

public:
    VertexBuffer<PositionType, ColorType>& vertex(const PositionType& position)
    {
        typedef Vertex<PositionType, ColorType> vertex;

        this->_verts.push_back(vertex({
                                          position,
                                          this->_nextColor
                                      }));
        return *this;
    }

    VertexBuffer<PositionType, ColorType>& color(const ColorType& color)
    {
        this->_nextColor = color;
        return *this;
    }
};

template <class PositionType, class NormalType, class TexcoordType>
class VertexBuffer<PositionType, NormalType, TexcoordType> : public RenderableBuffer
{
    const Shader<PositionType, NormalType, TexcoordType>& _shader;
    std::vector<Vertex<PositionType, NormalType, TexcoordType>> _verts;
    NormalType _nextNormal;
    TexcoordType _nextTexcoord;

public:
    VertexBuffer(const Shader<PositionType, NormalType, TexcoordType>& shader) : _shader(shader) { }
    virtual ~VertexBuffer() { }

    VertexBuffer<PositionType, NormalType, TexcoordType>& operator << (const Vertex<PositionType, NormalType, TexcoordType>& vertex)
    {
        this->_verts.push_back(vertex);

        return *this;
    }

    bool setup()
    {
        if (!this->setupRenderableBuffer(this->_verts.size()))
            return false;

        glBindVertexArray(this->_vertexArrayId);
        glBindBuffer(GL_ARRAY_BUFFER, this->_vertexBufferId);

        glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, NormalType, TexcoordType>)), 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, NormalType, TexcoordType>)), reinterpret_cast<const GLvoid*>(&this->_verts[0]));

        this->_shader.setupAttributes();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        this->_verts.clear();

        return true;
    }

public:
    VertexBuffer<PositionType, NormalType, TexcoordType>& vertex(const PositionType& position)
    {
        typedef Vertex<PositionType, NormalType, TexcoordType> vertex;

        this->_verts.push_back(vertex({
                                          position,
                                          this->_nextNormal,
                                          this->_nextTexcoord
                                      }));
        return *this;
    }

    VertexBuffer<PositionType, NormalType, TexcoordType>& normal(const NormalType& normal)
    {
        this->_nextNormal = normal;
        return *this;
    }

    VertexBuffer<PositionType, NormalType, TexcoordType>& texcoord(const TexcoordType& texcoord)
    {
        this->_nextTexcoord = texcoord;
        return *this;
    }
};

template <class PositionType, class NormalType, class TexcoordType, class ColorType>
class VertexBuffer<PositionType, NormalType, TexcoordType, ColorType> : public RenderableBuffer
{
    const Shader<PositionType, NormalType, TexcoordType, ColorType>& _shader;
    std::vector<Vertex<PositionType, NormalType, TexcoordType, ColorType>> _verts;
    NormalType _nextNormal;
    TexcoordType _nextTexcoord;
    ColorType _nextColor;

public:
    VertexBuffer(const Shader<PositionType, NormalType, TexcoordType, ColorType>& shader) : _shader(shader) { }
    virtual ~VertexBuffer() { }

    VertexBuffer<PositionType, NormalType, TexcoordType, ColorType>& operator << (const Vertex<PositionType, NormalType, TexcoordType, ColorType>& vertex)
    {
        this->_verts.push_back(vertex);

        return *this;
    }

    bool setup()
    {
        if (!this->setupRenderableBuffer(this->_verts.size()))
            return false;

        glBindVertexArray(this->_vertexArrayId);
        glBindBuffer(GL_ARRAY_BUFFER, this->_vertexBufferId);

        glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, NormalType, TexcoordType, ColorType>)), 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, NormalType, TexcoordType, ColorType>)), reinterpret_cast<const GLvoid*>(&this->_verts[0]));

        this->_shader.setupAttributes();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        this->_verts.clear();

        return true;
    }

public:
    VertexBuffer<PositionType, NormalType, TexcoordType, ColorType>& vertex(const PositionType& position)
    {
        typedef Vertex<PositionType, NormalType, TexcoordType, ColorType> vertex;

        this->_verts.push_back(vertex({
                                          position,
                                          this->_nextNormal,
                                          this->_nextTexcoord,
                                          this->_nextColor
                                      }));
        return *this;
    }

    VertexBuffer<PositionType, NormalType, TexcoordType, ColorType>& normal(const NormalType& normal)
    {
        this->_nextNormal = normal;
        return *this;
    }

    VertexBuffer<PositionType, NormalType, TexcoordType, ColorType>& texcoord(const TexcoordType& texcoord)
    {
        this->_nextTexcoord = texcoord;
        return *this;
    }

    VertexBuffer<PositionType, NormalType, TexcoordType, ColorType>& color(const ColorType& color)
    {
        this->_nextColor = color;
        return *this;
    }
};


// Texture
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

#endif // GL_UTILITIES_H
