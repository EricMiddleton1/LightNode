#pragma once

#include <array>
#include <unordered_map>

class GammaTable {
public:
	using Table = std::array<uint16_t, 256>;

	static const Table& get(float gamma);

private:
	static Table createTable(float gamma);

	static std::unordered_map<float, Table> tables;
};
