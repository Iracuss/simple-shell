#ifndef REDIRECT_H
#define REDIRECT_H

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

#endif