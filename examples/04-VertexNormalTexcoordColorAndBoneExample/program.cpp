
#include "glfw-setup.h"

// First stb image so gl.utilities can use it for image loading
#include "stb_image.h"
#include "gl.utilities.shaders.h"
#include "gl.utilities.vertexbuffers.h"
#include "gl.utilities.textures.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef Vertex<glm::vec3, glm::vec3, glm::vec2, glm::vec4, int> ExampleVertex;
typedef Shader<glm::vec3, glm::vec3, glm::vec2, glm::vec4, int> ExampleShader;
typedef VertexBuffer<glm::vec3, glm::vec3, glm::vec2, glm::vec4, int> ExampleVertexBuffer;

class Program : public GlfwProgram
{
public:
    Program(int width, int height);

    virtual bool SetUp();
    virtual void Render(double deltaTime);
    virtual void CleanUp();
    virtual void OnResize(int width, int height);

    glm::mat4 _proj, _view;
    glm::vec3 _pos;
    glm::mat4 _bones[4];

    float _value = 0.0f;
    float _mult = 15.0f;
    float _angle;
    glm::mat4 _model;

    ExampleShader _shader;
    ExampleVertexBuffer _vbuffer;
    Texture _texture;
};

Program::Program(int width, int height)
    : GlfwProgram (width, height), _vbuffer(_shader)
{
    this->title = "Vertex, Normal, Texcoord, Color and Bone example";
}

const static std::string vshader(
        "#version 150\n"

        "in vec3 vertex;"
        "in vec3 normal;"
        "in vec2 texcoord;"
        "in vec4 color;"
        "in int bone;"

        "uniform mat4 u_projection;"
        "uniform mat4 u_view;"
        "uniform mat4 u_model;"
        "layout(std140) uniform BonesBlock"
        "{"
        "   mat4 u_bones[32];"
        "};"

        "out vec2 f_texcoord;"
        "out vec4 f_color;"

        "void main()"
        "{"
        "    gl_Position = u_projection * u_view * u_model * u_bones[bone] * vec4(vertex.xyz, 1.0);"
        "    f_texcoord = texcoord;"
        "    f_color = color;"
        "}"
    );

const static std::string fshader(
        "#version 150\n"

        "uniform sampler2D u_texture;"

        "in vec2 f_texcoord;"
        "in vec4 f_color;"

        "out vec4 color;"

        "void main()"
        "{"
        "   color = f_color * texture(u_texture, f_texcoord);"
        "}"
    );

bool Program::SetUp()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor((142.0f / 255.0f), (179.0f / 255.0f), (171.0f / 255.0f), 1.0f);

    this->_shader.compile(vshader, fshader, 4);

    this->_vbuffer
            << ExampleVertex({ { 10.0f, -10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 1 })
            << ExampleVertex({ { 10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f }, 0 })
            << ExampleVertex({ { -10.0f, 10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f }, 1 })
            << ExampleVertex({ { -10.0f, -10.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }, 0 });
    this->_vbuffer.setup();

    this->_bones[0] = glm::mat4(1.0f);
    this->_bones[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    this->_model = glm::mat4(1.0f);
    this->_texture.setup();
    this->_texture.load("examples/opengl.png");

    return true;
}

void Program::OnResize(int width, int height)
{
    glViewport(0, 0, width, height);

    this->_proj = glm::perspective(glm::radians(90.0f), float(width) / float(height), 0.1f, 4096.0f);
    this->_view = glm::lookAt(this->_pos + glm::vec3(32.0f), this->_pos, glm::vec3(0.0f, 0.0f, 1.0f));
}

void Program::Render(double deltaTime)
{
    this->_value += (this->_mult * deltaTime);
    if (this->_value > 5.0f || this->_value < -5.0f) this->_mult = -this->_mult;
    this->_bones[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, this->_value));

    this->_angle += (90.0f * deltaTime);
    this->_model = glm::mat4(1.0f);
    this->_model = glm::rotate(this->_model, glm::radians(this->_angle), glm::vec3(0.0f, 0.0f, 1.0f));
    this->_model = glm::translate(this->_model, glm::vec3(0.0f, 30.0f, 0.0f));
    this->_model = glm::rotate(this->_model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    this->_shader.use();
    this->_shader.setupBones(((const float(*)[16])glm::value_ptr(this->_bones[0])), 2);
    this->_shader.setupMatrices(glm::value_ptr(this->_proj), glm::value_ptr(this->_view), glm::value_ptr(this->_model));
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
    return Program(1024, 768)
            .Run(argc, argv);
}
