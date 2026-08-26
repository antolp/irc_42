#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

volatile sig_atomic_t Server::_stopRequested = 0;

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
	//destroy clients (responsible for deleting their FDs)
	for (std::map<int, Client *>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();
	//destroy channels

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
//then accept(), recv(), send()
//POLLHUP at the end because on certain condition the client could be disconnected
//before sending its final bytes
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

			if (events & (POLLERR | POLLNVAL))
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

			if (events & POLLHUP)
			{
				removeClient(i);
				continue;
			}

			++i;
		}
		removeDisconnectedClients();
	}

	std::cout << std::endl
			  << "Shutdown requested, closing server!"
			  << std::endl;
}

void Server::removeDisconnectedClients()
{
	for (std::size_t i = 0; i < _pollFds.size();)
	{
		if (_pollFds[i].fd == _listenerFd)
		{
			++i;
			continue;
		}

		Client *client = findClient(_pollFds[i].fd);

		if (client != NULL
			&& client->isDisconnectRequested())
		{
			removeClient(i);
			continue;
		}

		++i;
	}
}


void Server::cleanupClientIrcState(Client &client)
{
	(void)client;
}

//shared one-channel membership removal for KICK/PART
//caller must NOT use channel after this call if it was the last member
//(dangling pointer)
void Server::removeMemberFromChannel(Channel *channel, int fd)
{
	if (channel == NULL)
		return;

	channel->removeMember(fd);

	if (!channel->empty())
		return;

	_channels.erase(channel->getName());
	delete channel;
}