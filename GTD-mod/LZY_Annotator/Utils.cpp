#pragma once

#include "headers/Utils.h"

std::vector<std::vector<std::string>> readStringCSV(std::string filePath)
{
	std::string line = "";
	std::ifstream file(filePath);
	std::vector<std::vector<std::string>> result;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::vector<std::string> newEntry;

		while (lineStream.good())
		{
			std::string cell = "";
			std::getline(lineStream, cell, ',');
			newEntry.push_back(cell);
		}
		result.push_back(newEntry);
	}
	file.close();
	return result;
}

std::vector<std::vector<float>> readFloatCSV(std::string filePath)
{
	std::string line = "";
	std::ifstream file(filePath);
	std::vector<std::vector<float>> result;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::vector<float> newEntry;

		while (lineStream.good())
		{
			std::string cell = "";
			std::getline(lineStream, cell, ',');
			newEntry.push_back(std::stof(cell));
		}
		result.push_back(newEntry);
	}
	file.close();
	return result;
}

