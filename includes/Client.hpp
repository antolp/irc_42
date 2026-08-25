#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <cstddef>
#include <string>

class Client
{
public:
	explicit Client(int fd);
	~Client();

	int getFd() const;

	//output
	void		appendOutput(const char *data, std::size_t length);
	bool		hasOutput() const;
	const char	*getOutputData() const;
	std::size_t	getOutputSize() const;
	void		consumeOutput(std::size_t length);

	//input
	void		appendInput(const char *data, std::size_t length);
	std::size_t	getInputSize() const;
	bool		popLine(std::string &line);

	//state
	void		requestDisconnect();
	bool		isDisconnectRequested() const;

	//registration
	bool hasNickname() const;
	bool isRegistered() const;
	void setPasswordAccepted();
	void setNickname(const std::string &nickname);
	void setUser(const std::string &username, const std::string &realname);
	void setRegistered(bool registered);
	bool isRegistrationReady() const;

	const std::string &getNickname() const;
	const std::string &getUsername() const;
	const std::string &getRealname() const;
	std::string getPrefix() const;

private:
	Client(const Client &other);
	Client &operator=(const Client &other);

	std::string	_nickname;
	std::string	_username;
	std::string	_realname;

	bool		_passwordAccepted;
	bool		_hasUser;
	bool		_registered;

	int			_fd;
	std::string	_outputBuffer;
	std::string	_inputBuffer;
	bool		_disconnectRequested;
};

#endif