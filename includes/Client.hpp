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

private:
	Client(const Client &other);
	Client &operator=(const Client &other);

	int         _fd;
	std::string _outputBuffer;
	std::string _inputBuffer;
    bool		_disconnectRequested;
};

#endif