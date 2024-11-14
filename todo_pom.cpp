#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <sys/select.h>
#include <unistd.h> // For STDIN_FILENO
#include <termios.h>
#include <iomanip> // For formatted output

using namespace std;

struct Task {
    string content;
    int lineNumber;
};

// Constants for Pomodoro duration (in seconds)
const int WORK_DURATION = 1 * 60;  // 25 minutes
const int BREAK_DURATION = 5 * 60;  // 5 minutes

// Global variable to store remaining time (in seconds)
int remainingTime = WORK_DURATION;

// ANSI Escape Codes for Styling
const string RESET = "\033[0m";
const string BOLD = "\033[1m";
const string UNDERLINE = "\033[4m";
const string RED = "\033[0;31m";
const string GREEN = "\033[0;32m";
const string YELLOW = "\033[0;33m";
const string BLUE = "\033[0;34m";

// Load tasks from org file
vector<Task> loadTasks(const string &filename) {
    vector<Task> tasks;
    ifstream file(filename);
    string line;
    int lineNumber = 0;

    while (getline(file, line)) {
        if (line.find("* TODO") != string::npos) {
            tasks.push_back({line, lineNumber});
        }
        lineNumber++;
    }
    file.close();
    return tasks;
};

void markTaskAsDone(const string &filename, Task &task) {
    vector<string> lines;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    size_t pos = task.content.find("TODO");
    if (pos != string::npos) {
        task.content.replace(pos, 4, "DONE");
    }
    lines[task.lineNumber] = task.content;

    ofstream outFile(filename);
    for (const auto &line : lines) {
        outFile << line << endl;
    }
    outFile.close();
}

// Set terminal to raw mode to capture keypresses
void setRawMode(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

// Check if there is any input available in stdin
bool isInputAvailable() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout) > 0;
}


// Function to display a dynamic progress bar
void displayProgressBar(int elapsedTime, int totalDuration) {
    int progress = (elapsedTime * 50) / totalDuration; // Calculate progress (max 50 characters)
    string bar = "[";

    for (int i = 0; i < progress; ++i) {
        bar += "=";  // Completed part of the bar
    }

    for (int i = progress; i < 50; ++i) {
        bar += " ";  // Remaining part of the bar
    }

    bar += "]";
    // Print the progress bar with the remaining time
    cout << "\r" << BLUE << bar << RESET << " " << (totalDuration - elapsedTime) / 60 << "m " << (totalDuration - elapsedTime) % 60 << "s remaining";
    cout.flush();
}







// Pomodoro timer function with dynamic progress bar
void pomodoroTimer(int& remainingTimeForTask) {
    cout << GREEN << BOLD << "Pomodoro session started. Press 'c' to complete the task at any time." << RESET << endl;

    setRawMode(true);

    int elapsedTime = 0;

    // Work session loop
    while (elapsedTime < remainingTimeForTask) {
        displayProgressBar(elapsedTime, remainingTimeForTask);
        this_thread::sleep_for(chrono::seconds(1));
        elapsedTime++;

        if (isInputAvailable()) {
            char ch;
            read(STDIN_FILENO, &ch, 1);
            if (ch == 'c' || ch == 'C') {
                cout << "\n" << GREEN << BOLD << "Task completed early!" << RESET << endl;
                remainingTimeForTask -= elapsedTime;  // Update remaining time after early completion
                setRawMode(false);
                return;
            }
        }
    }

    cout << "\n" << YELLOW << "Work session complete. Starting break..." << RESET << endl;
    elapsedTime = 0;

    // Break session loop
    while (elapsedTime < BREAK_DURATION) {
        displayProgressBar(elapsedTime, BREAK_DURATION);
        this_thread::sleep_for(chrono::seconds(1));
        elapsedTime++;
    }

    cout << "\n" << GREEN << "Break over! Starting another Pomodoro session..." << RESET << endl;
    remainingTimeForTask = WORK_DURATION;  // Reset the remaining time for the next task
    setRawMode(false);
}





int main() {
    vector<string> orgFiles = {"/home/zaine/master-folder/org_files/master.org"};
    if (orgFiles.empty()) {
        cout << RED << "No .org files found in the specified directory." << RESET << endl;
        return 1;
    }

    vector<Task> tasks;
    for (const auto &filePath : orgFiles) {
        vector<Task> fileTasks = loadTasks(filePath);
        tasks.insert(tasks.end(), fileTasks.begin(), fileTasks.end());
    }
    if (tasks.empty()) {
        cout << RED << "No TODO tasks found in the specified .org file." << RESET << endl;
        return 1;
    }

    bool continueSession = true;
    while (continueSession && !tasks.empty()) {
        if (orgFiles.size() < 1) {
            cout << RED << "No .org files found in the specified directory." << RESET << endl;
            return 1;
        }
        cout << "\n" << BLUE << BOLD << "--- TODO Tasks ---" << RESET << endl;

        // Display tasks
        for (size_t i = 0; i < tasks.size(); ++i) {
            cout << BLUE << i + 1 << ". " << tasks[i].content << RESET << endl;
        }

        int taskIndex;
        cout << "\n" << BOLD << "Select a task to start Pomodoro (enter task number): " << RESET;
        cin >> taskIndex;
        if (taskIndex < 1 || taskIndex > tasks.size()) {
            cout << RED << "Invalid task number. Please try again." << RESET << endl;
            continue;
        }
        if (taskIndex > 0 && taskIndex <= tasks.size()) {
            Task &selectedTask = tasks[taskIndex - 1];
            cout << GREEN << "Selected task: " << selectedTask.content << RESET << endl;

            if (remainingTime < WORK_DURATION) {
                cout << YELLOW << "Resuming previous Pomodoro with " << remainingTime / 60 << " minutes remaining." << RESET << endl;
            }

            pomodoroTimer(remainingTime);

            cout << GREEN << "Marking task as DONE in the file..." << RESET << endl;
            markTaskAsDone(orgFiles[0], selectedTask);
            cout << GREEN << "Task marked as DONE." << RESET << endl;

            tasks.erase(tasks.begin() + (taskIndex - 1));
        } else {
            cout << RED << "Invalid task number. Please try again." << RESET << endl;
            continue;
        }

        char continueChoice;
        cout << BOLD << "Do you want to select another task? (y/n): " << RESET;
        cin >> continueChoice;
        if (continueChoice != 'y' && continueChoice != 'Y' && continueChoice != 'n' && continueChoice != 'N') {
            cout << RED << "Invalid choice. Please try again." << RESET << endl;
            continue;
        }
        continueSession = (continueChoice == 'y' || continueChoice == 'Y');
        if (continueSession) {
            if (tasks.empty()) {
                cout << YELLOW << "No more tasks remaining. Exiting..." << RESET << endl;
                continueSession = false;
                return 0;
            }
        }
    }

    cout << GREEN << "Session complete. Exiting..." << RESET << endl;
    return 0;
}
