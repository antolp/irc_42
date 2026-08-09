#include "Server.hpp"
#include "Client.hpp"

//function NEEDS to be prototyped as static in header
//else its type goes from "void (*)(int)" to "void (Server::*)(int)"
void Server::handleSignal(int signalNumber)
{
	(void)signalNumber;
	_stopRequested = 1;
}

//Install small process singal handlers before any Server instance is ran
//SIGPIPE is ignored because it fucks with send()
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

//returns Client * from FD
Client *Server::findClient(int fd)
{
	std::map<int, Client *>::iterator it =
		_clients.find(fd);

	if (it == _clients.end())
		return NULL;

	return it->second;
}

Client *Server::findClientByNickname(const std::string &nickname)
{
	for (std::map<int, Client *>::iterator it = _clients.begin(); 
		it != _clients.end(); it++)
	{
		if (it->second->getNickname() == nickname)
			return it->second;
	}
	return NULL;
}

//not complete
bool Server::isValidNickname(const std::string &nickname) const
{
	if (nickname.empty())
		return false;

	if (nickname[0] == '#' || nickname[0] == '&' || nickname[0] == ':')
		return false;

	for (std::size_t i = 0; i < nickname.size(); ++i)
	{
		if (nickname[i] == ' '
			|| nickname[i] == ','
			|| nickname[i] == '\r'
			|| nickname[i] == '\n'
			|| nickname[i] == '\t')
		{
			return false;
		}
	}

	return true;
}

//missing some stuff
//No multi-lin sendNumeric() calls here because that would just take too much space
void	Server::SendWelcome(Client &client)
{
	sendNumeric(client, "001", ":Welcome to this really awesome IRC server !" + client.getPrefix());
	sendNumeric(client, "002", ":This server has been running for (idk)");
	sendNumeric(client, "003", ":here's a nice snail in ASCII art :");
	sendNumeric(client, "004", ":   ▄▄ ▄████▄▐▄▄▄▌");
	sendNumeric(client, "005", ":  ▐  ████▀███▄█▄▌");
	sendNumeric(client, "006", ":▐ ▌  █▀▌  ▐▀▌▀█▀ ");
	sendNumeric(client, "007", ": ▀   ▌ ▌  ▐ ▌    ");
	sendNumeric(client, "008", ":     █ █  ▐▌█    ");
	sendNumeric(client, "376", ":End of MOTD command");
}