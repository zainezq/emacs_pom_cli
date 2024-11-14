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

using namespace std;

struct Task {
    string content;
    int lineNumber;
};

// Constants for Pomodoro duration (in seconds)
const int WORK_DURATION = 25 * 60;  // 25 minutes
const int BREAK_DURATION = 5 * 60;  // 5 minutes

int remainingTime = WORK_DURATION;

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

    // Read the file line by line and store it in a vector
    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    // Find the position of "TODO" in the line and replace it with "DONE"
    size_t pos = task.content.find("TODO");
    if (pos != string::npos) {
        task.content.replace(pos, 4, "DONE");  // Replace "TODO" with "DONE"
    }
    lines[task.lineNumber] = task.content;  // Update the line in the vector

    // Write the updated lines back to the file
    ofstream outFile(filename);
    for (const auto &line : lines) {
        outFile << line << endl;
    }
    outFile.close();
}

// Function to set terminal to raw mode
void setRawMode(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);          // Save old terminal settings
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);        // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Set new terminal settings
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old terminal settings
    }
}

// Function to check if there is any input available in stdin
bool isInputAvailable() {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout) > 0;
}







// Pomodoro timer function
void pomodoroTimer() {
    cout << "Pomodoro session started. Press 'c' to complete the task at any time.\n";

    setRawMode(true); // Enable raw mode to capture key presses instantly

    // Work session loop
    while (remainingTime > 0) {
        cout << "Time remaining: " << remainingTime / 60 << " minutes\r";
        cout.flush();
        this_thread::sleep_for(chrono::seconds(1));
        remainingTime--;

        // Check if the user pressed 'c' to complete the task early
        if (isInputAvailable()) {
            char ch;
            read(STDIN_FILENO, &ch, 1);
            if (ch == 'c' || ch == 'C') {
                cout << "\nTask completed early!\n";
                setRawMode(false);
                return;
            }
        }
    }

    // If we reach here, a full Pomodoro is completed
    cout << "\nWork session complete. Starting break...\n";
    this_thread::sleep_for(chrono::seconds(BREAK_DURATION));

    // After break, reset remaining time for the next Pomodoro
    remainingTime = WORK_DURATION;
    cout << "Break over! Starting another Pomodoro session...\n";
}








int main() {
    vector<string> orgFiles = {"/home/zaine/master-folder/org_files/master.org"};

    vector<Task> tasks;
    // This loop loads tasks from multiple files and combines them into a single list.
    for (const auto &filePath : orgFiles) {
        vector<Task> fileTasks = loadTasks(filePath);
        tasks.insert(tasks.end(), fileTasks.begin(), fileTasks.end());
    }

    bool continueSession = true;
    while (continueSession && !tasks.empty()) {
        cout << "\n--- TODO Tasks ---\n";
        // Display tasks
        for (size_t i = 0; i < tasks.size(); ++i) {
            cout << i + 1 << ". " << tasks[i].content << endl;
        }

        int taskIndex;
        cout << "\nSelect a task to start Pomodoro (enter task number): ";
        cin >> taskIndex;

        if (taskIndex > 0 && taskIndex <= tasks.size()) {
            Task &selectedTask = tasks[taskIndex - 1];
            cout << "Selected task: " << selectedTask.content << endl;

            // If there’s any remaining time from the last task, carry it over
            if (remainingTime < WORK_DURATION) {
                cout << "Resuming previous Pomodoro with " << remainingTime / 60 << " minutes remaining.\n";
            }

            pomodoroTimer();

            // Mark task as done
            cout << "Marking task as DONE in the file...\n";
            markTaskAsDone(orgFiles[0], selectedTask);
            cout << "Task marked as DONE.\n";

            // Remove the completed task from the list
            tasks.erase(tasks.begin() + (taskIndex - 1));
        } else {
            cout << "Invalid task number. Please try again.\n";
            continue;
        }

        char continueChoice;
        cout << "Do you want to select another task? (y/n): ";
        cin >> continueChoice;
        continueSession = (continueChoice == 'y' || continueChoice == 'Y');
    }

    cout << "Session complete. Exiting...\n";
    return 0;
}