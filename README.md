# sish - Custom C Shell

## Overview
**sish** is a lightweight, custom Unix shell implemented in C. It provides standard shell functionalities such as command execution, piping, I/O redirection, and background processing, alongside a custom raw terminal mode for history navigation and custom built-in Git commands.

---

## Features

* **Custom Prompt:** Displays a colorized prompt showing the user and current directory (`username@directory $ `).
* **Raw Input Mode:** Utilizes `termios` to enable a raw terminal mode, allowing for character-by-character input reading and escape sequence parsing.
* **Command History:** Supports tracking up to 10 previous commands using the `HIST_MAX` limit.
* **Arrow Key Navigation:** Users can navigate their command history using the UP and DOWN arrow keys.
* **Piping:** Supports piping output between multiple commands in a chain.
* **I/O Redirection:** Supports redirecting input from files using `<` and redirecting output to files using `>`.
* **Background Execution:** Supports running commands in the background by appending `&` to the end of the argument list.

---

## Built-In Commands

The shell includes several built-in commands that bypass standard process forking to execute directly within the main process:

| Command | Description |
| :--- | :--- |
| `exit` | Exits the shell, frees allocated arguments and history, and safely disables raw mode. |
| `cd [dir]` | Changes the current working directory, defaulting to the `HOME` environment variable if no directory is specified. |
| `history` | Displays the command history up to the maximum limit. |
| `gaa` | A shortcut built-in that executes `git add .`. |
| `gc [msg]` | A shortcut built-in that executes `git commit -m "msg"`. |
| `gp` | A shortcut built-in that executes `git push`. |

---

## Project Structure

* **`main.c`**: Contains the main execution loop, formats the current working directory, handles tokenization, and integrates all components.
* **`sish.h`**: Stores the required standard library includes and global macros (e.g., buffer sizes and history limits).
* **`raw.h`**: Manages the terminal's raw mode configuration, reading raw input, and handling arrow key sequences and backspaces.
* **`history.h`**: Provides functions to add strings to the history array and free history memory.
* **`builtin.h`**: Contains the logic and process handling for built-in shell commands and Git shortcuts.
* **`execute.h`**: Handles process forking, pipe creation, configuring file descriptors for redirection, and executing binaries via `execvp`.
* **`redirect.h`**: Parses command arguments to identify and extract input (`<`) and output (`>`) redirection targets.
* **`background.h`**: Detects if a command should be run in the background by checking for the `&` token at the end of the arguments.

## How To Build
```bash
gcc sish.c -o sish
./sish
```