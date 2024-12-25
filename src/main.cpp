#include <iostream>
#include <string>
#include <vector>
#include <format>

// BUILTINS
const std::string EXIT = "exit";
const std::string EXIT0 = "exit 0";
const std::string ECHO = "echo";
const std::string TYPE = "type";

static bool is_type(std::string command) {
	// if command == TYPE command, true; else false
	if (command == TYPE) return true;
	return false;
}

static bool is_echo(std::string command) {
	// if command == ECHO command, true; else false
	if (command == ECHO) return true;
	return false;
}

static bool is_exit(std::string command) {
	// if command == EXIT command, true; else false
	if (command == EXIT0) return true;
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
	std::vector<std::string> valid_commands{EXIT, EXIT0, ECHO, TYPE};

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
			std::cout << std::format("{}: command not found", input) << std::endl;
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

		// check for type command
		if (is_type(command)) {
			if (validate_command(parsed_input[1], valid_commands)) {
				// handle builtin command
				for (int i = 0; i < valid_commands.size(); i++)
				{
					if (valid_commands[i] == parsed_input[1]) std::cout << std::format("{} is a shell builtin", parsed_input[1]) << std::endl;
				}
			}
			else {
				//handle unrecognized commands
				std::cout << std::format("{}: not found", parsed_input[1]) << std::endl;
			}
		}
	}
}
