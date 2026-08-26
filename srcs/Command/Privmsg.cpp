#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

//PRIVMSG
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

	const std::string &targetName = parameters[0];

	//Si la target est un channel (# ou &)
	if (targetName[0] == '#' || targetName[0] == '&')
	{
		Channel *channel = findChannel(targetName);

		if (channel == NULL)
		{
			sendNumeric(
				client,
				"401",
				targetName + " :No such nick/channel"
			);
			return;
		}

		if (!channel->hasMember(client.getFd()))
		{
			sendNumeric(
				client,
				"404",
				targetName + " :Cannot send to channel"
			);
			return;
		}

		std::string message =
			":"
			+ client.getPrefix()
			+ " PRIVMSG "
			+ targetName
			+ " :"
			+ parameters[1];

		broadcastToChannel(*channel, message, client.getFd());
		std::cout << command.getName() << " successful" << std::endl;
		return;
	}

	//Si la target est un user
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
	std::cout << command.getName() << " successful" << std::endl;
}

