
#include "glfw-setup.h"

// First stb image so gl.utilities can use it for image loading
#include "stb_image.h"
#include "gl.utilities.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
    Shader<glm::vec3, glm::vec3, glm::vec2> _shader;
    VertexBuffer<glm::vec3, glm::vec3, glm::vec2> _vbuffer;
    Texture _texture;
};

Program::Program(int width, int height)
    : GlfwProgram (width, height), _vbuffer(_shader)
{ }

bool Program::SetUp()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor((142.0f / 255.0f), (179.0f / 255.0f), (171.0f / 255.0f), 1.0f);

    this->_shader.compileFromFile("examples/vertex.glsl", "examples/fragment.glsl");

    this->_texture.setup();
    this->_texture.load("examples/opengl.png");

    this->_vbuffer
            << Vertex<glm::vec3, glm::vec3, glm::vec2>({ { 10.0f, -10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f } })
            << Vertex<glm::vec3, glm::vec3, glm::vec2>({ { 10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 1.0f } })
            << Vertex<glm::vec3, glm::vec3, glm::vec2>({ { -10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } })
            << Vertex<glm::vec3, glm::vec3, glm::vec2>({ { -10.0f, -10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
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
    this->_texture.use();
    this->_vbuffer.render();
}

void Program::CleanUp()
{
    this->_texture.cleanup();
    this->_vbuffer.cleanup();
}

int main(int argc, char* argv[])
{
    return Program(1024, 768).Run(argc, argv);
}
