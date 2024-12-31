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

// SPECIAL CHARS
const std::vector<char> SPEC_CHARS {'\\', '$', '\"'};

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

static void escape_special_chars(std::string& command, size_t starting_index) {
	size_t special_char_index = starting_index + 1;
	for (auto c : SPEC_CHARS) {
		if (c == command[special_char_index + 1]) {
			command.erase(command.begin() + special_char_index);
		}
	}
}

static size_t find_closing_quote_index(std::string& command, size_t starting_index, bool single_quote=false) {
	size_t closing_quote_index = starting_index + 1;
	if (single_quote) {
		while (closing_quote_index < command.size()) {
			if (command[closing_quote_index] == '\'') {
				// then extract middle contents
				return closing_quote_index;
			}
			closing_quote_index++;
		}
	}
	else {
		while (closing_quote_index < command.size()) {
			if (command[closing_quote_index] == '\"') {
				// then extract middle contents
				return closing_quote_index;
			}
			else if (command[closing_quote_index] == '\\') {
				escape_special_chars(command, closing_quote_index);
			}
			closing_quote_index++;
		}
	}

	return -1;
}

static std::vector<std::string> parse_input(std::string& command) {
	size_t a = 0;
	std::vector<std::string> parsed_input;

	// traverses command and separates args using spaces, double quotes, single quotes
	for (size_t i = 0; i < command.size(); i++) {
		// if space
		if (command[i] == ' ') {
			extract_string_between(parsed_input, command, a, i); // doesn't include space (command[i])
			if (parsed_input[parsed_input.size() - 1] != " ") parsed_input.push_back(" ");
			a = i + 1;
		}
		// if double quotes
		else if (command[i] == '\"') {
			size_t closing_quote_index = find_closing_quote_index(command, i);
			if (closing_quote_index != -1) {
				extract_string_between(parsed_input, command, i + 1, closing_quote_index);
				if (parsed_input[parsed_input.size() - 1] != " ") parsed_input.push_back(" ");
				i = closing_quote_index;
				a = i + 1;
			}
		}
		// if single quotes
		else if (command[i] == '\'') {
			size_t closing_quote_index = find_closing_quote_index(command, i, true);
			if (closing_quote_index != -1) {
				extract_string_between(parsed_input, command, i + 1, closing_quote_index);
				if (parsed_input[parsed_input.size() - 1] != " ") parsed_input.push_back(" ");
				i = closing_quote_index;
				a = i + 1;
			}
		}
		// if backslash outside of quotes
		else if (command[i] == '\\') {
			// remove backslash
			command.erase(command.begin() + i);
			i = i - 1;
		}
		// if end of command
		else if (i == command.size() - 1) {
			extract_string_between(parsed_input, command, a, i + 1); // includes command[i]
		}
	}
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
			std::tuple<bool, std::string> validated_path = validate_path(paths, command);
			if (std::get<0>(validated_path)) {
				// allow for multiple args
				std::string fullpath = std::get<1>(validated_path).c_str();
				for (size_t i = 2; i < parsed_input.size(); i++) {
					if (i % 2 == 0) fullpath += std::format(" \"{}\"", parsed_input[i]);
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
			for (size_t i = 2; i < parsed_input.size(); i++) { // skip echo command string and the space after
				to_echo += std::format("{}", parsed_input[i]);
			}
			std::cout << to_echo << std::endl;
			continue;
		}

		// check for type command
		if (is_type(command)) {
			std::string type_arg = parsed_input[2];
			if (validate_command(type_arg, valid_commands)) {
				// handle builtin command
				for (int i = 0; i < valid_commands.size(); i++)
				{
					if (valid_commands[i] == type_arg) std::cout << std::format("{} is a shell builtin", type_arg) << std::endl;
				}
			}
			else if (std::get<0>(validate_path(paths, type_arg))) {
				// handle paths
				// FIGURE OUT HOW TO DO THIS WITHOUT RUNNING "VALIDATE_PATH" TWICE!!
				std::cout << std::format("{} is {}", type_arg, std::get<1>(validate_path(paths, type_arg))) << std::endl;
			}
			else {
				//handle unrecognized commands
				std::cout << std::format("{}: not found", type_arg) << std::endl;
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
			fs::path p = fs::path(parsed_input[2]);
			if (parsed_input[2][0] == '~') {
				if (const char* env_h = std::getenv("HOME")) {
					std::string path_remainder;
					if (parsed_input[2].size() > 1) {
						// check for ~/dir command
						path_remainder = parsed_input[2].substr(1, parsed_input[2].size());
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

// get this (echo "klay"clarke) to print out (klayclarke) instead of (klay clarke)
// need to save spaces

// for revising parse_input, need to figure out how to parse each arg and save spaces