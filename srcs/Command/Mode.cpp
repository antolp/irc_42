#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

namespace
{
	bool isChannelTarget(const std::string &target)
	{
		if (target.empty())
			return false;

		return target[0] == '#' || target[0] == '&';
	}

	bool modeNeedsArgument(char mode, bool adding)
	{
		if (mode == 'k')
			return true;

		if (mode == 'o')
			return true;

		if (mode == 'l' && adding)
			return true;

		return false;
	}

	bool parseChannelModeChanges(const std::vector<std::string> &parameters, 
			std::vector<ModeChange> &changes)
	{
		const std::string &modeString = parameters[1];

		if (modeString.empty())
		{
			// std::cout
			// 	<< "MODE rejected: empty mode string"
			// 	<< std::endl;
			return false;
		}

		bool adding = true;
		bool signSeen = false;
		std::size_t argumentIndex = 2;

		for (std::size_t i = 0; i < modeString.size(); ++i)
		{
			const char character = modeString[i];

			if (character == '+')
			{
				adding = true;
				signSeen = true;
				continue;
			}

			if (character == '-')
			{
				adding = false;
				signSeen = true;
				continue;
			}

			if (!signSeen)
			{
				// std::cout
				// 	<< "MODE rejected: mode '"
				// 	<< character
				// 	<< "' has no preceding + or -"
				// 	<< std::endl;
				return false;
			}

			ModeChange change(adding, character);

			if (modeNeedsArgument(character, adding))
			{
				if (argumentIndex >= parameters.size())
				{
					// std::cout
					// 	<< "MODE rejected: mode "
					// 	<< (adding ? '+' : '-')
					// 	<< character
					// 	<< " requires an argument"
					// 	<< std::endl;
					return false;
				}

				change.hasArgument = true;
				change.argument = parameters[argumentIndex];
				++argumentIndex;
			}

			changes.push_back(change);
		}

		if (changes.empty())
		{
			// std::cout << "MODE rejected: no modes in mode string" << std::endl;
			return false;
		}

		return true;
	}
}

//MODE
//lets a client query or modify a channel's modes
//client : "MODE <channel> [<modes> [arguments]]"
//modes  :	+/-i	toggle invite-only channel
//			+/-t	toggle only channel operators can change the topic
//			+/-k	set/remove channel key		(+k <key>, -k <key>)
//			+/-o 	give/remove operator status	(+o <nick>, -o <nick>)
//			+/-l	set/remove user limit		(+l <limit>, -l)
//server :	on query, replies with the channel's current modes
//			on change, validates permissions/mode arguments, applies valid changes,
//	then broadcasts ":nick!user@localhost MODE <channel> <modes> [arguments...]"
void Server::handleMode(Client &client, const Command &command)
{
	const std::vector<std::string> &parameters =
		command.getParameters();

	if (!client.isRegistered())
	{
		sendNumeric(
			client,
			"451",
			":You have not registered"
		);
		return;
	}

	if (parameters.empty())
	{
		sendNumeric(
			client,
			"461",
			"MODE :Not enough parameters"
		);
		return;
	}

	const std::string &target = parameters[0];

	//no user mode subject doesnt ask for it
	if (!isChannelTarget(target))
		return;

	Channel *channel = findChannel(target);
	if (channel == NULL)
	{
		sendNumeric(
			client,
			"403",
			target + " :No such channel"
		);
		return;
	}

	//no query subject doesnt ask for it
	if (parameters.size() == 1)
		return;

	//a debattre de nouveau
	if (!channel->IsMemberOperator(client.getFd()))
	{
		sendNumeric(
			client,
			"482",
			target + " :You're not channel operator"
		);
		return;
	}

	std::vector<ModeChange> changes;
	if (!parseChannelModeChanges(parameters, changes))
		return;

	for (std::size_t i = 0; i < changes.size(); ++i)
		applyChannelModeChange(client, *channel, changes[i]);
	if (_debug)
		std::cout << command.getName() << " successful" << std::endl;
}
