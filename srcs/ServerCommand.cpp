#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

namespace
{
	const char *SERVER_NAME = "ircserv";
}

void Server::dispatchCommand(Client &client, const Command &command)
{

	// //needs to be deleted or added to a debug class
	// std::cout << "\t dispatching command :\n\t\t" << command.getName();
	// const std::vector<std::string> &parameters = command.getParameters();
	// for (std::size_t i = 0; i < parameters.size(); ++i)
	// 	std::cout << parameters[i] << ",\t";
	// std::cout << std::endl;
	// //

	const std::string &name = command.getName();

	if (name == "PING")
		handlePing(client, command);
	else if (name == "QUIT")
		handleQuit(client, command);
	else if (name == "CAP")
		handleCap(client, command);
	else if (name == "PASS")
		handlePass(client, command);
	else if (name == "USER")
		handleUser(client, command);
	else if (name == "NICK")
		handleNick(client, command);
	else if (name == "PRIVMSG")
		handlePrivmsg(client, command);
	else if (name == "JOIN")
		handleJoin(client, command);
	else if (name == "TOPIC")
		handleTopic(client, command);
	else if (name == "KICK")
		handleKick(client, command);
	else if (name == "INVITE")
		handleInvite(client, command);
	else
		handleUnknownCommand(client, command);
}

void Server::handleUnknownCommand(
	Client &client,
	const Command &command)
{
	std::cout
		<< "Unknown command from fd "
		<< client.getFd()
		<< ": "
		<< command.getName()
		<< std::endl;
}

//PING
//lets clients and server to verify that the connection is still active
//client : "PING :token"
//server : ":ircserv PONG ircserv :token"
void Server::handlePing(
	Client &client,
	const Command &command)
{
	if (command.getParameterCount() == 0)
	{
		queueLine(
			client,
			":ircserv 409 * :No origin specified"
		);
		return;
	}

	const std::vector<std::string> &parameters = command.getParameters();

	queueLine(
		client,
		std::string(":")
			+ SERVER_NAME
			+ " PONG "
			+ SERVER_NAME
			+ " :"
			+ parameters[0]
	);
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}

//QUIT
//lets clients quit the server, ending the connection
void Server::handleQuit(Client &client, const Command &command)
{
	std::string reason = "Client Quit";

	if (command.getParameterCount() >= 1)
		reason = command.getParameters()[0];

	client.requestDisconnect(reason);
}

//CAP
//clients send "CAP LS 302" at connection and require a normal answers
void Server::handleCap(Client &client, const Command &command)
{
	const std::vector<std::string> &parameters = command.getParameters();

	if (parameters.empty())
		return;

	if (parameters[0] == "LS")
	{
		queueLine(
			client,
			":ircserv CAP * LS"
		);
		return;
	}

	if (parameters[0] == "REQ")
	{
		std::string requested;

		if (parameters.size() >= 2)
			requested = parameters[1];

		queueLine(
			client,
			":ircserv CAP * NAK" + requested
		);
	}
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}

//PASS
//	required for registration
//client : "PASS <password>"
//lets client supply the correct password
//on success no answer
void Server::handlePass(Client &client, const Command &command)
{
	if (client.isRegistered())
	{
		sendNumeric(
			client,
			"462",
			":You may not reregister"
		);
		return;
	}

	if (command.getParameterCount() < 1)
	{
		sendNumeric(
			client,
			"461",
			"PASS :Not enough parameters"
		);
		return;
	}

	if (command.getParameters()[0] != _password)
	{
		sendNumeric(
			client,
			"464",
			":Password incorrect"
		);
		return;
	}

	client.setPasswordAccepted();
	tryCompleteRegistration(client);
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}

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

	const std::string key = nickname;
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
		_nicknameIndex.erase(client.getNickname());
	}

	client.setNickname(nickname);
	_nicknameIndex[key] = client.getFd();

	tryCompleteRegistration(client);
}

//USER
//	required for registration
//client : "USER <username> <mode> <unused> :<realname>"
//we don't use mode nor unused
//lets client set its username, and optionnaly its IRL name
void Server::handleUser(Client &client, const Command &command)
{
	if (client.isRegistered())
	{
		sendNumeric(
			client,
			"462",
			":You may not reregister"
		);
		return;
	}

	if (command.getParameterCount() < 4)
	{
		sendNumeric(
			client,
			"461",
			"USER :Not enough parameters"
		);
		return;
	}

	const std::vector<std::string> &parameters = command.getParameters();
	client.setUser(parameters[0], parameters[3]);
	tryCompleteRegistration(client);
	//needs to be deleted or added to a debug class
	std::cout << command.getName() << " successful" << std::endl;
}

void Server::tryCompleteRegistration(Client &client)
{
	if (client.isRegistered())
		return;

	if (!client.isRegistrationReady())
		return;

	client.setRegistered(true);
	SendWelcome(client);

	//needs to be deleted or added to a debug class
	std::cout << "\t User " 
		<< client.hasNickname() 
		<< "(" + client.getUsername() + ")"
		<< " fd : " << client.getFd()
		<<  "has successfully registered\n";
}

//PRIVMSG
//	(for now no channels)
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

	if (channel->empty())
	{
		_channels.erase(channelName);
		delete channel;
	}
	std::cout << command.getName() << " successful" << std::endl;
}

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