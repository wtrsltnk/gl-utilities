
#include <GL/glextl.h>

#include "glfw-setup.h"
#include <GLFW/glfw3.h>

GlfwProgram* GlfwProgram::program = nullptr;

GlfwProgram::GlfwProgram(int width, int height)
    : width(width), height(height), title("GlfwWindow"), keepRunning(true), window(nullptr)
{
    GlfwProgram::program = this;
}

GlfwProgram::~GlfwProgram()
{ }

int GlfwProgram::Run(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        this->args.push_back(argv[i]);
    }

    if (glfwInit() == GLFW_FALSE)
        return -1;

    this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), NULL, NULL);
    if (this->window == nullptr)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(this->window, GlfwProgram::KeyActionCallback);
    glfwSetMouseButtonCallback(this->window, GlfwProgram::MouseActionCallback);
    glfwSetCursorPosCallback(this->window, GlfwProgram::MouseMoveCallback);
    glfwSetFramebufferSizeCallback(this->window, GlfwProgram::ResizeCallback);

    glfwMakeContextCurrent(this->window);

    glExtLoadAll((PFNGLGETPROC*)glfwGetProcAddress);

    if (this->SetUp())
    {
        GlfwProgram::ResizeCallback(this->window, this->width, this->height);

        double time = glfwGetTime();
        while (glfwWindowShouldClose(this->window) == 0 && this->keepRunning)
        {
            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            this->Render(glfwGetTime() - time);
            time = glfwGetTime();

            glfwSwapBuffers(this->window);

        }
        this->CleanUp();
    }

    glfwTerminate();

    return 0;
}

float GlfwProgram::elapsed() const
{
    return glfwGetTime();
}

void GlfwProgram::KeyActionCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
    if(GlfwProgram::program)
    {
        //GlfwProgram::program->_input;
    }
}

void GlfwProgram::MouseActionCallback(GLFWwindow*, int button, int action, int mods)
{
    if(GlfwProgram::program)
    {
        //GlfwProgram::program->_input;
    }
}

void GlfwProgram::MouseMoveCallback(GLFWwindow*, double x, double y)
{
    if(GlfwProgram::program)
    {
        //GlfwProgram::program->_input;
    }
}

void GlfwProgram::ResizeCallback(GLFWwindow*, int width, int height)
{
    if(GlfwProgram::program)
    {
        if (width <= 0) width = 1;
        if (height <= 0) height = 1;

        GlfwProgram::program->width = width;
        GlfwProgram::program->height = height;

        GlfwProgram::program->OnResize(width, height);
    }
}
