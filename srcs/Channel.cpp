#include "Channel.hpp"

// Member
bool Channel::Member::isOperator() const
{
	return _isOperator;
}

void Channel::Member::setOperator(bool enabled)
{
	_isOperator = enabled;
}

// Channel
Channel::Channel(const std::string &name)
	: _name(name),
	  _topic(""),
	  _inviteOnly(false),
	  _topicRestricted(false),
	  _hasKey(false),
	  _key(""),
	  _hasUserLimit(false),
	  _userLimit(0)
{
}

const std::string &Channel::getName() const
{
	return _name;
}

const std::string &Channel::getTopic() const
{
	return _topic;
}

void Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

bool Channel::hasTopic() const
{
	return !_topic.empty();
}

bool Channel::addMember(int fd, bool makeOperator)
{
	if (hasMember(fd))
		return false;

	_members.insert(std::make_pair(fd, Member(NULL, makeOperator)));
	return true;
}

void Channel::removeMember(int fd)
{
	_members.erase(fd);
}

bool Channel::hasMember(int fd) const
{
	return _members.find(fd) != _members.end();
}

bool Channel::IsMemberOperator(int fd) const
{
	MemberMap::const_iterator it = _members.find(fd);
	if (it == _members.end())
		return false;

	return it->second.isOperator();
}

bool Channel::setMemberOperator(int fd, bool enabled)
{
	MemberMap::iterator it = _members.find(fd);
	if (it == _members.end())
		return false;

	it->second.setOperator(enabled);
	return true;
}

bool Channel::empty() const
{
	return _members.empty();
}

std::size_t Channel::getMemberCount() const
{
	return _members.size();
}

const Channel::MemberMap &Channel::getMembers() const
{
	return _members;
}

void Channel::addInvite(const std::string &nickname)
{
	_invitedUsers.insert(nickname);
}

void Channel::removeInvite(const std::string &nickname)
{
	_invitedUsers.erase(nickname);
}

bool Channel::isInvited(const std::string &nickname) const
{
	return _invitedUsers.find(nickname) != _invitedUsers.end();
}
