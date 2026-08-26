#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Command.hpp"

#include <iostream>
#include <vector>

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

