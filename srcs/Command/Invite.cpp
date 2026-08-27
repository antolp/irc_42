#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

//INVITE
//lets a user invite another user to a channel
//client : "INVITE <nickname> <channel>"
//server : to sender -> 341 RPL_INVITING <nickname> <channel>
//         to target -> ":nick!user@localhost INVITE <nickname> :<channel>"
void Server::handleInvite(Client &client, const Command &command)
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

	if (command.getParameterCount() < 2)
	{
		sendNumeric(
			client,
			"461",
			"INVITE :Not enough parameters"
		);
		return;
	}

	const std::string &targetNick = command.getParameters()[0];
	const std::string &channelName = command.getParameters()[1];

	Client *target = findClientByNickname(targetNick);

	if (target == NULL)
	{
		sendNumeric(
			client,
			"401",
			targetNick + " :No such nick/channel"
		);
		return;
	}

	Channel *channel = findChannel(channelName);

	if (channel == NULL)
	{
		sendNumeric(
			client,
			"403",
			channelName + " :No such channel"
		);
		return;
	}

	if (!channel->hasMember(client.getFd()))
	{
		sendNumeric(
			client,
			"442",
			channelName + " :You're not on that channel"
		);
		return;
	}

	//a debattre
	if (channel->isInviteOnly() && !channel->IsMemberOperator(client.getFd()))
	{
		sendNumeric(
			client,
			"482",
			channelName + " :You're not channel operator"
		);
		return;
	}

	if (channel->hasMember(target->getFd()))
	{
		sendNumeric(
			client,
			"443",
			targetNick + " " + channelName + " :is already on channel"
		);
		return;
	}

	channel->addInvite(targetNick);

	sendNumeric(
		client,
		"341",
		targetNick + " " + channelName
	);

	std::string inviteMessage =
		":"
		+ client.getPrefix()
		+ " INVITE "
		+ targetNick
		+ " :"
		+ channelName;

	queueLine(*target, inviteMessage);
	std::cout << command.getName() << " successful" << std::endl;
}