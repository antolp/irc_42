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
// class Channel;
// class Command;

// //Server should owns the listening socket, every Client, every Channel

class Server
{
public:
    Server(unsigned short port, const std::string &password);
    ~Server();

    void run();
    // static void SignalHandlers();

private:
	//the server shouldn't be copiable or initilized multiple times for obvious reasons
	//OCF isn't required anywa
    Server(const Server &other);
	Server &operator=(const Server &other);

	//process signals
    static volatile sig_atomic_t _stopRequested;
    static void handleSignal(int signalNumber);
	void installSignalHandlers();


	//socket
	void createListeningSocket(unsigned short port);
	bool receiveFromClient(std::size_t index);
	void setNonBlocking(int fd);
	void addPollFd(int fd, short events);
	void acceptClient();
	void removeClient(std::size_t index);
	Client *findClient(int fd);

	
	//send experiment
	void queueBroadcast(int senderFd, const char* data, std::size_t length);
	bool flushClientOutput(std::size_t index);
	
	// void Server::closeListeningSocket(unsigned short port);

	int                             _listenerFd;
	std::vector<struct pollfd> 		_pollFds;
    std::string                     _password;

	//temp outgoing-data storage indexed by client descriptor
	//later Client::_outputBuffer.
	std::map<int, std::string> _outputBuffers;
    std::map<int, Client *>         _clients;
    // std::map<std::string, Channel *> _channels;
};

#endif
