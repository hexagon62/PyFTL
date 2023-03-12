#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <string_view>

class AddressData
{
public:
	AddressData() = default;

	void parse(std::istream& in);

	uintptr_t get(size_t hash, const char* str) const;

	static constexpr size_t hash(const std::string_view& str)
	{
		static_assert(sizeof(size_t) == 4, "Only 32-bit is supported right now.");

		size_t h = 2166136261;

		for (char c : str)
		{
			h ^= c;
			h *= 16777619;
		}

		return h;
	}

	template<size_t N>
	static constexpr size_t hash(const char(&str)[N])
	{
		return AddressData::hash(std::string_view(str));
	}

private:
	std::unordered_map<size_t, uintptr_t> addresses;
};
