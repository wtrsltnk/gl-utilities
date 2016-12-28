
#include "glfw-setup.h"

#include "gl.utilities.shaders.h"
#include "gl.utilities.vertexbuffers.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef Vertex<glm::vec3, glm::vec4> LocalVertex;

class Program : public GlfwProgram
{
public:
    Program(int width, int height);

    virtual bool SetUp();
    virtual void Render();
    virtual void CleanUp();
    virtual void OnResize(int width, int height);

    glm::mat4 _proj, _view;
    glm::vec3 _pos;
    Shader<glm::vec3, glm::vec4> _shader;
    VertexBuffer<glm::vec3, glm::vec4> _vbuffer;
};

Program::Program(int width, int height)
    : GlfwProgram (width, height), _vbuffer(_shader)
{
    this->title = "Vertex and Color example";
}

const static std::string vshader(
        "#version 150\n"

        "in vec3 vertex;"
        "in vec4 color;"

        "uniform mat4 u_projection;"
        "uniform mat4 u_view;"
        "uniform mat4 u_model;"

        "out vec4 f_color;"

        "void main()"
        "{"
        "    gl_Position = u_projection * u_view * u_model * vec4(vertex.xyz, 1.0);"
        "    f_color = color;"
        "}"
    );

const static std::string fshader(
        "#version 150\n"

        "in vec4 f_color;"
        "out vec4 color;"

        "void main()"
        "{"
        "   color = f_color;"
        "}"
    );

bool Program::SetUp()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor((142.0f / 255.0f), (179.0f / 255.0f), (171.0f / 255.0f), 1.0f);

    this->_shader.compile(vshader, fshader);

    this->_vbuffer
            .color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)).vertex(glm::vec3(10.0f, -10.0f, 0.0f))
            .color(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)).vertex(glm::vec3(10.0f, 10.0f, 0.0f))
            .color(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)).vertex(glm::vec3(-10.0f, 10.0f, 0.0f))
            .color(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)).vertex(glm::vec3(-10.0f, -10.0f, 0.0f));

    this->_vbuffer.setup();

    return true;
}

void Program::OnResize(int width, int height)
{
    glViewport(0, 0, width, height);

    this->_proj = glm::perspective(glm::radians(90.0f), float(width) / float(height), 0.1f, 4096.0f);
    this->_view = glm::lookAt(
                glm::vec3(this->_pos.x + 12.0f, this->_pos.y + 12.0f, this->_pos.z + 10.0f),
                glm::vec3(this->_pos.x, this->_pos.y, this->_pos.z),
                glm::vec3(0.0f, 0.0f, 1.0f)
                );
}

void Program::Render()
{
    this->_shader.use();
    this->_shader.setupMatrices(glm::value_ptr(this->_proj), glm::value_ptr(this->_view), glm::value_ptr(glm::mat4(1.0f)));
    this->_vbuffer.render();
}

void Program::CleanUp()
{
    this->_vbuffer.cleanup();
}

int main(int argc, char* argv[])
{
    return Program(1024, 768).Run(argc, argv);
}
