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