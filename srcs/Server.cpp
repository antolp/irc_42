#include "Server.hpp"

#include "Client.hpp"

//Registers a descriptor and the events it should watch in the collection
//passed to poll(). revents will later contain the events reported by poll()
void Server::addPollFd(int fd, short events)
{
	//DEBUG
	std::cout << "adding " << fd << " " << events << " at addPollFd" << std::endl;
    struct pollfd descriptor;

    descriptor.fd = fd;
    descriptor.events = events;
    descriptor.revents = 0;

    _pollFds.push_back(descriptor);
}

Server::Server(unsigned short port, const std::string &password)
	: _listenerFd(-1),
	_password(password)
{
	//open sockets at init
	createListeningSocket(port);
	static_cast<void>(password);
}

Server::~Server()
{
	//destroy clients
	//destroy channels
	//respective class responsible of releasing of their socket's fds (clients)

	//releases the kernel's listening socket
	if (_listenerFd >= 0)
		close(_listenerFd);
}

//Configures a socket descriptor in non-blocking mode so an operation
//cannot freeze the entire server
void Server::setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl() failed");
}

//for now just to test TCP connections
//one poll() call to report fd readiness
//then accept(), recv() end eventually send() (not here yet)
void Server::run()
{
    while (true)
    {
        int readyCount = poll(&_pollFds[0], _pollFds.size(), -1);

        if (readyCount == -1)
            throw std::runtime_error("poll() failed");

        for (std::size_t i = 0; i < _pollFds.size();)
        {
            short events = _pollFds[i].revents;

            if (events == 0)
            {
                ++i;
                continue;
            }

            if (_pollFds[i].fd == _listenerFd)
            {
                if (events & POLLIN)
                    acceptClient();

                ++i;
                continue;
            }

            if (events & (POLLERR | POLLHUP | POLLNVAL))
            {
                removeClient(i);
                continue;
            }

            if (events & POLLIN)
            {
                if (!receiveFromClient(i))
                {
                    removeClient(i);
                    continue;
                }
            }

            ++i;
        }
    }
}