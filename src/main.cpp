#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <csignal>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

struct ProcessInfo
{
    pid_t pid;
    std::string name;
};

// Function to check if a given process name contains the partial name
auto process_matches(const std::string& processName, const std::string& partialName) -> bool
{
    return processName.find(partialName) != std::string::npos;
}

// Function to get the name of a process given its PID
auto get_process_name(pid_t pid) -> std::string
{
    std::string proc_path = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream proc_file(proc_path);
    if (proc_file.is_open()) {
        std::string name;
        std::getline(proc_file, name);
        return name;
    }
    return "";
}

// Function to find processes that match the partial name
auto find_processes_by_name(const std::string& partialName) -> std::vector<ProcessInfo> {
    std::vector<ProcessInfo> matching_processes;
    DIR* proc_dir = opendir("/proc");
    if (proc_dir == nullptr) {
        std::cerr << "Error: Unable to open /proc directory!" << std::endl;
        return matching_processes;
    }

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            pid_t pid = atoi(entry->d_name);
            if (pid > 0) {  // Skip non-PID entries
                std::string process_name = get_process_name(pid);
                if (process_matches(process_name, partialName)) {
                    matching_processes.push_back({ pid, process_name });
                }
            }
        }
    }
    closedir(proc_dir);
    return matching_processes;
}

// Function to kill a process given its PID using SIGKILL
auto kill_process(pid_t pid) -> bool {
    return kill(pid, SIGKILL) == 0;
}

// Function to parse a comma-separated string of numbers
auto parse_numbers(const std::string& input) -> std::vector<int> {
    std::vector<int> numbers;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, ',')) {
        try {
            numbers.push_back(std::stoi(item));
        } catch (const std::invalid_argument&) {
            std::cerr << "Invalid number: " << item << std::endl;
        }
    }
    return numbers;
}

auto main(int argc, char* argv[]) -> int {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <partial_process_name> [--list]" << std::endl;
        return 1;
    }

    std::string partial_name = argv[1];
    bool list_only = (argc == 3 && std::string(argv[2]) == "--list");

    std::vector<ProcessInfo> processes = find_processes_by_name(partial_name);

    if (processes.empty()) {
        std::cout << "No processes found with the partial name \"" << partial_name << "\"." << std::endl;
        return 0;
    }

    if (list_only) {
        std::cout << "Found the following processes:" << std::endl;
        for (size_t i = 0; i < processes.size(); ++i) {
            std::cout << i + 1 << ". " << processes[i].name << " (PID: " << processes[i].pid << ")" << std::endl;
        }

        std::cout << "Enter the numbers of the processes you want to terminate (comma-separated): ";
        std::string input;
        std::cin >> input;

        std::vector<int> numbers = parse_numbers(input);
        for (int number : numbers) {
            if (number > 0 && static_cast<size_t>(number) <= processes.size()) {
                pid_t pid = processes[number - 1].pid;
                if (kill_process(pid)) {
                    std::cout << "Terminated process " << processes[number - 1].name << " (PID: " << pid << ")" << std::endl;
                } else {
                    std::cerr << "Failed to terminate process " << processes[number - 1].name << " (PID: " << pid << ")" << std::endl;
                }
            } else {
                std::cerr << "Invalid process number: " << number << std::endl;
            }
        }
    } else {
        std::cout << "Do you want to terminate all found processes? (y/n): ";
        char choice;
        std::cin >> choice;

        if (choice == 'y' || choice == 'Y') {
            for (const auto& process : processes) {
                if (kill_process(process.pid)) {
                    std::cout << "Terminated process " << process.name << " (PID: " << process.pid << ")" << std::endl;
                } else {
                    std::cerr << "Failed to terminate process " << process.name << " (PID: " << process.pid << ")" << std::endl;
                }
            }
        } else
        {
            std::cout << "No processes were terminated." << std::endl;
        }
    }

    return 0;
}
