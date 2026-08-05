#ifndef CLIENT_HPP
# define CLIENT_HPP

//for now clients don't have their own buffer
class Client
{
public:
	explicit Client(int fd);
	~Client();

	int getFd() const;

private:
	Client(const Client &other);
	Client &operator=(const Client &other);

	int _fd;
};

#endif