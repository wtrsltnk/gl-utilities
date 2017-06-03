#ifndef GLEXTLOADER_H
#define GLEXTLOADER_H

#define MAX_EXTENSIONS 1024

#include <map>
#include <string>
#include <vector>
#include <fstream>

class Prototype
{
public:
    Prototype();
    Prototype(const Prototype& other);

    bool operator < (const Prototype other) const;

    std::string name;
    std::string decl;
};

class TypeDefinition
{
public:
    TypeDefinition();
    TypeDefinition(const TypeDefinition& other);

    std::string name;
    std::string returnType;
    std::vector<std::string> params;
};

class Feature
{
public:
    std::string name;
    std::map<Prototype, TypeDefinition> mapped;
};

class Writer
{
    class StatementChild
    {
    public:
        StatementChild(Writer& scope);
        StatementChild(const std::string& statement);
        StatementChild(const std::string& statement, Writer& scope, int t = 2);

        int _type;   // 0 == statement, 1 == scope, 2 == ifdef, 3 == ifndef
        Writer& _scope;
        std::string _statement;
    };

    std::vector<StatementChild> _children;

public:
    Writer& IfDef(const std::string& statement, Writer& child);
    Writer& IfNotDef(const std::string& statement, Writer& child);
    Writer& Statement(const std::string& statement);
    Writer& Statement(const Writer& writer);
    Writer& Statement(const std::vector<std::string>& statements);
    Writer& EmptyLine();
    Writer& Enter(Writer& child);

    void Write(std::ostream& out, int depth = 0) const;

};

std::vector<Feature> LoadFeatures(std::ifstream& in);
std::string WriteFeatures(const std::vector<Feature>& features);

#endif // GLEXTLOADER_H
