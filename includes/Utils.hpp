#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>

std::string ircCaseFold(const std::string &value);
bool ircEquals(
	const std::string &left,
	const std::string &right);
#endif
