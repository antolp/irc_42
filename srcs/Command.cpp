#include "Command.hpp"

//anonymous namespace
namespace
{
	const std::size_t MAX_COMMAND_PARAMETERS = 15;

	//toupper isn't explicitely ASCII (not definitive)
	char uppercaseAscii(char character)
	{
		if (character >= 'a' && character <= 'z')
			return character - 'a' + 'A';

		return character;
	}
}

Command::Command(const std::string &line)
	: _valid(false)
{
	parse(line);
}

//semicolon important only if trailing, example :
//NICK name:blabla			-> ["name:blabla"] (which may be rejected by the NICK command handler)
//PRIVMSG name :coucou		-> ["name", "blabla"]
void Command::parse(const std::string &line)
{
	std::size_t position = 0;
	std::size_t tokenStart;

	//linient towards spaces
	while (position < line.size() && line[position] == ' ')
		++position;

	if (position == line.size())
		return;

	//client-to-server commands don't need an IRC prefix
	//leading ":" not acceptable as command name
	if (line[position] == ':')
		return;

	tokenStart = position;

	while (position < line.size() && line[position] != ' ')
		++position;

	_name = line.substr(tokenStart, position - tokenStart);

	if (_name.empty())
		return;

	normalizeName();

	while (position < line.size())
	{
		while (position < line.size() && line[position] == ' ')
			++position;

		if (position == line.size())
			break;

		if (_parameters.size() >= MAX_COMMAND_PARAMETERS)
			return;

		if (line[position] == ':')
		{
			++position;
			_parameters.push_back(line.substr(position));
			_valid = true;
			return;
		}

		tokenStart = position;

		while (position < line.size() && line[position] != ' ')
			++position;

		_parameters.push_back(
			line.substr(tokenStart, position - tokenStart)
		);
	}

	_valid = true;
}

void Command::normalizeName()
{
	for (std::size_t i = 0; i < _name.size(); ++i)
		_name[i] = uppercaseAscii(_name[i]);
}

bool Command::isValid() const
{
	return _valid;
}

const std::string &Command::getName() const
{
	return _name;
}

const std::vector<std::string> &Command::getParameters() const
{
	return _parameters;
}

std::size_t Command::getParameterCount() const
{
	return _parameters.size();
}