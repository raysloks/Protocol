#pragma once

#include <vector>
#include <memory>
#include <filesystem>

class Protocol;

class Config
{
public:
	void load(const std::filesystem::path& file);

	std::vector<std::unique_ptr<Protocol>> protocols;
};

