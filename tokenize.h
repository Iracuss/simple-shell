#ifndef TOKEN_H
#define TOKEN_H


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

// What a lazy/hacky way to do this
char* removeQuotes(char *str)
{   
    return strtok(str, "\"");
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

#endif