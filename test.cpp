#include "range.hpp"
#include <vector>
#include <iostream>
#include <string>

int main() {
	std::vector<int> input;
	for (int i = 0; i < 50; ++i) {
		input.push_back(i);
	}

	auto rangeMap = map( input, [](int x){ return x+1; });
	std::cout << "rangeMap" << std::endl;
	for ( int x : rangeMap ) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	auto rangeFilter = filter( input, [](int x) { return x % 2 == 0; });
	std::cout << "rangeFilter" << std::endl;
	for ( int x : rangeFilter ) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	std::vector<int> input1;
	for (int i = 10; i > 0; --i) {
		input1.push_back(i);
	}

	auto rangeZip = zip( input, input1 );
	std::cout << "rangeZip" << std::endl;
	for ( const std::pair<int, int>& p : rangeZip ) {
		std::cout << p.first << ", " << p.second << std::endl;
	}

	auto rangeRange = range(1, 32, 3);
	std::cout << "rangeRange" << std::endl;
	for (const auto& x : rangeRange) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	rangeRange = range(10, 0, -3);
	for (const auto& x : rangeRange) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	rangeRange = range(0, 4, -2);
	for (const auto& x : rangeRange) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	rangeRange = range(0, -4, 2);
	for (const auto& x : rangeRange) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	std::cout << "take and infiniteSequence" << std::endl;
	auto rangeTake = take(input, 5);
	for ( int x : rangeTake ) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	for (int x : take(infiniteSequence(0, -1), 10)) {
		std::cout << x << " ";
	}
	std::cout << std::endl;

	std::string s = "ABCD";
	std::cout << "enumerate" << std::endl;
	auto rangeEnumerate = enumerate(s);
	for ( const auto& p : rangeEnumerate ) {
		std::cout << p.first << " " << p.second << std::endl;
	}

	for ( auto [i, c] : s | enumerate() ) {
		std::cout << i << " " << c << std::endl;
	}

	std::vector<int> input2{ 1, 1, 1, 1, 1 };
	auto rangeTake1 = take(input2, 3);
	input2[0] = 0;
	for (int x : rangeTake1) {
		std::cout << x << " ";
	}
	std::cout << std::endl;
}
