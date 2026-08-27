#ifndef COMMAND_HPP
# define COMMAND_HPP

# include <cstddef>
# include <string>
# include <vector>

class Command
{
public:
	explicit Command(const std::string &line);

	bool isValid() const;

	const std::string &getName() const;
	const std::vector<std::string> &getParameters() const;
	std::size_t getParameterCount() const;

private:
	void parse(const std::string &line);
	void normalizeName();

	bool						_valid;
	std::string					_name;
	std::vector<std::string>	_parameters;
};

#endif