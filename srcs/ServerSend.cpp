#include "Server.hpp"

//queues received bytes for every connected client except the sender
void Server::queueBroadcast(
    int senderFd,
    const char* data,
    std::size_t length)
{
    for (std::size_t i = 0; i < _pollFds.size(); ++i)
    {
        const int targetFd = _pollFds[i].fd;

        if (targetFd == _listenerFd || targetFd == senderFd)
            continue;

        _outputBuffers[targetFd].append(data, length);

        //ask the next poll() call to report when this socket can be written
		//hangs without this
        _pollFds[i].events |= POLLOUT;
    }
}

//sends part of a client's queued output after poll() reports POLLOUT
//false when client should be disconnected
bool Server::flushClientOutput(std::size_t index)
{
    const int fd = _pollFds[index].fd;

    std::map<int, std::string>::iterator output =
        _outputBuffers.find(fd);

    if (output == _outputBuffers.end() || output->second.empty())
    {
        _pollFds[index].events &= ~POLLOUT;
        return true;
    }

    const ssize_t sent = send(
        fd,
        output->second.data(),
        output->second.size(),
        0
    );

    if (sent <= 0)
    {
        std::cerr
            << "send() failed for fd "
            << fd
            << std::endl;

        return false;
    }

    output->second.erase(
        0,
        static_cast<std::size_t>(sent)
    );

    //stop watching for writable events when nothing remains to send
	//again hangs without this, sometimes
    if (output->second.empty())
        _pollFds[index].events &= ~POLLOUT;

    return true;
}
