#include "Server.hpp"
#include "Client.hpp"

Client *Server::findClientByNickname(const std::string &nickname)
{
	for (std::map<int, Client *>::iterator it = _clients.begin(); 
		it != _clients.end(); it++)
	{
		if (it->second->getNickname() == nickname)
			return it->second;
	}
	return NULL;
}

//not complete
bool Server::isValidNickname(const std::string &nickname) const
{
	if (nickname.empty())
		return false;

	if (nickname[0] == '#' || nickname[0] == '&' || nickname[0] == ':')
		return false;

	for (std::size_t i = 0; i < nickname.size(); ++i)
	{
		if (nickname[i] == ' '
			|| nickname[i] == ','
			|| nickname[i] == '\r'
			|| nickname[i] == '\n'
			|| nickname[i] == '\t')
		{
			return false;
		}
	}

	return true;
}

//missing some stuff
//No multi-lin sendNumeric() calls here because that would just take too much space
void	Server::SendWelcome(Client &client)
{
	sendNumeric(client, "001", ":Welcome to this really awesome IRC server !" + client.getPrefix());
	sendNumeric(client, "002", ":This server has been running for (idk)");
	sendNumeric(client, "003", ":here's a nice snail in ASCII art :");
	sendNumeric(client, "004", ":   ───▄▄▄");
	sendNumeric(client, "005", ":─▄▀░▄░▀▄");
	sendNumeric(client, "006", ":─█░█▄▀░█");
	sendNumeric(client, "007", ":─█░▀▄▄▀█▄█▄▀3");
	sendNumeric(client, "008", ":▄▄█▄▄▄▄███▀");
	sendNumeric(client, "376", ":End of MOTD command");

}
