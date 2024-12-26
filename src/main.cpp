#include <iostream>
#include <string>
#include <vector>
#include <format>
#include <filesystem>
#include <cstdlib>
#include <tuple>

#define DBM(str) do { std::cout << str << std::endl; } while (false) // Debug messages simplified

namespace fs = std::filesystem;

// BUILTINS
const std::string EXIT = "exit";
const std::string EXIT0 = "exit 0";
const std::string ECHO = "echo";
const std::string TYPE = "type";
const std::string PWD = "pwd";
const std::string CD = "cd";

static bool is_cd(std::string command) {
	// if command == CD command, true; else false
	if (command == CD) return true;
	return false;
}

static bool is_pwd(std::string command) {
	// if command == PWD command, true; else false
	if (command == PWD) return true;
	return false;
}

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

static std::vector<std::string> parse_paths(std::string paths) {
	std::string token;
	std::vector<std::string> tokens;
	size_t pos = 0;

#ifdef linux
	char delim = ':';

	while ((pos = paths.find(delim)) != std::string::npos) {
		token = paths.substr(0, pos);
		tokens.push_back(token);
		paths.erase(0, pos + 1);
	}
	
	tokens.push_back(paths);
	return tokens;
#endif

#ifdef _WIN32
	char delim = ';';

	while ((pos = paths.find(delim)) != std::string::npos) {
		token = paths.substr(0, pos);
		tokens.push_back(token);
		paths.erase(0, pos + 1);
	}

	tokens.push_back(paths);
	return tokens;
#endif
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

static std::tuple<bool, std::string> validate_path(std::vector<std::string> paths, std::string filename) {
	// check whether path + file_name is in file system
	for (int i = 0; i < paths.size(); i++) {
		std::string fullpath = std::format("{}/{}", paths[i], filename);
		fs::path f{ fullpath };
		if (fs::exists(f)) return std::tuple<bool, std::string>{true, fullpath};
	}
	return std::tuple<bool, std::string>{false, ""};
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
	std::vector<std::string> valid_commands{EXIT, EXIT0, ECHO, TYPE, PWD, CD};

	// store paths
	std::vector<std::string> paths;
	if (const char* env_p = std::getenv("PATH")) paths = parse_paths(env_p);

	// repl
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
			// check executable
			std::string arg;
			if (parsed_input.size() > 1) arg = parsed_input[1];
			std::tuple<bool, std::string> validated_path = validate_path(paths, command);
			if (std::get<0>(validated_path)) {
				std::string fullpath = std::format("{} {}", std::get<1>(validated_path).c_str(), parsed_input[1]);
				// KEEP IN MIND: below code won't work if there is a valid path that isn't executable
				if (system(fullpath.c_str()) == 0) {
					continue;
				}
			}
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
			continue;
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
			else if (std::get<0>(validate_path(paths, parsed_input[1]))) {
				// handle paths
				// FIGURE OUT HOW TO DO THIS WITHOUT RUNNING "VALIDATE_PATH" TWICE!!
				std::cout << std::format("{} is {}", parsed_input[1], std::get<1>(validate_path(paths, parsed_input[1]))) << std::endl;
			}
			else {
				//handle unrecognized commands
				std::cout << std::format("{}: not found", parsed_input[1]) << std::endl;
			}
			continue;
		}

		// check for pwd command
		if (is_pwd(command)) {
			fs::path cwd = fs::current_path();
			std::cout << cwd.string() << std::endl;
			continue;
		}

		// check for cd command
		if (is_cd(command)) {
			fs::path p = fs::path(parsed_input[1]);
			if (fs::exists(p)) fs::current_path(p);
			else std::cout << std::format("cd: {}: No such file or directory", p.string()) << std::endl;
			continue;
		}
	}
}

// current goal: implement cd builtin for 
	//Absolute paths, like /usr/local/bin.
	//Relative paths, like ./, ../, ./dir.
	//The ~ character, which stands for the user's home directory