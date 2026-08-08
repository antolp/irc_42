#include "Client.hpp"

#include <unistd.h>

bool Client::hasNickname() const
{
	return !_nickname.empty();
}

bool Client::isRegistered() const
{
	return _registered;
}

const std::string &Client::getNickname() const
{
	return _nickname;
}

const std::string &Client::getUsername() const
{
	return _username;
}

const std::string &Client::getRealname() const
{
	return _realname;
}

void	Client::setPasswordAccepted()
{
	_passwordAccepted = true;
}

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUser(const std::string &username, const std::string &realname)
{
	_username = username;
	_realname = realname;
	_hasUser = true;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
}

//clients needs to succeed PASS, USER and NICK commands to be registered to the server
bool Client::isRegistrationReady() const
{
	return _passwordAccepted && !_nickname.empty() && _hasUser;
}

//for now no peer address, hexchat and irssi doesn't seem to check it
std::string Client::getPrefix() const
{
	return _nickname + "!" + _username + "@localhost";
}

