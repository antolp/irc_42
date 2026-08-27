#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

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
	else if (name == "JOIN")
		handleJoin(client, command);
	else if (name == "TOPIC")
		handleTopic(client, command);
	else if (name == "KICK")
		handleKick(client, command);
	else if (name == "INVITE")
		handleInvite(client, command);
	else if (name == "MODE")
		handleMode(client, command);
	else
		handleUnknownCommand(client, command);
}

void Server::handleUnknownCommand(Client &client, const Command &command)
{
	std::cout
		<< "Unknown command from fd "
		<< client.getFd()
		<< ": "
		<< command.getName()
		<< std::endl;
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
		<< client.getNickname() 
		<< "(" + client.getUsername() + ")"
		<< " fd : " << client.getFd()
		<<  "has successfully registered\n";
}
