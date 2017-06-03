#include "glextloader.h"
#include <algorithm>
#include <iostream>


////////////////////////////////////////////////////////////////////////////////////////////
/// Forward declarations of helper methods
////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::string> ParseParameters(std::string p);
void replaceAll(std::string &s, const std::string &search, const std::string &replace);


////////////////////////////////////////////////////////////////////////////////////////////
/// Prototype methods
////////////////////////////////////////////////////////////////////////////////////////////
bool IsPrototype(const std::string& line)
{
    return line.substr(0, 6) == std::string("GLAPI ");
}

Prototype ReadPrototype(const std::string& line)
{
    Prototype p;

    std::string::size_type pos = line.find("APIENTRY ") + std::string("APIENTRY ").size();
    p.name = line.substr(pos, line.substr(pos).find_first_of(' '));

    // Find the opening bracket for the parameters
    pos = line.find_last_of('(') + 1;

    p.decl = line.substr(std::string("GLAPI ").size());
    p.decl = p.decl.substr(0, p.decl.size() - 1);
    if (p.decl.find("(void)") != std::string::npos)
        replaceAll(p.decl, "(void)", "()");

    return p;
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Type definition methods
////////////////////////////////////////////////////////////////////////////////////////////
bool IsTypeDefinition(const std::string& line)
{
    return line.substr(0, 8) == std::string("typedef ")
            && line.find("(APIENTRYP ") != std::string::npos;
}

TypeDefinition ReadTypeDefinition(const std::string& line)
{
    TypeDefinition t;

    std::string::size_type pos = line.find("(APIENTRYP ");
    t.returnType = line.substr(std::string("typedef ").size(), pos - std::string("typedef ").size() - 1);
    pos += std::string("(APIENTRYP ").size();
    t.name = line.substr(pos, line.substr(pos).find(')'));


    // Find the opening bracket for the parameters
    pos = line.find_last_of('(') + 1;

    // Parse the parameters
    t.params = ParseParameters(line.substr(pos, line.find_last_of(')') - pos));

    return t;
}

std::map<Prototype, TypeDefinition> MapPrototypeToTypeDefinitions(const std::vector<Prototype>& prototypes, const std::vector<TypeDefinition>& typedefinitions)
{
    std::map<Prototype, TypeDefinition> result;

    for (auto m : prototypes)
    {
        std::string upper = m.name;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        for (auto t : typedefinitions)
        {
            if (t.name.find(upper) != std::string::npos)
                result.insert(std::make_pair(m, t));
        }
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////
/// Feature methods
////////////////////////////////////////////////////////////////////////////////////////////
bool IsFeatureStart(const std::string& line)
{
    return line.substr(0, 11) == std::string("#ifndef GL_");
}

bool IsFeatureEnd(const std::string& line)
{
    return line.substr(0, 13) == std::string("#endif /* GL_");
}

bool LoadFeature(Feature& feature, std::ifstream& in, std::string line)
{
    std::vector<Prototype> prototypes;
    std::vector<TypeDefinition> typeDefinitions;

    do
    {
        if (IsFeatureStart(line))
        {
            feature.name = line.substr(line.find_first_of(' ') + 1);
        }
        else if (IsTypeDefinition(line))
        {
            typeDefinitions.push_back(ReadTypeDefinition(line));
        }
        else if (IsPrototype(line))
        {
            prototypes.push_back(ReadPrototype(line));
        }
        else if (IsFeatureEnd(line))
        {
            feature.mapped = MapPrototypeToTypeDefinitions(prototypes, typeDefinitions);

            // We are at the end of a feature, and can notify that we have found a feature
            return true;
        }
    }
    while (std::getline(in, line));

    // We are at the end of the stream, but have not found the end of the feature yet. This is an error situation
    return false;
}

std::vector<Feature> LoadFeatures(std::ifstream& in)
{
    std::vector<Feature> result;
    std::string line;

    while (std::getline(in, line))
    {
        Feature feature;
        if (LoadFeature(feature, in, line))
            result.push_back(feature);
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////
/// Helper methods
////////////////////////////////////////////////////////////////////////////////////////////
void replaceAll(std::string &s, const std::string &search, const std::string &replace)
{
    for(auto pos = 0; ; pos += replace.length())
    {
        // Locate the substring to replace
        pos = s.find( search, pos);
        if(pos == std::string::npos) break;
        // Replace by erasing and inserting
        s.erase(pos, search.length());
        s.insert(pos, replace);
    }
}

std::vector<std::string> ParseParameters(std::string p)
{
    std::vector<std::string> r;

    // Make sure we end with a comma, otherwise the last item is not parsed
    p += ",";

    while (p.find_first_of(",") != std::string::npos)
    {
        // Trim the white spaces in front
        p = p.substr(p.find_first_not_of(' '));

        // Determine the key=value pair
        std::string kvp = p.substr(0, p.find_first_of(",)["));
        std::string key = kvp.substr(kvp.find_last_of("* ") + 1);

        // Add the parameter to our list
        if (key != "void")
            r.push_back(key);

        // Remove the parameter we just added from the parameter string
        p = p.substr(p.find_first_of(",)") + 1);
    }

    return r;
}


////////////////////////////////////////////////////////////////////////////////////////////
/// Prototype and TypeDefinition implementation
////////////////////////////////////////////////////////////////////////////////////////////
Prototype::Prototype() { }

Prototype::Prototype(const Prototype& p)
    : name(p.name), decl(p.decl)
{ }

bool Prototype::operator < (const Prototype other) const
{
    return this->name < other.name;
}

TypeDefinition::TypeDefinition() { }

TypeDefinition::TypeDefinition(const TypeDefinition& other)
    : name(other.name), returnType(other.returnType), params(other.params)
{ }
