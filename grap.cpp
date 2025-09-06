#include <iostream>
#include <string>
#include <filesystem>   // For directory traversal (C++17+)
#include <fstream>      // For file reading
#include <vector>       // If storing matches or file lists
#include <system_error> // For handling filesystem errors
#include <unistd.h>

constexpr const char* COLOR_RED     = "\x1B[91m";
constexpr const char* COLOR_MAGENTA = "\x1B[95m";
constexpr const char* COLOR_GREEN   = "\x1B[92m";
constexpr const char* COLOR_RESET   = "\x1B[0m";

namespace fs 	= std::filesystem;

enum ExitCode {
    MATCH_FOUND = 0,
    NO_MATCH = 1,
    ERROR_OCCURRED = 2
};

//Initialize in main() based on command-line arguments
size_t file_print_offset;

ExitCode search_file(const std::string& file_path, const std::string& pattern) {
    std::ifstream file(file_path);
    if (!file) throw std::runtime_error("Could not open file: " + file_path);

    std::string line;
    int line_number = 0;
    bool printed_path = false;

    bool is_terminal = isatty(STDOUT_FILENO);
    const char* magenta = is_terminal ? COLOR_MAGENTA : "";
    const char* green   = is_terminal ? COLOR_GREEN   : "";
    const char* red     = is_terminal ? COLOR_RED     : "";
    const char* reset   = is_terminal ? COLOR_RESET   : "";

    while (std::getline(file, line)) {
        line_number += 1;

		size_t found_pos = line.find(pattern, 0);
		if (found_pos != std::string::npos){
			if (!printed_path) {
				std::cout << magenta << file_path.substr(file_print_offset) << reset << "\n";
				printed_path = true;
			}
			std::cout << green << line_number << ": " << reset;
			std::cout << line.substr(0, found_pos);
			std::cout << red << pattern << reset;
			std::cout << line.substr(found_pos + pattern.length());
            std::cout << "\n";
		}
    }

    return printed_path ? MATCH_FOUND : NO_MATCH;
}


ExitCode traverse_directory(const std::string& path, const std::string& pattern){
	try{
		//Iterating over directory entries recursively
		//recursive_directory_iterator will automatically not follow ".", ".." and symlinks
		ExitCode ret = NO_MATCH;
		for(const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(path)){
			if(dir_entry.is_regular_file()){
				if(search_file(dir_entry.path(), pattern) == MATCH_FOUND && ret == NO_MATCH){
					ret = MATCH_FOUND;
				}
				if(ret == ERROR_OCCURRED) break;
			}
		}
		return ret;
	} catch(const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return ERROR_OCCURRED;
	}
}

/* grap should terminate with exit code 0 if a match is found, 1 if no match was found, and 2 if an error occurred
 * This mimics the behavior of the grep utility
 * argc is either 2 or 3, argv[1] = pattern, argv[2] = path
 */
int main(int argc, char **argv){
	std::string directory_path = ".";
	std::string pattern;
	
	if(argc == 2){
		pattern = argv[1];
		file_print_offset = 2; //Skip "./" prefix if no directory argument
	} else if(argc == 3){
		pattern = argv[1];
		directory_path = argv[2];
		file_print_offset = 0; //Print full path if directory argument is given
	} else{
		std::cout << "usage: greptile <pattern> [directory] \n";
		return ERROR_OCCURRED;
	}
	//Return 0 if a match was found, 1 otherwise
	return (traverse_directory(directory_path, pattern) == 0) ? 0 : 1;
}
