#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

namespace
{
	struct ModeChange
	{
		ModeChange(bool addingMode, char modeCharacter)
			: adding(addingMode),
			  mode(modeCharacter),
			  hasArgument(false)
		{
		}

		bool        adding;
		char        mode;
		bool        hasArgument;
		std::string argument;
	};

	bool isChannelTarget(const std::string &target)
	{
		if (target.empty())
			return false;

		return target[0] == '#' || target[0] == '&';
	}

	bool isSupportedChannelMode(char mode)
	{
		return (
			mode == 'i'
			|| mode == 't'
			|| mode == 'k'
			|| mode == 'o'
			|| mode == 'l'
		);
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
	
	bool parseChannelModeChanges(
		const std::vector<std::string> &parameters,
		std::vector<ModeChange> &changes,
		std::size_t &argumentIndex)
	{
		const std::string &modeString = parameters[1];

		if (modeString.empty())
		{
			std::cout
				<< "MODE rejected: empty mode string"
				<< std::endl;
			return false;
		}

		bool adding = true;
		bool signSeen = false;

		argumentIndex = 2;

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
				std::cout
					<< "MODE rejected: mode '"
					<< character
					<< "' has no preceding + or -"
					<< std::endl;
				return false;
			}

			ModeChange change(adding, character);

			if (modeNeedsArgument(character, adding))
			{
				if (argumentIndex >= parameters.size())
				{
					std::cout
						<< "MODE rejected: mode "
						<< (adding ? '+' : '-')
						<< character
						<< " requires an argument"
						<< std::endl;
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
			std::cout
				<< "MODE rejected: no modes in mode string"
				<< std::endl;
			return false;
		}

		return true;
	}
}





//
//WILL HAVE TO GO
//
namespace
{
	void debugChannelModeQuery(const std::string &target)
	{
		std::cout
			<< "CHANNEL MODE QUERY:"
			<< std::endl
			<< "\tchannel: "
			<< target
			<< std::endl;
	}

	void debugModeChanges(
		const std::vector<ModeChange> &changes)
	{
		for (std::size_t i = 0; i < changes.size(); ++i)
		{
			const ModeChange &change = changes[i];

			std::cout
				<< "\tchange "
				<< i
				<< ": "
				<< (change.adding ? '+' : '-')
				<< change.mode;

			if (!isSupportedChannelMode(change.mode))
				std::cout << " [unsupported]";

			if (change.hasArgument)
			{
				std::cout
					<< " argument=\""
					<< change.argument
					<< "\"";
			}

			std::cout << std::endl;
		}
	}

	void debugUnusedModeParameters(
		const std::vector<std::string> &parameters,
		std::size_t argumentIndex)
	{
		if (argumentIndex >= parameters.size())
			return;

		std::cout
			<< "\tunused parameters:"
			<< std::endl;

		for (std::size_t i = argumentIndex;
			 i < parameters.size();
			 ++i)
		{
			std::cout
				<< "\t\t"
				<< parameters[i]
				<< std::endl;
		}
	}

	void debugChannelModeParsing(
		const std::vector<std::string> &parameters,
		const std::vector<ModeChange> &changes,
		std::size_t argumentIndex)
	{
		std::cout
			<< "CHANNEL MODE parsed:"
			<< std::endl
			<< "\tchannel: "
			<< parameters[0]
			<< std::endl
			<< "\traw mode string: "
			<< parameters[1]
			<< std::endl;

		debugModeChanges(changes);
		debugUnusedModeParameters(parameters, argumentIndex);
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
		std::cout
			<< "MODE rejected: client not registered"
			<< std::endl;
		return;
	}

	if (parameters.empty())
	{
		std::cout
			<< "MODE rejected: missing target"
			<< std::endl;
		return;
	}

	const std::string &target = parameters[0];

	if (!isChannelTarget(target))
		return;

	//MODE #channel
	if (parameters.size() == 1)
	{
		debugChannelModeQuery(target);
		return;
	}

	std::vector<ModeChange> changes;
	std::size_t argumentIndex;

	if (!parseChannelModeChanges(
			parameters,
			changes,
			argumentIndex))
	{
		return;
	}

	debugChannelModeParsing(
		parameters,
		changes,
		argumentIndex);
}