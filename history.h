#ifndef HIST_H
#define HIST_H

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

#endif