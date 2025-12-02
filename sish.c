#include "sish.h"
#include "raw.h"
#include "tokenize.h"
#include "history.h"
#include "background.h"
#include "redirect.h"
#include "builtin.h"
#include "execute.h"

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
        rawReadLine(input, sizeof(input), history, &history_i, history_count, refined_cwd); // Testing out canonical mode

        addHistory(input, history);
        if(history_count < HIST_MAX)
        {
            history_count++;
        }

        //tokenize
        //n is the rows of args generated
        int n = tokenize(input, args);

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

        // If args ends with & then remove and mark background as 1
        int background = getBackground(args, n);

        // Main forking function that creates processes
        executeCommands(args, background, input_file, output_file, n);

        // Free anything needed in next loop
        input[0] = '\0';
        input_file = NULL;
        output_file = NULL;
        freeArgs(args);
    }
    atexit(disableRawMode);
    return 0;
}