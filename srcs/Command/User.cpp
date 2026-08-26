#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

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

