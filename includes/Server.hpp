#ifndef SERVER_HPP
# define SERVER_HPP

# include <csignal>
# include <map>
# include <set>
# include <string>
# include <vector>


# include <arpa/inet.h>
# include <cctype>
# include <cstring>
# include <fcntl.h>
# include <iostream>
# include <netinet/in.h>
# include <poll.h>
# include <stdexcept>
# include <sys/socket.h>
# include <unistd.h>


// //forward declaration
class Client;
class Channel;
class Command;

//Server should owns the listening socket, every Client, every Channel
class Server
{
public:
	Server(unsigned short port, const std::string &password);
	~Server();

	void run();

private:
	Server(const Server &other);
	Server &operator=(const Server &other);

	//process signals
	static volatile sig_atomic_t _stopRequested;
	static void handleSignal(int signalNumber);
	void installSignalHandlers();

	//socket
	void	createListeningSocket(unsigned short port);
	bool	receiveFromClient(std::size_t index);
	void	setNonBlocking(int fd);
	void	addPollFd(int fd, short events);
	void	acceptClient();
	void	removeClient(std::size_t index);
	Client	*findClient(int fd);
	void	removeDisconnectedClients();

	//send
	void		queueBroadcastLine(int senderFd, const std::string &line);
	bool		queueRaw(Client &client, const char *data, std::size_t length);
	bool		queueLine(Client &client, const std::string &line);	
	bool		flushClientOutput(std::size_t index);
	bool		sendNumeric(Client &client, const std::string &numeric, const std::string &arguments);

	// Command processing
	void handleCompleteLine(Client &client, const std::string &line);
	void dispatchCommand(Client &client, const Command &command);
	void handlePing(Client &client, const Command &command);
	void handleQuit(Client &client, const Command &command);
	void handleCap(Client &client, const Command &command);
	void handlePass(Client &client, const Command &command);
	void handleNick(Client &client, const Command &command);
	void handleUser(Client &client, const Command &command);
	void handlePrivmsg(Client &client, const Command &command);
	void handleJoin(Client &client, const Command &command);
	void handleTopic(Client &client, const Command &command);
	void handleKick(Client &client, const Command &command);
	void handleInvite(Client &client, const Command &command);
	void handleUnknownCommand(Client &client, const Command &command);

	//Utilities
	Client *findClientByNickname(const std::string &nickname);
	Channel *findChannel(const std::string &name);
	void	tryCompleteRegistration(Client &client);
	bool	isValidNickname(const std::string &nickname) const;
	void	SendWelcome(Client &client);
	void    broadcastToChannel(const Channel &channel, const std::string &message, int excludeFd = -1);
	void    sendChannelNames(Client &client, const Channel &channel);

	int                             _listenerFd;
	std::vector<struct pollfd> 		_pollFds;
	std::string                     _password;

	std::map<int, Client *>         _clients;
	std::map<std::string, int>		_nicknameIndex;

	std::map<std::string, Channel *> _channels;
};

#endif
