#include "Addresses.hpp"

#include <sstream>

void AddressData::parse(std::istream& in)
{
	std::string lexeme;
	std::unordered_map<size_t, std::string> names;
	size_t strId = -1;

	auto errLoc = [&] {
		return std::to_string(size_t(in.tellg()) - lexeme.size() + 1);
	};

	while (in >> lexeme)
	{
		switch (lexeme[0])
		{
		case '#': // Comment
			in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			break;
		default:
			if (strId != size_t(-1))
			{
				uintptr_t v = 0;
				std::istringstream ss{ lexeme };
				ss >> std::hex >> v;

				if (ss.fail() || !ss.eof())
					throw std::invalid_argument("Invalid token \"" + lexeme + "\" at " + errLoc());

				if (this->addresses.count(strId))
				{
					auto& key = names[strId];
					throw std::logic_error("Duplicate entry for \"" + key + "\" at " + errLoc());
				}

				this->addresses.emplace(strId, v);
				strId = -1;
			}
			else
			{
				strId = this->hash(lexeme.c_str());
				names.emplace(strId, lexeme);
			}
		}
	}
}

uintptr_t AddressData::get(size_t hash, const char* str) const
{
	if (!this->addresses.count(hash))
		throw std::invalid_argument("Key '" + std::string(str) + "' is missing or not valid");

	return this->addresses.at(hash);
}