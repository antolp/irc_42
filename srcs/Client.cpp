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