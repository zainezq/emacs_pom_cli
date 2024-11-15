# TASK HANDLER WITH EMACS SUPPORT
This is a simple C++ CLI application that reads tasks from `.org` files, allows the user to select tasks or add new ones, and starts a Pomodoro timer for focused work. Once a task is completed, it updates the task status from `TODO` to `DONE` directly within the `.org` file.

## Features
- Load tasks from multiple `.org` files.
- List available `TODO` tasks from the file(s).
- Select a task to start a Pomodoro timer.
- Add new tasks, which will be appended to the end of the org file.
- Pomodoro timer runs for 25 minutes with a 5-minute break.
- Mark tasks as `DONE` in the `.org` file once completed.

## How It Works
1. **Load Tasks**: The program loads all tasks marked with `* TODO` from specified `.org` files.
2. **Pomodoro Timer**: When a task is selected, a 25-minute timer begins. 
3. **Task Completion**: Press `c` during the Pomodoro timer to mark the task as complete.
4. **File Update**: Once a task is marked as complete, the application updates the `.org` file, changing `TODO` to `DONE`.

## Usage

### Prerequisites
- Linux-based system with a C++ compiler.
- Change the org file path located at the top of the todo_pom.cpp file (line 26)

### Compile and Run
1. Clone the repository or download the code.
2. Compile the code using `g++`:
   ```bash
   g++ -o todo_pomodoro todo_pom.cpp
3. Run the application:
   ```bash
   ./todo_pomodoro

