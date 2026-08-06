#include "Client.hpp"

#include <unistd.h>

Client::Client(int fd)
	: _fd(fd)
{
}

Client::~Client()
{
	if (_fd >= 0)
		close(_fd);
}

int Client::getFd() const
{
	return _fd;
}

//buffer operations
void Client::appendOutput(
	const char *data,
	std::size_t length)
{
	_outputBuffer.append(data, length);
}

bool Client::hasOutput() const
{
	return !_outputBuffer.empty();
}

const char *Client::getOutputData() const
{
	return _outputBuffer.data();
}

std::size_t Client::getOutputSize() const
{
	return _outputBuffer.size();
}

void Client::consumeOutput(std::size_t length)
{
	if (length >= _outputBuffer.size())
		_outputBuffer.clear();
	else
		_outputBuffer.erase(0, length);
}

void Client::appendInput(
	const char *data,
	std::size_t length)
{
	_inputBuffer.append(data, length);
}

std::size_t Client::getInputSize() const
{
	return _inputBuffer.size();
}

//extract line
bool Client::popLine(std::string &line)
{
	const std::size_t newline = _inputBuffer.find('\n');

	if (newline == std::string::npos)
		return false;

	line = _inputBuffer.substr(0, newline);
	_inputBuffer.erase(0, newline + 1);

	if (!line.empty()
		&& line[line.size() - 1] == '\r')
	{
		line.erase(line.size() - 1);
	}

	return true;
}