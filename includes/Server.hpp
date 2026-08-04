#ifndef SERVER_HPP
# define SERVER_HPP

# include <csignal>
# include <map>
# include <set>
# include <string>
# include <vector>

//forward declaration
class Channel;
class Client;
class Command;

//Server should owns the listening socket, every Client, every Channel

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

	int                             _listenFd;
    std::string                     _password;
    std::map<int, Client *>         _clients;
    std::map<std::string, Channel *> _channels;
};

#endif

//to add :
    // void dispatch(Client &client, const std::string command);
    // void handlePass(Client &client, const std::string command);
    // void handleNick(Client &client, const std::string command);
    // void handleUser(Client &client, const std::string command);
    // void handlePing(Client &client, const std::string command);
    // void handleJoin(Client &client, const std::string command);
    // void handlePart(Client &client, const std::string command);
    // void handleMessage(Client &client, const std::string command);
    // void handleTopic(Client &client, const std::string command);
    // void handleKick(Client &client, const std::string command);

	//eventually command should probably be its own class