#ifndef BUILTIN_H
#define BUILTIN_H

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
    } else if(strcmp(args[0][0], "gaa") == 0) {
        pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork failed");
            return 1;
        } else if(pid == 0) {
            char *tokens[] = {"git", "add", ".", NULL};
            execvp("git", tokens);
            
            // If we fail
            perror("execvp failed");
            exit(1);
        }
        wait(NULL);
        return 1;
    } else if(strcmp(args[0][0], "gc") == 0 && args[0][1] != NULL) {
        pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork failed");
            return 1;
        } else if(pid == 0) {
            char *tokens[] = {"git", "commit", "-m", args[0][1], NULL};
            execvp("git", tokens);
            
            // If we fail
            perror("execvp failed");
            exit(1);
        }
        wait(NULL);
        return 1;
    } else if(strcmp(args[0][0], "gp") == 0) {
        pid_t pid = fork();
        if(pid < 0)
        {
            perror("fork failed");
            return 1;
        } else if(pid == 0) {
            char *tokens[] = {"git", "push", NULL};
            execvp("git", tokens);
            
            // If we fail
            perror("execvp failed");
            exit(1);
        }
        wait(NULL);
        return 1;
    }
    // The git shortcuts make me want to make a shortcut built in
    return 0;
}

#endif