#include <iostream>
#include "glextloader.h"

void printVersion();
void printHelp();

int main(int argc, char* argv[])
{
    printVersion();

    if (argc == 1)
    {
        std::cout << "Too few arguments, I need atleast a source directory where I cn find glext.h." << std::endl;
        return 0;
    }

    if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")
    {
        printHelp();
        return 0;
    }

    std::string sourcefolder = argv[1];

    std::ifstream stream(sourcefolder + "\\include\\GL\\glext.h");
    if (!stream.is_open())
    {
        std::cout << "Unable to find " << sourcefolder << "\\include\\GL\\glext.h" << std::endl;
        return 0;
    }
    std::vector<Feature> features = LoadFeatures(stream);
    stream.close();

    if (features.size() == 0)
    {
        std::cout << "No features found in " << sourcefolder << "\\include\\GL\\glext.h" << std::endl;
        return 0;
    }

    std::cout << int(features.size()) << " features loaded from "  << sourcefolder << "\\include\\GL\\glext.h" << std::endl;

    std::string targetfolder = argv[1];

    // GLEXTL header file
    std::ofstream glext_h((targetfolder + "\\include\\GL\\glextl.h").c_str());
    if (!glext_h.is_open())
    {
        std::cout << "Unable to open " << targetfolder << "\\include\\GL\\glextl.h" << std::endl;
        return 0;
    }

    // GLEXTL header file
    Writer()
            // Write Declaration
            .IfNotDef("GLEXTL_H", Writer()
                      .Statement("#define GLEXTL_H")
                      .Statement("#include <GL/gl.h>")
                      .Statement("#define GL_GLEXT_PROTOTYPES")
                      .Statement("#include <GL/glext.h>")
                      .EmptyLine()
                      .Statement("typedef void* (PFNGLGETPROC)(const GLubyte* name);")
                      .EmptyLine()
                      .Statement("GLboolean glExtLoadAll(PFNGLGETPROC* proc);")
                      .Statement("GLboolean glExtLoadCore(PFNGLGETPROC* proc);")
                      .Statement("GLboolean glExtLoadOne(PFNGLGETPROC* proc, const char* name);")
                      .Statement("GLboolean glExtIsLoaded(const char* name);")
                      .EmptyLine()
                      )
            .EmptyLine()

            // Write implementation
            .IfDef("GLEXTL_IMPLEMENTATION", Writer()
                   .IfNotDef("_GLEXTL_IMPLEMENTATION_GUARD_", Writer()
                             .Statement("#define _GLEXTL_IMPLEMENTATION_GUARD_")
                             .Statement(WriteFeatures(features))
                             )
                   )
            .Write(glext_h);

    glext_h.close();

    std::cout << features.size() << " features written to " << targetfolder << "\\include\\GL\\glextl.h" << std::endl;

    // GLEXTL example implementation file
    std::ofstream glext_cpp((targetfolder + "\\glextl_impl.cpp").c_str());
    if (!glext_cpp.is_open())
    {
        std::cout << "Unable to open " << targetfolder << "\\glextl_impl.cpp" << std::endl;
        return 0;
    }

    Writer()
            .EmptyLine()
            .Statement("#define GLEXTL_IMPLEMENTATION")
            .Statement("#include <GL/glextl.h>")
            .EmptyLine()
            .Write(glext_cpp);

    glext_cpp.close();

    std::cout << "Example implementation file written to " << targetfolder << "\\glextl.cpp" << std::endl;

    return 0;
}

void printVersion()
{
    std::cout << "Version?" << std::endl;
}

void printHelp()
{
    std::cout << "Help?" << std::endl;
}
