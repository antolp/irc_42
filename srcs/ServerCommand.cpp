#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

namespace
{
	const char *SERVER_NAME = "ircserv";
}

void Server::dispatchCommand(Client &client, const Command &command)
{
	const std::string &name = command.getName();

	if (name == "PING")
		handlePing(client, command);
	else if (name == "QUIT")
		handleQuit(client, command);
	else if (name == "CAP")
		handleCap(client, command);
	else
		handleUnknownCommand(client, command);
}

void Server::handleUnknownCommand(
	Client &client,
	const Command &command)
{
	std::cout
		<< "Unknown command from fd "
		<< client.getFd()
		<< ": "
		<< command.getName()
		<< std::endl;
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

	const std::vector<std::string> &parameters =
		command.getParameters();

	queueLine(
		client,
		std::string(":")
			+ SERVER_NAME
			+ " PONG "
			+ SERVER_NAME
			+ " :"
			+ parameters[0]
	);
}

//QUIT
//lets clients quit the server, ending the connection
void Server::handleQuit(
	Client &client,
	const Command &command)
{
	(void)command;

	client.requestDisconnect();
}

//CAP
//not mandatory and not definitive but clients seem to refuse connections
//when the standard response is not given
//clients send "CAP LS 302" at connection and require a normal answers
void Server::handleCap(Client &client, const Command &command)
{
	const std::vector<std::string> &parameters =
		command.getParameters();

	if (parameters.empty())
		return;

	if (parameters[0] == "LS")
	{
		queueLine(
			client,
			":ircserv CAP * LS : (lol)"
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
			":ircserv CAP * NAK : (?)" + requested
		);
	}
}