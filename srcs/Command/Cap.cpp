#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

//CAP
//clients send "CAP LS 302" at connection and require a normal answers
void Server::handleCap(Client &client, const Command &command)
{
	const std::vector<std::string> &parameters = command.getParameters();

	if (parameters.empty())
		return;

	if (parameters[0] == "LS")
	{
		queueLine(
			client,
			":ircserv CAP * LS"
		);
		return;
	}

	if (parameters[0] == "REQ")
	{
		std::string requested;

		if (parameters.size() >= 2)
			requested = parameters[1];

		queueLine(
			client,
			":ircserv CAP * NAK" + requested
		);
	}
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}

