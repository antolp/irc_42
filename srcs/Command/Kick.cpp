#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

//KICK
//lets channel operator eject a user from a channel
//client : "KICK <channel> <nickname> [comment]"
//server : broadcast ":nick!user@localhost KICK <channel> <nickname> :[comment]"
void Server::handleKick(Client &client, const Command &command)
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
			"KICK :Not enough parameters"
		);
		return;
	}

	const std::string &channelName = command.getParameters()[0];
	const std::string &targetNick = command.getParameters()[1];

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

	if (!channel->IsMemberOperator(client.getFd()))
	{
		sendNumeric(
			client,
			"482",
			channelName + " :You're not channel operator"
		);
		return;
	}

	Client *target = findClientByNickname(targetNick);

	if (target == NULL || !channel->hasMember(target->getFd()))
	{
		sendNumeric(
			client,
			"441",
			targetNick + " " + channelName + " :They aren't on that channel"
		);
		return;
	}

	std::string reason = client.getNickname();
	if (command.getParameterCount() >= 3)
		reason = command.getParameters()[2];

	std::string kickMessage =
		":"
		+ client.getPrefix()
		+ " KICK "
		+ channelName
		+ " "
		+ targetNick
		+ " :"
		+ reason;

	broadcastToChannel(*channel, kickMessage);
	removeMemberFromChannel(channel, target->getFd());
	std::cout << command.getName() << " successful" << std::endl;
}

