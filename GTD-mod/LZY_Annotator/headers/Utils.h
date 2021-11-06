#pragma once
#include <iostream>
#include <vector>
#include <sstream> 
#include <cstdlib>
#include <string>
#include <fstream>


std::vector<std::vector<std::string>> readStringCSV(std::string filePath);
std::vector<std::vector<float>> readFloatCSV(std::string filePath);

