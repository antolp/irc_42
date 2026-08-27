#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Utils.hpp"

#include <iostream>
#include <vector>
#include <set>

//NICK
//	required for registration
//client : "NICK <nickname>"
//lets client provide a nickname the server should use
//that nickname can be reset by client
//server sends :
//	oldNick!user@host NICK :newNick to clients sharing a channel
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

	const std::string &nickname = command.getParameters()[0];

	if (!isValidNickname(nickname))
	{
		sendNumeric(
			client,
			"432",
			nickname + " :Erroneous nickname"
		);
		return;
	}

	const std::string key = ircCaseFold(nickname);

	std::map<std::string, int>::iterator existing =
		_nicknameIndex.find(key);

	if (existing != _nicknameIndex.end()
		&& existing->second != client.getFd())
	{
		sendNumeric(
			client,
			"433",
			nickname + " :Nickname is already in use"
		);
		return;
	}

	//exact same nickname do nothing
	//case-only change such as Alice -> ALICE still allowed
	if (client.hasNickname()
		&& client.getNickname() == nickname)
	{
		return;
	}

	const bool wasRegistered = client.isRegistered();

	std::string oldNickname;
	std::string oldPrefix;
	std::set<int> recipients;

	if (client.hasNickname())
	{
		oldNickname = client.getNickname();

		if (wasRegistered)
		{
			oldPrefix = client.getPrefix();

			// The client itself must receive the NICK change.
			recipients.insert(client.getFd());

			//each client sharing a channel with user must receive de message ONCE
			for (std::map<std::string, Channel *>::iterator it =
					_channels.begin();
				 it != _channels.end();
				 ++it)
			{
				Channel *channel = it->second;

				if (!channel->hasMember(client.getFd()))
					continue;

				const Channel::MemberMap &members =
					channel->getMembers();

				for (Channel::MemberMap::const_iterator member =
						members.begin();
					 member != members.end();
					 ++member)
				{
					recipients.insert(member->first);
				}
			}
		}
		
		//update invites
		for (std::map<std::string, Channel *>::iterator it =
				_channels.begin();
			 it != _channels.end();
			 ++it)
		{
			Channel *channel = it->second;

			if (channel->isInvited(oldNickname))
			{
				channel->removeInvite(oldNickname);
				channel->addInvite(nickname);
			}
		}

		_nicknameIndex.erase(
			ircCaseFold(oldNickname)
		);
	}

	client.setNickname(nickname);
	_nicknameIndex[key] = client.getFd();

	//the first NICK used during registration is not broadcast.
	if (wasRegistered)
	{
		const std::string nickMessage =
			":" + oldPrefix
			+ " NICK :"
			+ nickname;

		for (std::set<int>::iterator it = recipients.begin();
			 it != recipients.end();
			 ++it)
		{
			Client *target = findClient(*it);

			if (target != NULL)
				queueLine(*target, nickMessage);
		}
	}

	tryCompleteRegistration(client);
}