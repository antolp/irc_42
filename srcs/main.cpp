#include "Server.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

//checking valid TCP port num using stream overloads >> (if read succeed)
//no char, lower than 65535
static bool checkPort(const std::string &text, unsigned short &port)
{
	std::istringstream	stream(text);
	unsigned long		value;
	char				extraChar;

	if (!(stream >> value))
		return false;

	if (stream >> extraChar)
		return false;

	if (value == 0 || value > 65535)
		return false;
	
	port = static_cast<unsigned short>(value);
	return true;
}


int main(int argc, char **argv)
{
	unsigned short port;

	if (argc != 3 || !checkPort(argv[1], port) || argv[2][0] == '\0')
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return EXIT_FAILURE;
	}
	//try to initialize and start the server
	try
	{
		Server server(port, argv[2]);
		server.run();
	}
	catch (const std::exception &error)
	{
		std::cerr << "ircserv: " << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "ircserv: unexpected error" << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
