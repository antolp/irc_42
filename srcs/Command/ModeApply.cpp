#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include <limits>
#include <string>

namespace
{
	//sstream
	bool parsePositiveUserLimit(const std::string& text, std::size_t& limit)
	{
		if (text.empty() || text[0] < '0' || text[0] > '9')
			return false;
		std::istringstream stream(text);

		std::size_t value;
		char extra;

		if (!(stream >> value))
			return false;

		if (stream >> extra)
			return false;

		if (value == 0)
			return false;

		limit = value;
		return true;
	}
}

//dispatcher
void Server::applyChannelModeChange(Client &client, Channel &channel, const ModeChange &change)
{
	switch (change.mode)
	{
		case 'i':
			applyInviteOnlyMode(client, channel, change);
			break;
		case 't':
			applyTopicRestrictedMode(client, channel, change);
			break;
		case 'k':
			applyKeyMode(client, channel, change);
			break;
		case 'o':
			applyOperatorMode(client, channel, change);
			break;
		case 'l':
			applyUserLimitMode(client, channel, change);
			break;
		default:
			sendNumeric(
				client,
				"472",
				std::string(1, change.mode)
				+ " :is unknown mode char to me"
			);
			break;
	}
}

//+/-i
//no arg
void Server::applyInviteOnlyMode(Client &client, Channel &channel, const ModeChange &change)
{
	if (channel.isInviteOnly() == change.adding)
		return;

	channel.setInviteOnly(change.adding);
	broadcastModeChange(client, channel, change);
}

//+/-t
//no arg
void Server::applyTopicRestrictedMode(Client &client, Channel &channel, const ModeChange &change)
{
	if (channel.isTopicRestricted() == change.adding)
		return;

	channel.setTopicRestricted(change.adding);
	broadcastModeChange(client, channel, change);
}

//+/-k
//arg is key
void Server::applyKeyMode(Client &client, Channel &channel, const ModeChange &change)
{
	if (change.adding)
	{
		if (change.argument.empty())
		{
			sendNumeric(
				client,
				"696",
				channel.getName()
				+ " k :Invalid mode parameter"
			);
			return;
		}

		if (channel.hasKey()
			&& channel.getKey() == change.argument)
		{
			return;
		}

		channel.setKey(change.argument);
	}
	else
	{
		if (!channel.hasKey())
			return;

		channel.removeKey();
	}

	broadcastModeChange(client, channel, change);
}

//+/-o
//arg is nick
void Server::applyOperatorMode(Client &client, Channel &channel, const ModeChange &change)
{
	Client *target = findClientByNickname(change.argument);

	if (target == NULL)
	{
		sendNumeric(
			client,
			"401",
			change.argument + " :No such nick/channel"
		);
		return;
	}

	if (!channel.hasMember(target->getFd()))
	{
		sendNumeric(
			client,
			"441",
			change.argument + " " + channel.getName()
			+ " :They aren't on that channel"
		);
		return;
	}

	if (channel.IsMemberOperator(target->getFd()) == change.adding)
		return;

	channel.setMemberOperator(target->getFd(), change.adding);
	broadcastModeChange(client, channel, change);
}

//+/-l
//here + and - work differently
//+ : arg is uint limit
//- : no arg removes limit
void Server::applyUserLimitMode(Client &client, Channel &channel, const ModeChange &change)
{
	if (change.adding)
	{
		std::size_t limit;

		if (!parsePositiveUserLimit(change.argument, limit))
		{
			sendNumeric(
				client,
				"696",
				channel.getName()
				+ " l "
				+ change.argument
				+ " :Invalid mode parameter"
			);
			return;
		}

		if (channel.hasUserLimit()
			&& channel.getUserLimit() == limit)
		{
			return;
		}

		channel.setUserLimit(limit);
	}
	else
	{
		if (!channel.hasUserLimit())
			return;

		channel.removeUserLimit();
	}

	broadcastModeChange(client, channel, change);
}

//nick!user@localhost MODE <channel> <modes> [arguments]
void Server::broadcastModeChange(Client &client, Channel &channel, const ModeChange &change)
{
	std::string message =
		":"
		+ client.getPrefix()
		+ " MODE "
		+ channel.getName()
		+ " "
		+ (change.adding ? "+" : "-")
		+ std::string(1, change.mode);

	if (change.hasArgument)
		message += " " + change.argument;

	broadcastToChannel(channel, message);
}
