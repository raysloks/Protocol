#pragma once

#include <sstream>
#include <vector>

std::string extract_token(std::istringstream& ss);
std::vector<std::string> extract_tokens(std::istringstream& ss);