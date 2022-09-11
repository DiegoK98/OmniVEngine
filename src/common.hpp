#pragma once

#include "defines.hpp"

namespace OmniV
{
    std::vector<std::string> tokenize(const std::string& string, const std::string& delim = ", ", bool includeEmpty = false);

    std::string toLower(const std::string& value);

    bool toBool(const std::string& str);

    int toInt(const std::string& str);

    unsigned int toUInt(const std::string& str);

    float toFloat(const std::string& str);

    Vector2f toVector2f(const std::string& str);

    Vector3f toVector3f(const std::string& str);

    Vector4f toVector4f(const std::string& str);
}
