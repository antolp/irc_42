#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Utils.hpp"

volatile sig_atomic_t Server::_stopRequested = 0;

Server::Server(unsigned short port, const std::string &password) : 
	_listenerFd(-1),
	_password(password),
	_debug(0)
{
	//open sockets at init
	_stopRequested = 0;
	installSignalHandlers();
	createListeningSocket(port);
	static_cast<void>(password);
}

Server::~Server()
{
	// Channels can contain references to client fds, so destroy them first.
	for (std::map<std::string, Channel *>::iterator it = _channels.begin();
		 it != _channels.end(); ++it)
	{
		delete it->second;
	}
	_channels.clear();

	// Client owns its connected socket and closes it in its destructor.
	for (std::map<int, Client *>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		delete it->second;
	}
	_clients.clear();
	_nicknameIndex.clear();
	_pollFds.clear();

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



//removes every IRC-level reference to a client while the Client object is still alive. 
//This must happen before the client is erased/deleted so an fd
//	reused by a future connection cannot inherit channel membership/operator
//	state, and a reused nickname cannot inherit an invitation.
void Server::cleanupClientIrcState(Client &client)
{
	const int fd = client.getFd();
	const std::string nickname = client.getNickname();
	std::set<int> quitRecipients;

	//Build the union of clients sharing at least one channel with this client.
	//std::set makes sure a peer sharing several channels receives QUIT only once
	for (std::map<std::string, Channel *>::iterator it = _channels.begin();
		 it != _channels.end(); ++it)
	{
		Channel *channel = it->second;

		if (!channel->hasMember(fd))
			continue;

		const Channel::MemberMap &members = channel->getMembers();
		for (Channel::MemberMap::const_iterator member = members.begin();
			 member != members.end(); ++member)
		{
			if (member->first != fd)
				quitRecipients.insert(member->first);
		}
	}
	//queueline to quitRecipients
	if (client.isRegistered())
	{
		std::string reason = client.getDisconnectReason();
		if (reason.empty())
			reason = "Connection closed";

		const std::string quitMessage =
			":" + client.getPrefix() + " QUIT :" + reason;

		for (std::set<int>::iterator it = quitRecipients.begin();
			 it != quitRecipients.end(); ++it)
		{
			Client *target = findClient(*it);
			if (target != NULL)
				queueLine(*target, quitMessage);
		}
	}

	//remove membership and stale invitations, thenb delete empty channels
	for (std::map<std::string, Channel *>::iterator it = _channels.begin();
		 it != _channels.end(); )
	{
		Channel *channel = it->second;

		if (!nickname.empty())
			channel->removeInvite(nickname);
		channel->removeMember(fd);

		if (channel->empty())
		{
			delete channel;
			_channels.erase(it++);
		}
		else
		{
			++it;
		}
	}

	//remove the nickname only if the index entry still belongs to this fd
	if (!nickname.empty())
	{
		std::map<std::string, int>::iterator nickIt =
			_nicknameIndex.find(ircCaseFold(nickname));

		if (nickIt != _nicknameIndex.end() && nickIt->second == fd)
			_nicknameIndex.erase(nickIt);
	}
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

	_channels.erase(ircCaseFold(channel->getName()));
	delete channel;
}