#include "Server.hpp"
#include "Client.hpp"

//at file scope just to limit endless line and crashing memory (from test 6)
//(not definitive)
namespace
{
	const std::size_t MAX_PENDING_INPUT =
		64 * 1024;
}

//Create, configure, bind, and start the TCP listening socket
//Every failure closes the temporary descriptor before throwing, so partial startup shouldn't leak a file descriptor
void Server::createListeningSocket(unsigned short port)
{
	struct sockaddr_in	address;
	int					reuse = 1;
	int					fd = -1;

	//socket() syscall : asking the kernel for an IPv4 TCP endpoint
	//SOCK_STREAM selects ordered TCP byte-stream communication (from bircd)
	fd  = socket(AF_INET, SOCK_STREAM, 0);
	if (fd  == -1)
		throw std::runtime_error("socket() failed");

	//setsockopt() syscall : SO_REUSEADDR lets a recently restarted
	//development server bind the same port without waiting for old TCP state.
	if (setsockopt(fd , SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		close(fd );
		throw std::runtime_error("setsockopt() failed");
	}

	//fcntl() syscall : changes fd behavior
	//exact form required by the subject is F_SETFL, O_NONBLOCK
	if (fcntl(fd , F_SETFL, O_NONBLOCK) == -1)
	{
		close(fd );
		throw std::runtime_error("fcntl() failed on listening socket");
	}

	//htonl host to network long
	//htons host tO network short
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	//bind() syscall : assigns the requested port on every local interface (INADDR_ANY) to this socket
	//after socket() is called, you need to bind it to an interface or whatever
	if (bind(fd , reinterpret_cast<struct sockaddr *>(&address),
			 sizeof(address)) == -1)
	{
		close(fd );
		throw std::runtime_error("bind() failed; port may already be in use");
	}

	//listen() syscall : turns the bound socket into a passive socket
	//The kernel may queue up to SOMAXCONN connections (from man listen)
	if (listen(fd , SOMAXCONN) == -1)
	{
		close(fd );
		throw std::runtime_error("listen() failed");
	}
	_listenerFd = fd;
	
	//for now no client class
	addPollFd(_listenerFd, POLLIN);

	std::cout
		<< "Listening on 0.0.0.0:" 
		<< port 
		<< std::endl;
}


//Accepts one pending connection after poll() reports POLLIN on the
//listening socket, makes the new socket non-blocking, and registers it
//
//(see Server::run())
void Server::acceptClient()
{
	struct sockaddr_in clientAddress;
	socklen_t          addressSize;
	int                clientFd;
	Client            *client;

	std::memset(&clientAddress, 0, sizeof(clientAddress));
	addressSize = sizeof(clientAddress);

	clientFd = accept(
		_listenerFd,
		reinterpret_cast<struct sockaddr *>(&clientAddress),
		&addressSize
	);

	if (clientFd == -1)
	{
		std::cerr << "accept() failed" << std::endl;
		return;
	}

	client = NULL;

	try
	{
		setNonBlocking(clientFd);

		if (_clients.find(clientFd) != _clients.end())
		{
			throw std::runtime_error(
				"accepted fd already belongs to a client"
			);
		}
		//Client object now represents this connected socket
		client = new Client(clientFd);

		_clients.insert(std::make_pair(clientFd, client));
		addPollFd(clientFd, POLLIN);
	}
	catch (...)
	{
		_clients.erase(clientFd);
		if (client != NULL)
			delete client;
		else
			close(clientFd);
		throw;
	}

	std::cout
		<< "New client connected: fd "
		<< client->getFd()
		<< std::endl;
}

//closes the client descriptor at index and removes its pollfd entry
//then clean client from IRC (channels, invite, nick ETC)
//then delete client
void Server::removeClient(std::size_t index)
{
    const int fd = _pollFds[index].fd;

    std::map<int, Client *>::iterator client =
        _clients.find(fd);

    if (client != _clients.end())
    {
        Client *removed = client->second;

        cleanupClientIrcState(*removed);

        _clients.erase(client);
        delete removed;
    }

    _pollFds.erase(_pollFds.begin() + index);

    std::cout
        << "Removed client fd "
        << fd
        << std::endl;
}

//Reads currently available bytes from Client from index in _pollFds 
//Returns false when the connection must be removed
//then extract line and lets server handle it (for now just broadcast it to all clients)
bool Server::receiveFromClient(std::size_t index)
{
	const int fd = _pollFds[index].fd;
	Client   *client = findClient(fd);
	char      buffer[1024];

	if (client == NULL)
		return false;

	//recv() syscall : copies currently available TCP bytes into given buffer 
	//It is called once for POLLIN event. A non-positive result
	//ends this client;s errno is not inspected and does not control a retry (unsure)
	const ssize_t received = recv(
		fd,
		buffer,
		sizeof(buffer),
		0
	);

	if (received == 0)
	{
		std::cout
			<< "Client disconnected: fd "
			<< fd
			<< std::endl;

		return false;
	}

	if (received < 0)
	{
		std::cerr
			<< "recv() failed for fd "
			<< fd
			<< std::endl;

		return false;
	}

	client->appendInput(
		buffer,
		static_cast<std::size_t>(received)
	);

	std::string line;

	while (client->popLine(line))
	{
		handleCompleteLine(*client, line);
		//cuts the connection here, no trailing PING after QUIT if one TCP read contains both
		if (client->isDisconnectRequested())
			break;
	}

	if (client->getInputSize() > MAX_PENDING_INPUT)
	{
		std::cerr
			<< "Input buffer limit exceeded for fd "
			<< fd
			<< std::endl;

		return false;
	}

	return true;
}