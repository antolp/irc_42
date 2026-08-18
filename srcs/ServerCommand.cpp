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

	// //needs to be deleted or added to a debug class
	// std::cout << "\t dispatching command :\n\t\t" << command.getName();
	// const std::vector<std::string> &parameters = command.getParameters();
	// for (std::size_t i = 0; i < parameters.size(); ++i)
	// 	std::cout << parameters[i] << ",\t";
	// std::cout << std::endl;
	// //

	const std::string &name = command.getName();

	if (name == "PING")
		handlePing(client, command);
	else if (name == "QUIT")
		handleQuit(client, command);
	else if (name == "CAP")
		handleCap(client, command);
	else if (name == "PASS")
		handlePass(client, command);
	else if (name == "USER")
		handleUser(client, command);
	else if (name == "NICK")
		handleNick(client, command);
	else if (name == "PRIVMSG")
		handlePrivmsg(client, command);
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
	std::cout << command.getName() << " successful" << std::endl;
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

//PASS
//	required for registration
//client : "PASS <password>"
//lets client supply the correct password
//on success no answer
void Server::handlePass(Client &client, const Command &command)
{
	if (client.isRegistered())
	{
		sendNumeric(
			client,
			"462",
			":You may not reregister"
		);
		return;
	}

	if (command.getParameterCount() < 1)
	{
		sendNumeric(
			client,
			"461",
			"PASS :Not enough parameters"
		);
		return;
	}

	if (command.getParameters()[0] != _password)
	{
		sendNumeric(
			client,
			"464",
			":Password incorrect"
		);
		return;
	}

	client.setPasswordAccepted();
	tryCompleteRegistration(client);
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}

//NICK
//	required for registration
//client : "NICK <nickname>"
//lets client provide a nickname the server should use
//that nickname can be reset by client
void Server::handleNick(Client &client, const Command &command)
{
	if (command.getParameterCount() < 1)
	{
		sendNumeric(
			client,
			"431",
			":No nickname given"
		);
		return;
	}

	const std::string &nickname =command.getParameters()[0];
	if (!isValidNickname(nickname))
	{
		sendNumeric(
			client,
			"432",
			nickname + " :Erroneous nickname"
		);
		return;
	}

	const std::string key = nickname;
	std::map<std::string, int>::iterator existing = _nicknameIndex.find(key);
	
	if (existing != _nicknameIndex.end() && existing->second != client.getFd())
	{
		sendNumeric(
			client,
			"433",
			nickname + " :Nickname is already in use"
		);
		return;
	}
	if (client.hasNickname())
	{
		_nicknameIndex.erase(client.getNickname());
	}

	client.setNickname(nickname);
	_nicknameIndex[key] = client.getFd();

	tryCompleteRegistration(client);
}

//USER
//	required for registration
//client : "USER <username> <mode> <unused> :<realname>"
//we don't use mode nor unused
//lets client set its username, and optionnaly its IRL name
void Server::handleUser(Client &client, const Command &command)
{
	if (client.isRegistered())
	{
		sendNumeric(
			client,
			"462",
			":You may not reregister"
		);
		return;
	}

	if (command.getParameterCount() < 4)
	{
		sendNumeric(
			client,
			"461",
			"USER :Not enough parameters"
		);
		return;
	}

	const std::vector<std::string> &parameters = command.getParameters();
	client.setUser(parameters[0], parameters[3]);
	tryCompleteRegistration(client);
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}

void Server::tryCompleteRegistration(Client &client)
{
	if (client.isRegistered())
		return;

	if (!client.isRegistrationReady())
		return;

	client.setRegistered(true);
	SendWelcome(client);

	//needs to be deleted or added to a debug class
	std::cout << "\t User " 
		<< client.hasNickname() 
		<< "(" + client.getUsername() + ")"
		<< " fd : " << client.getFd()
		<<  "has successfully registered\n";
}

//PRIVMSG
//	(for now no channels)
//client : "PRIVMSG <target> <text to be sent>"
//server : ":user!userh@localhost PRIVMSG <target> <text to be sent>"
//lets user send some message, target can be both a user or a channel !!
void Server::handlePrivmsg(Client &client, const Command &command)
{
	if (!client.isRegistered())
	{
		sendNumeric(
			client,
			"451",
			":You have not registered"
		);
		return;
	}

	const std::vector<std::string> &parameters =
		command.getParameters();

	if (parameters.empty())
	{
		sendNumeric(
			client,
			"411",
			":No recipient given (PRIVMSG)"
		);
		return;
	}

	if (parameters.size() < 2)
	{
		sendNumeric(
			client,
			"412",
			":No text to send"
		);
		return;
	}

	Client *target =
		findClientByNickname(parameters[0]);

	if (target == NULL)
	{
		sendNumeric(
			client,
			"401",
			parameters[0] + " :No such nick/channel"
		);
		return;
	}

	std::string message =
		":"
		+ client.getPrefix()
		+ " PRIVMSG "
		+ target->getNickname()
		+ " :"
		+ parameters[1];

	queueLine(*target, message);
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}