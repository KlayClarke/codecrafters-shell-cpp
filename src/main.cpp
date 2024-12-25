#include <iostream>
#include <string>
#include <vector>
#include <format>

static bool validate_command(std::string command, std::vector<std::string> valid_commands) {
	// if command valid, true; else false
	for (int i = 0; i < valid_commands.size(); i++) {
		if (valid_commands[i] == command) {
			return true;
		}
	}

	return false;
}

int main() {
	// Flush after every std::cout / std:cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	// store valid commands
	std::vector<std::string> valid_commands{};

	bool exit = false;

	while (!exit) {
		// Uncomment this block to pass the first stage
		std::cout << "$ ";

		std::string input;
		std::getline(std::cin, input);

		if (!validate_command(input, valid_commands)) {
			std::string invalid_command_response = std::format("{}: command not found", input);
			std::cout << invalid_command_response << std::endl;
		}
	}
}
