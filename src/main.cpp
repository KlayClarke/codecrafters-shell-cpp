#include <iostream>
#include <string>
#include <vector>
#include <format>

const std::string EXIT = "exit 0";

const std::string ECHO = "echo";

static bool is_echo(std::string command) {
	// if command == ECHO command, true; else false
	if (command == ECHO) return true;
	return false;
}

static bool is_exit(std::string command) {
	// if command == EXIT command, true; else false
	if (command == EXIT) return true;
	return false;
}

static std::vector<std::string> parse_input(std::string command) {
	int cur = 0;
	std::string sub;
	std::string command_cp = command;
	std::vector<std::string> parsed_input;

	for (int i = 0; i < command.size(); i++) {
		if (command[i] == ' ') {
			if ((i - cur) < 0) break;
			int c_range = i - cur;
			sub = command.substr(cur, c_range);
			parsed_input.push_back(sub);
			cur = i + 1;
		}
	}

	sub = command.substr(cur, command.size() - cur);
	parsed_input.push_back(sub);

	return parsed_input;
}

static bool validate_command(std::string command, std::vector<std::string> valid_commands) {
	// if command valid, true; else false
	for (int i = 0; i < valid_commands.size(); i++) {
		if (valid_commands[i] == command) return true;
	}
	return false;
}

int main() {
	// Flush after every std::cout / std:cerr
	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	// store valid commands
	std::vector<std::string> valid_commands{ECHO};

	bool exit = false;

	while (!exit) {
		// Uncomment this block to pass the first stage
		std::cout << "$ ";

		std::string input;
		std::getline(std::cin, input);

		// parse / separate command and args
		std::vector<std::string> parsed_input = parse_input(input);

		// check for exit
		if (is_exit(input)) {
			exit = true;
			continue;
		}

		std::string command = parsed_input[0];

		// check for valid command
		if (!validate_command(command, valid_commands)) {
			std::string invalid_command_response = std::format("{}: command not found", input);
			std::cout << invalid_command_response << std::endl;
			continue;
		}

		// check for echo command
		if (is_echo(command)) {
			// handle echo
			std::string to_echo = "";
			for (auto& entry : parsed_input) {
				if (entry == command) continue;
				to_echo += std::format("{} ", entry); // add space after each word
			}
			to_echo = to_echo.substr(0, to_echo.size() - 1); // remove final space
			std::cout << to_echo << std::endl;
		}
	}
}
