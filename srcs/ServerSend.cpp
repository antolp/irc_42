#include "Server.hpp"
#include "Client.hpp"

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

		return false;
	}

	client->consumeOutput(
		static_cast<std::size_t>(sent)
	);

	if (!client->hasOutput())
		_pollFds[index].events &= ~POLLOUT;

	return true;
}

void Server::handleCompleteLine(
	Client &client,
	const std::string &line)
{
	std::cout
		<< "Complete line from fd "
		<< client.getFd()
		<< ": "
		<< line
		<< std::endl;

	queueBroadcastLine(client.getFd(), line);
}

//queues received line for every client except the sender
//now makes us of the Client class
//now queues lines instead of raw bytes
void Server::queueBroadcastLine(
	int senderFd,
	const std::string &line)
{
	for (std::size_t i = 0; i < _pollFds.size(); ++i)
	{
		const int targetFd = _pollFds[i].fd;

		if (targetFd == _listenerFd
			|| targetFd == senderFd)
		{
			continue;
		}

		Client *target = findClient(targetFd);

		if (target == NULL)
			continue;

		target->appendOutput(
			line.data(),
			line.size()
		);
		target->appendOutput("\r\n", 2);

		_pollFds[i].events |= POLLOUT;
	}
}