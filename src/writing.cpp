#include "glextloader.h"
#include <sstream>
#include <iostream>

std::string writeTabs(int count)
{
    std::stringstream out;
    for (int i = 0; i < count; i++) out << "    ";
    return out.str();
}

std::string join(const std::string& separator, const std::vector<std::string>& values)
{
    std::string result;
    for (auto k : values) result += (k != values.front() ? ", " : "") + k;
    return result;
}

std::string WriteFullFeature(Feature feature)
{
    std::stringstream out;

    out << "/* " << feature.name << " */;" << std::endl;
    for (auto mappedPrototype : feature.mapped)
    {
        out << mappedPrototype.second.name << " __" << mappedPrototype.first.name << " = 0; ";
        out << mappedPrototype.first.decl << " { if (__" << mappedPrototype.first.name << " != 0) " << (mappedPrototype.second.returnType != "void" ? "return " : "") << "(__" << mappedPrototype.first.name << ")(";
        out << join(", ", mappedPrototype.second.params);
        out << "); " << (mappedPrototype.second.returnType != "void" ? "return 0;" : "") << " }" << std::endl;
    }

    out << "GLboolean __load" << feature.name << "()" << std::endl;
    out << "{" << std::endl;
    out << "    GLboolean r = GL_FALSE;" << std::endl;
    for (auto j : feature.mapped) out << "    r = ((__" << j.first.name << " = (" << j.second.name << ")glExt_GetProcAddress((const GLubyte*)\"" << j.first.name << "\")) == NULL) || r;" << std::endl;
    out << "    return r;" << std::endl;
    out << "}" << std::endl;
    out << "static GLboolean __isLoaded" << feature.name << " = GL_FALSE;" << std::endl << std::endl;

    return out.str();
}

std::string WriteAllFeatures(const std::vector<Feature>& features)
{
    std::string result;
    for (auto feature : features) result += WriteFullFeature(feature);
    return result;
}

std::vector<std::string> WriteFeaturesForLoadAll(const std::vector<Feature>& features)
{
    std::vector<std::string> out;
    for (auto feature : features)
        out.push_back(std::string("__isLoaded") + feature.name + " = __load" + feature.name + "();");
    return out;
}

std::vector<std::string> WriteFeaturesFor_LoadOne(const std::vector<Feature>& features)
{
    std::vector<std::string> out;
    for (auto feature : features)
        out.push_back(std::string("if(strcmp(name,\"") + feature.name + "\") == 0) return __isLoaded" + feature.name + " = __load" + feature.name + "();");
    return out;
}

std::vector<std::string> WriteFeaturesFor_IsLoaded(const std::vector<Feature>& features)
{
    std::vector<std::string> out;
    for (auto feature : features)
        out.push_back(std::string("if(strcmp(name,\"") + feature.name + "\") == 0) return __isLoaded" + feature.name + ";");
    return out;
}

std::string WriteFeatures(const std::vector<Feature>& features)
{
    std::stringstream out;

    // Write Headers and proc loader
    Writer()
            .Statement("PFNGLGETPROC* __glExt_GetProcAddress = 0;")
            .Statement("void* glExt_GetProcAddress(const GLubyte* name)")
            .Enter(Writer()
                   .Statement("if(__glExt_GetProcAddress != 0) return (*__glExt_GetProcAddress)(name);")
                   .Statement("return 0;")
                   )
            .EmptyLine()

            // Write Foreach Extension
            .Statement(WriteAllFeatures(features))
            .EmptyLine()

            // Write glExtLoadAll
            .Statement("GLboolean glExtLoadAll(PFNGLGETPROC* proc)")
            .Enter(Writer()
                   .Statement("__glExt_GetProcAddress = proc;")
                   .Statement(WriteFeaturesForLoadAll(features))
                   .Statement("return GL_TRUE;")
                   )
            .EmptyLine()

            // Write glExtLoadOne
            .Statement("GLboolean glExtLoadOne(PFNGLGETPROC* proc, const char* name)")
            .Enter(Writer()
                   .Statement("__glExt_GetProcAddress = proc;")
                   .Statement(WriteFeaturesFor_LoadOne(features))
                   .Statement("return GL_FALSE;")
                   )
            .EmptyLine()

            // Write glExtIsLoaded
            .Statement("GLboolean glExtIsLoaded(const char* name)")
            .Enter(Writer()
                   .Statement(WriteFeaturesFor_IsLoaded(features))
                   .Statement("return GL_FALSE;")
                   )
            .Write(out);

    return out.str();
}


////////////////////////////////////////////////////////////////////////////////////////////
/// Writer implementation
////////////////////////////////////////////////////////////////////////////////////////////
static Writer empty;

Writer::StatementChild::StatementChild(Writer& scope) : _scope(scope), _type(1) { }
Writer::StatementChild::StatementChild(const std::string& statement) : _scope(empty), _statement(statement), _type(0) { }
Writer::StatementChild::StatementChild(const std::string& statement, Writer& scope, int t) : _scope(scope), _statement(statement), _type(t) { }

Writer& Writer::IfDef(const std::string& statement, Writer& child)
{
    this->_children.push_back(StatementChild(statement, child, 2));
    return *this;
}

Writer& Writer::IfNotDef(const std::string& statement, Writer& child)
{
    this->_children.push_back(StatementChild(statement, child, 3));
    return *this;
}

Writer& Writer::Statement(const std::string &statement)
{
    this->_children.push_back(StatementChild(statement));
    return *this;
}

Writer& Writer::Statement(const Writer& writer)
{
    std::stringstream out;
    writer.Write(out);
    return this->Statement(out.str());
}

Writer& Writer::Statement(const std::vector<std::string> &statements)
{
    for (auto statement : statements)
        this->_children.push_back(StatementChild(statement));
    return *this;
}

Writer& Writer::EmptyLine()
{
    this->_children.push_back(StatementChild(""));
    return *this;
}

Writer& Writer::Enter(Writer& child)
{
    this->_children.push_back(StatementChild(child));
    return *this;
}

void Writer::Write(std::ostream& out, int depth) const
{
    for (auto child : this->_children)
    {
        if (child._type == 0)
            out << writeTabs(depth) << child._statement << std::endl;
        else if (child._type == 1)
        {
            out << writeTabs(depth) << "{" << std::endl;
            child._scope.Write(out, depth + 1);
            out << writeTabs(depth) << "}" << std::endl;
        }
        else if (child._type == 2)
        {
            out << writeTabs(depth) << "#ifdef " << child._statement << std::endl;
            child._scope.Write(out, depth);
            out << writeTabs(depth) << "#endif // " << child._statement << std::endl;
        }
        else if (child._type == 3)
        {
            out << writeTabs(depth) << "#ifndef " << child._statement << std::endl;
            child._scope.Write(out, depth);
            out << writeTabs(depth) << "#endif // " << child._statement << std::endl;
        }
    }
}
