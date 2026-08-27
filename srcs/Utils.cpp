#include "Utils.hpp"

bool ircEquals(const std::string &left, const std::string &right)
{
	return ircCaseFold(left) == ircCaseFold(right);
}

//irssi expects RFC1459 on casemapping (couldn't setup 005)
std::string ircCaseFold(const std::string &value)
{
	std::string result = value;

	for (std::size_t i = 0; i < result.size(); ++i)
	{
		char &c = result[i];

		if (c >= 'A' && c <= 'Z')
			c = c - 'A' + 'a';
		else if (c == '[')
			c = '{';
		else if (c == ']')
			c = '}';
		else if (c == '\\')
			c = '|';
		else if (c == '~')
			c = '^';
	}

	return result;
}