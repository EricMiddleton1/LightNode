#include "GammaTable.hpp"

#include <cmath>

std::unordered_map<float, GammaTable::Table> GammaTable::tables;

const GammaTable::Table& GammaTable::get(float gamma) {
	try {
		return tables.at(gamma);
	}
	catch(const std::exception& e) {
		tables[gamma] = createTable(gamma);

		return tables[gamma];
	}
}

GammaTable::Table GammaTable::createTable(float gamma) {
	Table table;

	for(int i = 0; i < table.size(); ++i) {
		table[i] = 65535*std::pow(static_cast<float>(i)/(table.size()-1), gamma) + 0.5f;
	}
}
