#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"

//QUIT
//lets clients quit the server, ending the connection
//quit message is done by Server at cleanup
void Server::handleQuit(Client &client, const Command &command)
{
	std::string reason = "Client Quit";

	if (command.getParameterCount() >= 1)
		reason = command.getParameters()[0];

	client.requestDisconnect(reason);
}

