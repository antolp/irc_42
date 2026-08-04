#include "Server.hpp"

#include "Client.hpp"

#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>



Server::Server(unsigned short port, const std::string &password)
	: _listenFd(-1), _password(password)
{
	//open sockets at init (RAII)
	static_cast<void>(port);
	static_cast<void>(password);
}

Server::~Server()
{
	//destroy clients
	//destroy sockets
	//class responsible of releasing of their socket's fds

	//releases the kernel's listening socket
	if (_listenFd >= 0)
		close(_listenFd);
}

void Server::run()
{
	std::cout << "nothing"<< std::endl;
}