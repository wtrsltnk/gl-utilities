#ifndef VBUFFER_H
#define VBUFFER_H

#include <GL/glextl.h>
#include <string>
#include <vector>
#include <iostream>

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

class CompiledShader
{
protected:
    GLuint _shaderId;
public:
    CompiledShader() : _shaderId(0) { }
    virtual ~CompiledShader() { }

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
        auto vertexAttrib = glGetAttribLocation(this->_shaderId, this->_vertexAttributeName.c_str());
        glVertexAttribPointer(GLuint(vertexAttrib), sizeof(PositionType) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(PositionType) + sizeof(ColorType), 0);
        glEnableVertexAttribArray(GLuint(vertexAttrib));

        auto colorAttrib = glGetAttribLocation(this->_shaderId, this->_colorAttributeName.c_str());
        glVertexAttribPointer(GLuint(colorAttrib), sizeof(ColorType) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(PositionType) + sizeof(ColorType), reinterpret_cast<const GLvoid*>(sizeof(PositionType)));
        glEnableVertexAttribArray(GLuint(colorAttrib));
    }
};

template <class...> class VertexBuffer;

template <class PositionType, class ColorType>
class VertexBuffer<PositionType, ColorType>
{
    unsigned int _vertexArrayId;
    unsigned int _vertexBufferId;
public:
private:
    const Shader<PositionType, ColorType>& _shader;
    std::vector<Vertex<PositionType, ColorType>> _verts;
    int _vertexCount;

public:
    VertexBuffer(const Shader<PositionType, ColorType>& shader)
        : _vertexArrayId(0), _vertexBufferId(0), _shader(shader), _vertexCount(0)
    { }

    VertexBuffer<PositionType, ColorType>& operator << (const Vertex<PositionType, ColorType>& vertex)
    {
        this->_verts.push_back(vertex);

        return *this;
    }

    void setup()
    {
        glGenVertexArrays(1, &this->_vertexArrayId);
        glGenBuffers(1, &this->_vertexBufferId);

        glBindVertexArray(this->_vertexArrayId);
        glBindBuffer(GL_ARRAY_BUFFER, this->_vertexBufferId);

        glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, ColorType>)), 0, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, GLsizeiptr(this->_verts.size() * sizeof(Vertex<PositionType, ColorType>)), reinterpret_cast<const GLvoid*>(&this->_verts[0]));

        this->_shader.setupAttributes();

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        this->_vertexCount = this->_verts.size();
        this->_verts.clear();
    }

    void render()
    {
        glBindVertexArray(this->_vertexArrayId);
        glDrawArrays(GL_QUADS, 0, this->_vertexCount);
        glBindVertexArray(0);
    }
};

#endif // VBUFFER_H
