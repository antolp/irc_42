#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

//JOIN
//lets client create or join a channel
//client : "JOIN <channel>"
//server : broadcast ":nick!user@localhost JOIN :<channel>"
//followed by topic (331) and names (353/366) replies
void Server::handleJoin(Client &client, const Command &command)
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

	if (command.getParameterCount() < 1)
	{
		sendNumeric(
			client,
			"461",
			"JOIN :Not enough parameters"
		);
		return;
	}

	const std::string &channelName = command.getParameters()[0];

	if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
	{
		sendNumeric(
			client,
			"403",
			channelName + " :No such channel"
		);
		return;
	}

	Channel *channel = findChannel(channelName);

	if (channel == NULL)
	{
		channel = new Channel(channelName);
		_channels[channelName] = channel;
		channel->addMember(client.getFd(), true);
	}
	else
	{
		if (channel->hasMember(client.getFd()))
			return;

		channel->addMember(client.getFd(), false);
	}

	std::string joinMessage = ":" + client.getPrefix() + " JOIN :" + channelName;
	broadcastToChannel(*channel, joinMessage);

	if (channel->hasTopic())
		sendNumeric(client, "332", channelName + " :" + channel->getTopic());
	else
		sendNumeric(client, "331", channelName + " :No topic is set");
	sendChannelNames(client, *channel);
	std::cout << command.getName() << " successful" << std::endl;
}

