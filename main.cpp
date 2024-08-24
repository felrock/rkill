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

struct ProcessInfo {
    pid_t pid;
    std::string name;
};

// Function to check if a given process name contains the partial name
bool processMatches(const std::string& processName, const std::string& partialName) {
    return processName.find(partialName) != std::string::npos;
}

// Function to get the name of a process given its PID
std::string getProcessName(pid_t pid) {
    std::string procPath = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream procFile(procPath);
    if (procFile.is_open()) {
        std::string name;
        std::getline(procFile, name);
        return name;
    }
    return "";
}

// Function to find processes that match the partial name
std::vector<ProcessInfo> findProcessesByName(const std::string& partialName) {
    std::vector<ProcessInfo> matchingProcesses;
    DIR* procDir = opendir("/proc");
    if (procDir == nullptr) {
        std::cerr << "Error: Unable to open /proc directory!" << std::endl;
        return matchingProcesses;
    }

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            pid_t pid = atoi(entry->d_name);
            if (pid > 0) {  // Skip non-PID entries
                std::string processName = getProcessName(pid);
                if (processMatches(processName, partialName)) {
                    matchingProcesses.push_back({ pid, processName });
                }
            }
        }
    }
    closedir(procDir);
    return matchingProcesses;
}

// Function to kill a process given its PID using SIGKILL
bool killProcess(pid_t pid) {
    return kill(pid, SIGKILL) == 0;
}

// Function to parse a comma-separated string of numbers
std::vector<int> parseNumbers(const std::string& input) {
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <partial_process_name> [--list]" << std::endl;
        return 1;
    }

    std::string partialName = argv[1];
    bool listOnly = (argc == 3 && std::string(argv[2]) == "--list");

    std::vector<ProcessInfo> processes = findProcessesByName(partialName);

    if (processes.empty()) {
        std::cout << "No processes found with the partial name \"" << partialName << "\"." << std::endl;
        return 0;
    }

    if (listOnly) {
        std::cout << "Found the following processes:" << std::endl;
        for (size_t i = 0; i < processes.size(); ++i) {
            std::cout << i + 1 << ". " << processes[i].name << " (PID: " << processes[i].pid << ")" << std::endl;
        }

        std::cout << "Enter the numbers of the processes you want to terminate (comma-separated): ";
        std::string input;
        std::cin >> input;

        std::vector<int> numbers = parseNumbers(input);
        for (int number : numbers) {
            if (number > 0 && static_cast<size_t>(number) <= processes.size()) {
                pid_t pid = processes[number - 1].pid;
                if (killProcess(pid)) {
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
                if (killProcess(process.pid)) {
                    std::cout << "Terminated process " << process.name << " (PID: " << process.pid << ")" << std::endl;
                } else {
                    std::cerr << "Failed to terminate process " << process.name << " (PID: " << process.pid << ")" << std::endl;
                }
            }
        } else {
            std::cout << "No processes were terminated." << std::endl;
        }
    }

    return 0;
}
