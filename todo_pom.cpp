#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;

// This will be the Task Class to represent each to-do item

class Task
{
public:
    string name;
    string description;
    int priority;
    string status;
    int pomCount;

    Task(string n, string d, int p)
        : name(n), description(d), priority(p), status("Pending"), pomCount(0) {}

    void markComplete()
    {
        status = "Complete";
    }

    void incrementPom()
    {
        pomCount++;
    }

    void displayTask() const
    {
        cout << "Task: " << name << "\nDescription: " << description
             << "\nPriority: " << priority << "\nStatus: " << status
             << "\nPomodoros: " << pomCount << "\n";
    }
};

// The pomodoro timer class

class PomodoroTimer
{
public:
    int workDuration;
    int breakDuration;

    PomodoroTimer(int work = 25, int brk = 5) : workDuration(work), breakDuration(brk) {}

    void startSession(Task &task)
    {
        cout << "Starting pomodoro for Task: " << task.name << endl;
        timer(workDuration, "Work session done! break time");
        task.incrementPom();
        startBreak();
    }

    void startBreak()
    {
        cout << "Starting break ..." << endl;
        timer(breakDuration, "Break over! back to work");
    }

private:
    void timer(int duration, const string &message)
    {
        this_thread::sleep_for(chrono::minutes(duration));
        cout << message << endl;
    }
};

int main()
{
    PomodoroTimer pomodoroTimer;
    pomodoroTimer.startBreak();
    return 0;
}