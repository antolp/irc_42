#include <cstddef>
#include <string>
#include <set>
#include <map>

//channel contains and tracks members
//Members points to a client and stores info on client
// in context of a specific channel
class Channel
{
public:
	class Member
	{
	public:
		bool isOperator() const;
		void setOperator(bool enabled);

	private:
		bool _isOperator;
		Client	&client;
	};

	typedef std::map<int, Member> MemberMap;

	explicit Channel(const std::string &name);

	const std::string &getName() const;

	bool addMember(int fd, bool makeOperator);
	void removeMember(int fd);

	bool hasMember(int fd) const;
	bool IsMemberOperator(int fd) const;
	bool setMemberOperator(int fd, bool enabled);

	bool empty() const;
	std::size_t getMemberCount() const;

	const MemberMap &getMembers() const;

private:
	std::string _name;
	MemberMap   _members;
};