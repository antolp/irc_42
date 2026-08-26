#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"

namespace
{
	const std::size_t MAX_PENDING_OUTPUT =
		8 * 1024 * 1024;
}

//sends part of a client's queued output after poll() reports POLLOUT
//false when client should be disconnected
//Now using Client output_buffer
bool Server::flushClientOutput(std::size_t index)
{
	const int fd = _pollFds[index].fd;
	Client   *client = findClient(fd);

	if (client == NULL)
		return false;

	if (!client->hasOutput())
	{
		_pollFds[index].events &= ~POLLOUT;
		return true;
	}

	const ssize_t sent = send(
		fd,
		client->getOutputData(),
		client->getOutputSize(),
		0
	);

	if (sent <= 0)
	{
		std::cerr
			<< "send() failed for fd "
			<< fd
			<< std::endl;

		client->requestDisconnect("Write error");
		return false;
	}

	client->consumeOutput(
		static_cast<std::size_t>(sent)
	);

	if (!client->hasOutput())
		_pollFds[index].events &= ~POLLOUT;

	return true;
}

//queue raw bytes to client then enable POLLOUT so it can be sent
//true if all supplied bytes were queued
bool Server::queueRaw(Client &client, const char *data, std::size_t length)
{
	if (client.isDisconnectRequested())
		return  false;

	if (length == 0)
		return true;

	if (length > MAX_PENDING_OUTPUT 
		|| client.getOutputSize() > MAX_PENDING_OUTPUT - length)
	{
		client.requestDisconnect("Exceeded Client OutputBuffer");
		return false;
	}

	client.appendOutput(data, length);
	for (std::size_t i = 0; i < _pollFds.size(); ++i)
	{
		if (_pollFds[i].fd == client.getFd())
		{
			_pollFds[i].events |= POLLOUT;
			return true;
		}
	}
	return true;
}

//true if the complete CRLF-framed line was queued
bool Server::queueLine(Client &client, const std::string &line)
{
	std::string message = line;

	message += "\r\n";
	//needs to be deleted or added to a debug class
	std::cout << "\t Queuing message to " << client.getFd() << " : ``" << message;
	return (queueRaw(client, message.data(), message.size()));
}

//queues received line for every client except the sender
//now makes us of the Client class
//now queues lines instead of raw bytes
//ignoring queueline answer for now because one failed client should not prevent
//delivery to other clients
void Server::queueBroadcastLine(int senderFd, const std::string &line)
{
	for (std::map<int, Client *>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		Client &target = *it->second;

		if (target.getFd() == senderFd)
			continue;

		queueLine(target, line);
	}
}

void Server::handleCompleteLine(
	Client &client,
	const std::string &line)
{
	Command command(line);

	if (!command.isValid())
	{
		std::cerr
			<< "Malformed command from fd "
			<< client.getFd()
			<< std::endl;

		return;
	}

	//needs to be deleted or added to a debug class
	std::cout
		<< "Command from fd "
		<< client.getFd()
		<< ": "
		<< command.getName()
		<< "\n\t";

	const std::vector<std::string> &parameters =
		command.getParameters();

	for (std::size_t i = 0; i < parameters.size(); ++i)
	{
		std::cout
			<< " ["
			<< parameters[i]
			<< "]";
	}
	//

	std::cout << std::endl;

	// std::cout
	// 	<< "Complete line from fd "
	// 	<< client.getFd()
	// 	<< ": "
	// 	<< line
	// 	<< std::endl;

	// queueBroadcastLine(client.getFd(), line);
	dispatchCommand(client, command);
}

//helper to send server answers that have a numeric code
bool Server::sendNumeric( Client &client, const std::string &numeric, 
	const std::string &arguments)
{
	std::string replyTarget;
	if (client.hasNickname())
		replyTarget = client.getNickname();
	else 
		replyTarget = "*";
	std::string reply =
		":ircserv "
		+ numeric
		+ " "
		+ replyTarget;

	if (!arguments.empty())
		reply += " " + arguments;

	return queueLine(client, reply);
}