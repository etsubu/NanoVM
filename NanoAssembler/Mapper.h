#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

class Mapper {
public:
	Mapper();
	~Mapper();
	bool mapOpcode(std::string, std::pair<unsigned char, unsigned int>&);
	bool mapRegister(std::string, unsigned char&);
	int mapImmediate(std::string, std::vector<unsigned char>&);
private:
	std::unordered_map<std::string, std::pair<unsigned char, unsigned int>> opcodeMap;
	std::unordered_map<std::string, unsigned char> registerMap;
};