#include "sish.h"

static struct termios orig_termios; // Need to save the settings

void arrowKeyHistory(char *str, char *history[HIST_MAX], int history_i)
{
    write(STDOUT_FILENO, "\r\033[K", 4);

    if(history_i >= 0)
    {
        strcpy(str, history[history_i]);
        write(STDOUT_FILENO, str, strlen(str));
    } else {
        str[0] = '\0';
    }
}

void enableRawMode()
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;

    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int readRawInput()
{
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

void rawReadLine(char *str, int size, char *history[HIST_MAX], int *history_i, int history_count)
{
    enableRawMode();

    int len = 0;

    while(1)
    {
        int c = readRawInput();
        
        if(c == '\n')
        {
            str[len] = '\0';
            write(STDOUT_FILENO, "\n", 1);
            disableRawMode();
            return;
        }

        // Backspace
        if(c == 127 || c == 8)
        {
            if(len > 0)
            {
                len--;
                write(STDOUT_FILENO, "\b \b", 3); // Special characters are 3 bytes
            }
            continue;
        }

        // Escape sequences
        if(c == 27)
        {
            char sequence[2];
            if(read(STDIN_FILENO, &sequence[0], 1) == 0)
            {
                continue;
            }
            if(read(STDIN_FILENO, &sequence[1], 1) == 0)
            {
                continue;
            }
            // Arrow keys
            if(sequence[0] == '[')
            {
                // Place holder for now
                switch(sequence[1])
                {
                    case 'A': 
                        if(*history_i < HIST_MAX-1 && *history_i < history_count-1) (*history_i)++;
                        arrowKeyHistory(str, history, *history_i);
                        // printf("UP"); 
                        break;
                    case 'B': 
                        if(*history_i > -1) (*history_i)--;
                        arrowKeyHistory(str, history, *history_i);
                        len = strlen(str);
                        // printf("DOWN"); 
                        break;
                    case 'C': // Needs limits on how far we can go
                        write(STDOUT_FILENO, "\x1b[C", 3);
                        break;
                    case 'D': 
                        write(STDOUT_FILENO, "\x1b[D", 3); 
                        break;
                }
            }
        }

        if(isprint(c) && len < size - 1)
        {
            str[len++] = c;
            write(STDOUT_FILENO, &c, 1);
        }
    }
}

// What a lazy/hacky way to do this
char* removeQuotes(char *str)
{   
    return strtok(str, "\"");
}

void freeArgs(char *args[ARGS_ROWS][ARGS_COLS])
{
    for(int i = 0; i < ARGS_ROWS; i++)
    {
        if(args[i][0] == NULL)
        {
            return;
        }
        for(int j = 0; j < ARGS_COLS; j++)
        {
            if(args[i][j] == NULL)
            {
                break;
            }
            free(args[i][j]);
            args[i][j] = NULL;
        }
    }
}

// Might want to consider allocating memory to the user input and freeing it here
void freeHistory(char *history[HIST_MAX])
{
    for(int i = 0; i < HIST_MAX; i++)
    {
        if(history[i] != NULL)
        {
            free(history[i]);
            history[i] = NULL;
        }
    }
}

void addHistory(char *str, char *history[HIST_MAX])
{
    if(history[HIST_MAX-1] != NULL)
    {
        free(history[HIST_MAX-1]);
    }

    for(int i = HIST_MAX-2; i >= 0; --i)
    {
        history[i + 1] = history[i];
    }
    history[0] = strdup(str);
}

int builtins(char *args[ARGS_ROWS][ARGS_COLS], char *history[HIST_MAX])
{
    if(strcmp(args[0][0], "exit") == 0)
    {
        freeArgs(args);
        freeHistory(history);
        atexit(disableRawMode);
        exit(0);
    } else if(strcmp(args[0][0], "cd") == 0) {
        if(args[0][1] == NULL)
        {
            char *home = getenv("HOME");
            if(home == NULL)
            {
                fprintf(stderr, "cd: HOME not set\n");
                return 1;
            }
            if(chdir(home) != 0)
            {
                perror("CD failed");
            }
            return 1;
        } else if (chdir(args[0][1]) != 0) {
            perror("CD failed");
        }
        return 1;
    } else if(strcmp(args[0][0], "history") == 0) {
        for(int i = HIST_MAX-1; i >= 0; i--)
        {
            if(history[i] != NULL)
            {
                printf("[%d] %s\n", i+1, history[i]);
            }
        }
        return 1;
    }
    
    return 0;
}

int getBackground(char *args[ARGS_ROWS][ARGS_COLS], int N)
{
    int last = 0;
    while(args[N-1][last] != NULL)
    {
        last++;
    }

    if(last > 0 && strcmp(args[N-1][last-1], "&") == 0)
    {
        args[N-1][last-1] = NULL;
        return 1;
    }
    return 0;
}

void parseRedirect(char *args[ARGS_ROWS][ARGS_COLS], char **input_file, char **output_file, int N)
{
    *input_file = NULL;
    *output_file = NULL;

    for(int i = 0; i < N; ++i)
    {
        for(int j = 0; args[i][j] != NULL; ++j)
        {
            if(strcmp(args[i][j], "<") == 0)
            {
                *input_file = args[i][j + 1];
                args[i][j] = NULL;
            } else if (strcmp(args[i][j], ">") == 0) {
                *output_file = args[i][j + 1];
                args[i][j] = NULL;
            }
        }
    }
}

void executeCommands(char *args[ARGS_ROWS][ARGS_COLS], int background, char *input_file, char *output_file, int N)
{
    int pipes[N-1][2];
    pid_t first_pid = -1;

    for(int i = 0; i < N-1; ++i)
    {
        pipe(pipes[i]);
    }

    for(int i = 0; i < N; ++i)
    {
        pid_t pid = fork();
        if(pid < 0){
            perror("fork failed");
        } else if(pid == 0) {
            if(i > 0)
            {
                dup2(pipes[i-1][0], 0);
            }
            if (i < N-1)
            {
                dup2(pipes[i][1], 1);
            }

            for(int j = 0; j < N-1; ++j)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if(i == 0 && input_file != NULL)
            {
                int inp_open = open(input_file, O_RDONLY);
                if(inp_open < 0)
                {
                    perror("open input");
                    exit(1);
                }
                dup2(inp_open, 0);
                close(inp_open);
            }
            if(i == N-1 && output_file != NULL)
            {
                int out_open = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(out_open < 0)
                {
                    perror("open output");
                    exit(1);
                }
                dup2(out_open, 1);
                close(out_open);
            }

            for(int j = 0; j < N-1; ++j)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(args[i][0], args[i]);
            perror("execvp failed");
            exit(1);
        } else if (i == 0) {
            first_pid = pid;
        }
    }

    // PARENT
    for(int i = 0; i < N-1; ++i)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    if(!background)
    {
        for(int i = 0; i < N; ++i)
        {
        wait(NULL);
        }
    } else {
        printf("[BACKGROUND] Pid: %d\n", first_pid);
    }
}

int tokenize(char *str, char *args[ARGS_ROWS][ARGS_COLS])
{
    int i = 0;
    int rows = 0;
    char *token = strtok(str, " ");

    char combined[COMBINE_AMOUNT];
    int in_quotes = 0;
    combined[0] = '\0';

    while(token != NULL)
    {
        if(in_quotes)
        {
            strcat(combined, " ");
            strcat(combined, token);
            if(token[strlen(token)-1] == '"')
            {
                in_quotes = 0;
                args[rows][i++] = strdup(removeQuotes(combined)); // I need to release this memory
                combined[0] = '\0';
            }
        } else if (token[0] == '"' && token[strlen(token) - 1] != '"') {
            in_quotes = 1;
            strcat(combined, token);
        } else if(strcmp(token, "|") == 0) {
            args[rows++][i] = NULL;
            i = 0;
        } else {
            // Want to make it use strdup so I can just release it later along with the quote stuff
            args[rows][i++] = strdup(token);
        }
        token = strtok(NULL, " ");
    }
    args[rows][i] = NULL;
    return rows + 1;
}

void fixCwd(char cwd[CWD_AMOUNT], char *refine[2])
{
    int i = 0;
    char *token = strtok(cwd, "/");
    char *tokens[1024];

    while(token != NULL)
    {
        tokens[i++] = token;
        token = strtok(NULL, "/");
    }

    refine[0] = tokens[1];
    refine[1] = tokens[i-1];
}

int main()
{
    char input[INPUT_AMOUNT];
    char cwd[CWD_AMOUNT];
    char *args[ARGS_ROWS][ARGS_COLS] = {NULL};
    char *refined_cwd[2];

    char *history[HIST_MAX] = {NULL};
    int history_i = -1;
    int history_count = 0;

    char *input_file = NULL;
    char *output_file = NULL;

    while(1)
    {
        //Setting up cwd to look pretty
        getcwd(cwd, sizeof(cwd));
        fixCwd(cwd, refined_cwd);

        //Read
        printf("\033[1;32m%s@%s\033[0m $ ", refined_cwd[0], refined_cwd[1]);
        fflush(stdout);
        rawReadLine(input, sizeof(input), history, &history_i, history_count); // Testing out canonical mode

        addHistory(input, history);
        if(history_count < HIST_MAX)
        {
            history_count++;
        }

        //tokenize
        //n is the rows of args generated
        int n = tokenize(input, args);

        // exit(1);

        // for (int r = 0; r < n; r++) {
        //     printf("Command %d:\n", r);
        //     for (int c = 0; args[r][c] != NULL; c++) {
        //         printf("  %s\n", args[r][c]);
        //     }
        // }
        // exit(0);

        // If user just presses enter
        if(args[0][0] == NULL)
        {
            continue;
        }

        // Finds if its a built in and skip the forking
        if(builtins(args, history) == 1)
        {
            continue;
        }

        // Get the files after < or > and store them
        parseRedirect(args, &input_file, &output_file, n);

        // If args ends with & then remove and mark as 1
        int background = getBackground(args, n);

        // Main forking function that creates processes
        executeCommands(args, background, input_file, output_file, n);

        input_file = NULL;
        output_file = NULL;
        freeArgs(args);
    }
    atexit(disableRawMode);
    return 0;
}