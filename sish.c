#include "sish.h"

// Dont want to delete just yet
// void tokenize(char *str, char **args)
// {
//     int i = 0;
//     char *token = strtok(str, " ");
//     while(token != NULL)
//     {
//         args[i++] = token;
//         token = strtok(NULL, " ");
//     }
//     args[i] = NULL;
// }

// Might want to consider allocating memory to the user input and freeing it here
void freeHistory(char *history[10])
{
    for(int i = 0; i < 10; i++)
    {
        if(history[i] != NULL)
        {
            free(history[i]);
            history[i] = NULL;
        }
    }
}

void addHistory(char *str, char *history[10])
{
    if(history[9] != NULL)
    {
        free(history[9]);
    }

    for(int i = 8; i >= 0; --i)
    {
        history[i + 1] = history[i];
    }
    history[0] = strdup(str);
}

void removeNewLine(char *str)
{
    str[strcspn(str, "\n")] = '\0';
}

void readFromUser(char *str, int size)
{
    fgets(str, size, stdin);
    removeNewLine(str);
}

int builtins(char *args[16][64], char *history[10])
{
    if(strcmp(args[0][0], "exit") == 0)
    {
        freeHistory(history);
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
        for(int i = 9; i >= 0; i--)
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

int getBackground(char *args[16][64], int N)
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

void parseRedirect(char *args[16][64], char **input_file, char **output_file, int N)
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

void executeCommands(char *args[16][64], int background, char *input_file, char *output_file, int N)
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

int tokenize(char *str, char *args[16][64])
{
    // go through our str
    // once we see a | we stop
    // once we stop we store every token before into next row of commands
    // continue until the str hits null
    // once null just store the tokens into last commands row


    // for loop
    // if we see | we go into tokenize()?

    int i = 0;
    int rows = 0;
    int token_i = 0;
    char *token = strtok(str, " ");

    while(token != NULL)
    {
        if(strcmp(token, "|") == 0)
        {
            args[rows++][i] = NULL;
            i = 0;
        } else {
            args[rows][i++] = token;
        }
        token = strtok(NULL, " ");

        //if we see a pipe lets move rows
        //if not then just do tokenize normally

    }
    args[rows][i] = NULL;
    return rows + 1;
}

int main()
{
    char input[1024];
    char cwd[1024];
    char *args[16][64];

    char *history[10] = {NULL};

    char *input_file = NULL;
    char *output_file = NULL;

    while(1)
    {
        //Read
        printf("\033[1;32m%s\033[0m $ ", getcwd(cwd, sizeof(cwd)));
        readFromUser(input, sizeof(input));

        addHistory(input, history);

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
        // Set all the args to NULL?

    }
    return 0;
}