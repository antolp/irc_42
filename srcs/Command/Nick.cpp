#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"
#include "Utils.hpp"

#include <iostream>
#include <vector>

//NICK
//	required for registration
//client : "NICK <nickname>"
//lets client provide a nickname the server should use
//that nickname can be reset by client
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

	const std::string &nickname =command.getParameters()[0];
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
	std::map<std::string, int>::iterator existing = _nicknameIndex.find(key);
	
	if (existing != _nicknameIndex.end() && existing->second != client.getFd())
	{
		sendNumeric(
			client,
			"433",
			nickname + " :Nickname is already in use"
		);
		return;
	}
	if (client.hasNickname())
	{
		const std::string oldNickname = client.getNickname();

		//invites stored by nickname, rename should not loose them
		for (std::map<std::string, Channel *>::iterator it = _channels.begin();
			 it != _channels.end(); ++it)
		{
			Channel *channel = it->second;
			if (channel->isInvited(oldNickname))
			{
				channel->removeInvite(oldNickname);
				channel->addInvite(nickname);
			}
		}

		_nicknameIndex.erase(ircCaseFold(oldNickname));
	}

	client.setNickname(nickname);
	_nicknameIndex[key] = client.getFd();

	tryCompleteRegistration(client);
}

