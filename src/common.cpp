#include "common.hpp"

namespace OmniV
{
	std::vector<std::string> tokenize(const std::string& string, const std::string& delim, bool includeEmpty)
	{
		std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
		std::vector<std::string> tokens;

		while (lastPos != std::string::npos)
		{
			if (pos != lastPos || includeEmpty)
			{
				tokens.push_back(string.substr(lastPos, pos - lastPos));
			}

			lastPos = pos;

			if (lastPos != std::string::npos)
			{
				lastPos += 1;
				pos = string.find_first_of(delim, lastPos);
			}
		}

		return tokens;
	}

	std::string toLower(const std::string& value)
	{
		std::string result;
		result.resize(value.size());
		std::transform(value.begin(), value.end(), result.begin(), ::tolower);

		return result;
	}

	bool toBool(const std::string& str)
	{
		std::string value = toLower(str);
		if (value == "false")
		{
			return false;
		}
		else if (value == "true")
		{
			return true;
		}
		else
		{
			throw std::runtime_error("Could not parse boolean value \"" + str + "\"");
		}
	}

	int toInt(const std::string& str) {
		char* end_ptr = nullptr;
		int result = (int)strtol(str.c_str(), &end_ptr, 10);

		if (*end_ptr != '\0')
		{
			throw std::runtime_error("Could not parse integer value \"" + str + "\"");
		}

		return result;
	}

	unsigned int toUInt(const std::string& str) {
		char* end_ptr = nullptr;
		unsigned int result = (int)strtoul(str.c_str(), &end_ptr, 10);

		if (*end_ptr != '\0')
		{
			throw std::runtime_error("Could not parse integer value \"" + str + "\"");
		}

		return result;
	}

	float toFloat(const std::string& str)
	{
		char* end_ptr = nullptr;
		float result = (float)strtof(str.c_str(), &end_ptr);

		if (*end_ptr != '\0')
		{
			throw std::runtime_error("Could not parse floating point value \"" + str + "\"");
		}

		return result;
	}

	Vector2f toVector2f(const std::string& str) {
		std::vector<std::string> tokens = tokenize(str);
		if (tokens.size() != 2)
		{
			throw std::runtime_error("Expected 2 values");
		}

		return Vector2f(toFloat(tokens[0]), toFloat(tokens[1]));
	}

	Vector3f toVector3f(const std::string& str) {
		std::vector<std::string> tokens = tokenize(str);
		if (tokens.size() != 3)
		{
			throw std::runtime_error("Expected 3 values");
		}

		return Vector3f(toFloat(tokens[0]), toFloat(tokens[1]), toFloat(tokens[2]));
	}

	Vector4f toVector4f(const std::string& str) {
		std::vector<std::string> tokens = tokenize(str);
		if (tokens.size() != 4)
		{
			throw std::runtime_error("Expected 4 values");
		}

		return Vector4f(toFloat(tokens[0]), toFloat(tokens[1]), toFloat(tokens[2]), toFloat(tokens[3]));
	}
};