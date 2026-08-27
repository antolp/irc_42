#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

namespace
{
	const char *SERVER_NAME = "ircserv";
}

//PING
//lets clients and server to verify that the connection is still active
//client : "PING :token"
//server : ":ircserv PONG ircserv :token"
void Server::handlePing(
	Client &client,
	const Command &command)
{
	if (command.getParameterCount() == 0)
	{
		queueLine(
			client,
			":ircserv 409 * :No origin specified"
		);
		return;
	}

	const std::vector<std::string> &parameters = command.getParameters();

	queueLine(
		client,
		std::string(":")
			+ SERVER_NAME
			+ " PONG "
			+ SERVER_NAME
			+ " :"
			+ parameters[0]
	);
	//needs to be deleted or added to a debug class
	if (_debug)
		std::cout << command.getName() << " successful" << std::endl;
}
