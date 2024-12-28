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

static std::vector<size_t> find_single_quotes(std::string& command) {
	std::vector<size_t> out;
	for (size_t i = 0; i < command.size(); i++)
	{
		if (command[i] == '\'') out.push_back(i);
	}

	return out;
}

static std::vector<size_t> find_double_quotes(std::string& command) {
	std::vector<size_t> out;
	for (size_t i = 0; i < command.size(); i++) {
		if (command[i] == '\"') out.push_back(i);
	}

	return out;
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

static void extract_string_between(std::vector<std::string>& parsed_input, std::string& command, size_t a, size_t b) {
	if (b - a > 0) {
		std::string sub = command.substr(a, b - a);
		parsed_input.push_back(sub);
	}
}

static std::vector<std::string> parse_input(std::string& command) {
	size_t a = 0;
	std::string sub;
	std::vector<std::string> parsed_input;

	// get builtin command first
	for (size_t i = 0; i < command.size(); i++) {
		if (command[i] == ' ') {
			if ((i - a) < 0) break;
			size_t c_range = i - a;
			sub = command.substr(a, c_range);
			parsed_input.push_back(sub);
			a = i + 1;
			break;
		}
	}

	std::vector<size_t> single_quote_indices = find_single_quotes(command);
	std::vector<size_t> double_quote_indices = find_double_quotes(command);

	if (single_quote_indices.size() > 0 && single_quote_indices.size() % 2 == 0) {
		// enclosed in single quotes
		size_t x = 0;
		size_t y = 1;

		while (x < single_quote_indices.size()) {
			extract_string_between(parsed_input, command, single_quote_indices[x] + 1, single_quote_indices[y]);
			x += 2;
			y += 2;
		}
	}
	else if (double_quote_indices.size() > 0 && double_quote_indices.size() % 2 == 0) {
		// enclosed in double quotes
		// IMPORTANT: If backslash (\) is followed by $, `, or \ it should be removed;
		size_t x = 0;
		size_t y = 1;

		while (x < double_quote_indices.size()) {
			extract_string_between(parsed_input, command, double_quote_indices[x] + 1, double_quote_indices[y]);
			x += 2;
			y += 2;
		}
	}
	else {
		// not enclosed in quotes
		for (size_t i = a; i < command.size(); i++) {
			if (command[i] == ' ') {
				if ((i - a) < 0) break;
				size_t c_range = i - a;
				sub = command.substr(a, c_range);
				// goal to skip whitespace here and create space in method where I handle echo functionality
				a = i + 1;
				if (sub == "") continue;
				parsed_input.push_back(sub);
			}
		}
		sub = command.substr(a, command.size() - a);
		parsed_input.push_back(sub);
	}
	for (size_t i = 0; i < parsed_input.size(); i++)
	{
		DBM(parsed_input[i]);
	}
	return parsed_input;
}

// revised_parse_input():
// 1. get command [DONE]
// 2. traverse input [DOING]
	// a. if single quote or double quote seen, look for closing quote
	// b. if closing quote found, extract contents
	// c. else, find spaces and extract contents like that

static std::vector<std::string> revised_parse_input(std::string& command) {
	int a = 0;
	std::string sub;
	std::vector<std::string> parsed_input;

	// get builtin command first
	for (size_t i = 0; i < command.size(); i++) {
		if (command[i] == ' ') {
			if ((i - a) < 0) break;
			size_t c_range = i - a;
			sub = command.substr(a, c_range);
			parsed_input.push_back(sub);
			a = i + 1;
			break;
		}
	}
	//DBM(command.size());
	int b = a;
	while (a < command.size()) {
		if (command[a] == '\'') {
			size_t opening_quote_index = a;
			size_t closing_quote_index = a + 1;
			// look for closing single quote
			while (closing_quote_index < command.size()) {
				if (command[closing_quote_index] == '\'') {
					// then extract middle contents
					extract_string_between(parsed_input, command, opening_quote_index + 1, closing_quote_index);
					a = closing_quote_index + 1;
					b = a;
					break;
				}
				closing_quote_index++;
			}
		}
		else if (command[a] == '\"') {
			size_t opening_quote_index = a;
			size_t closing_quote_index = a + 1;
			// look for closing double quote
			while (closing_quote_index < command.size()) {
				if (command[closing_quote_index] == '\"') {
					// then extract middle contents
					extract_string_between(parsed_input, command, opening_quote_index + 1, closing_quote_index);
					a = closing_quote_index + 1;
					b = a;
					break;
				}
				closing_quote_index++;
			}
		}
		else if (command[a] == ' ') {
			// find spaces and extract args like that
			if (a - b >= 0) {
				int c_range = a - b;
				sub = command.substr(b, c_range);
			
				// goal to skip whitespace here and create space in method where I handle echo functionality
				b = a + 1;

				if (sub == "") continue;
				parsed_input.push_back(sub);
			}
		}
		a++;
	}
	sub = command.substr(b, command.size() - b);
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
		std::vector<std::string> parsed_input = revised_parse_input(input);

		// check for exit
		if (is_exit(input)) {
			exit = true;
			continue;
		}

		std::string command = parsed_input[0];

		// check for valid command
		if (!validate_command(command, valid_commands)) {
			// check executable
			std::vector<std::string> args = parsed_input;
			std::tuple<bool, std::string> validated_path = validate_path(paths, command);
			if (std::get<0>(validated_path)) {
				// allow for multiple args
				std::string fullpath = std::get<1>(validated_path).c_str();
				for (size_t i = 1; i < args.size(); i++) {
					fullpath += std::format(" '{}'", args[i]);
				}

				// AND KEEP IN MIND: below code won't work if there is a valid path that isn't executable
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
			if (parsed_input[1][0] == '~') {
				if (const char* env_h = std::getenv("HOME")) {
					std::string path_remainder;
					if (parsed_input[1].size() > 1) {
						// check for ~/dir command
						path_remainder = parsed_input[1].substr(1, parsed_input[1].size());
						p = fs::path(std::format("{}/{}", env_h, path_remainder.c_str()));
					}
					else {
						// check for ~ command
						p = fs::path(std::format("{}", env_h));
					}
				}
			}

			if (fs::exists(p)) fs::current_path(p);
			else std::cout << std::format("cd: {}: No such file or directory", p.string()) << std::endl;
			continue;
		}
	}
}
