#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

//TOPIC
//lets client view or change channel topic
//client : "TOPIC <channel> [topic]"
//server : view -> 332 RPL_TOPIC or 331 RPL_NOTOPIC
//         change -> broadcast ":nick!user@localhost TOPIC <channel> :[topic]"
void Server::handleTopic(Client &client, const Command &command)
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
			"TOPIC :Not enough parameters"
		);
		return;
	}

	const std::string &channelName = command.getParameters()[0];
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

	if (command.getParameterCount() == 1)
	{
		if (channel->hasTopic())
		{
			sendNumeric(
				client,
				"332",
				channelName + " :" + channel->getTopic()
			);
		}
		else
		{
			sendNumeric(
				client,
				"331",
				channelName + " :No topic is set"
			);
		}
		return;
	}

	const std::string &newTopic = command.getParameters()[1];
	channel->setTopic(newTopic);

	std::string topicMessage = ":" + client.getPrefix() + " TOPIC " + channelName + " :" + newTopic;
	broadcastToChannel(*channel, topicMessage);
	std::cout << command.getName() << " successful" << std::endl;
}

