#include <cstddef>
#include <string>
#include <set>
#include <map>

//channel contains and tracks members
//Members points to a client and stores info on client
// in context of a specific channel

class Client;

class Channel
{
public:
	class Member
	{
	public:
		Member(Client *client = NULL, bool isOperator = false) 
		{
			_isOperator = isOperator;
			_client = client;
		}

		bool isOperator() const;
		void setOperator(bool enabled);

	private:
		bool _isOperator;
		Client *_client;
	};

	typedef std::map<int, Member> MemberMap;

	explicit Channel(const std::string &name);

	const std::string &getName() const;

	const std::string &getTopic() const;
	void setTopic(const std::string &topic);
	bool hasTopic() const;

	bool addMember(int fd, bool makeOperator);
	void removeMember(int fd);

	bool hasMember(int fd) const;
	bool IsMemberOperator(int fd) const;
	bool setMemberOperator(int fd, bool enabled);

	bool empty() const;
	std::size_t getMemberCount() const;

	const MemberMap &getMembers() const;

	void addInvite(const std::string &nickname);
	void removeInvite(const std::string &nickname);
	bool isInvited(const std::string &nickname) const;

private:
	std::string _name;
	std::string _topic;
	MemberMap   _members;
	std::set<std::string> _invitedUsers;
};
