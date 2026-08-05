#include "Server.hpp"

#include "Client.hpp"

volatile sig_atomic_t Server::_stopRequested = 0;

//function NEEDS to be prototyped as static in header
//else its type goes from "void (*)(int)" to "void (Server::*)(int)"
void Server::handleSignal(int signalNumber)
{
	(void)signalNumber;
	_stopRequested = 1;
}

//Install small process-wide handlers before any Server instance is started
//SIGPIPE is ignored because it fucks with send()
//send failure rather than terminate the whole process (equivalent lol)
void Server::installSignalHandlers()
{
	struct sigaction action;
	struct sigaction ignoreAction;

	std::memset(&action, 0, sizeof(action));
	action.sa_handler = &Server::handleSignal;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction(SIGINT, &action, NULL) == -1)
		throw std::runtime_error("sigaction() failed for SIGINT");

	if (sigaction(SIGTERM, &action, NULL) == -1)
		throw std::runtime_error("sigaction() failed for SIGTERM");

	std::memset(&ignoreAction, 0, sizeof(ignoreAction));
	ignoreAction.sa_handler = SIG_IGN;
	sigemptyset(&ignoreAction.sa_mask);
	ignoreAction.sa_flags = 0;

	if (sigaction(SIGPIPE, &ignoreAction, NULL) == -1)
		throw std::runtime_error("sigaction() failed for SIGPIPE");
}


//Registers a descriptor and the events it should watch in the collection
//passed to poll(). revents will later contain the events reported by poll()
void Server::addPollFd(int fd, short events)
{
	//DEBUG
	std::cout << "adding " << fd << " " << events << " at addPollFd" << std::endl;

	for (std::size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd == fd)
			throw std::runtime_error(
				"attempted to register the same fd twice !!!"
			);
	}
	
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
	_stopRequested = 0;
	installSignalHandlers();
	createListeningSocket(port);
	static_cast<void>(password);
}

Server::~Server()
{
	//destroy clients
	//destroy channels
	//respective class responsible of releasing of their socket's fds (clients)

	//releases the kernel's listening socket
	for (std::size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd != -1)
			close(_pollFds[i].fd);
	}
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
	while (!_stopRequested)
	{
		int readyCount = poll(
			&_pollFds[0],
			_pollFds.size(),
			-1
		);

		if (readyCount == -1)
		{
			if (_stopRequested)
				break;

			throw std::runtime_error("poll() failed");
		}

		if (readyCount == 0)
			continue;

		for (std::size_t i = 0;
			 i < _pollFds.size() && !_stopRequested;)
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

			if (events & POLLOUT)
			{
				if (!flushClientOutput(i))
				{
					removeClient(i);
					continue;
				}
			}

			++i;
		}
	}

	std::cout << std::endl
			  << "Shutdown requested, closing server!"
			  << std::endl;
}