#ifndef EXEC_H
#define EXEC_H

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

#endif