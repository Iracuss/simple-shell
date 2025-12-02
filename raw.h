#ifndef RAW_H
#define RAW_H

static struct termios orig_termios; // Need to save the settings

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

void arrowKeyHistory(char *str, char *history[HIST_MAX], int history_i, char *refined_cwd[2])
{
    write(STDOUT_FILENO, "\r\033[K", 4); // clear

    // Print CWD
    disableRawMode();
    printf("\033[1;32m%s@%s\033[0m $ ", refined_cwd[0], refined_cwd[1]);
    fflush(stdout);
    enableRawMode();

    // Print history
    if(history_i >= 0)
    {
        strcpy(str, history[history_i]);
        write(STDOUT_FILENO, str, strlen(str));
    } else {
        str[0] = '\0';
    }
}

// Broken
void addLetter(char *str, const int cursor, int c, int *len)
{
    for(int i = *len; i < cursor; i--)
    {
        str[i] = str[i-1];
    }
    str[cursor] = c;

    (*len)++;
    str[*len] = '\0';

    write(STDOUT_FILENO, &str[cursor], *len - cursor);
}


void rawReadLine(char *str, int size, char *history[HIST_MAX], int *history_i, int history_count, char *refined_cwd[2])
{
    enableRawMode();
    int cursor = 0;
    int len = 0;

    while(1)
    {
        int c = readRawInput();
        
        if(c == '\n')
        {
            str[len] = '\0';
            *history_i = -1;
            write(STDOUT_FILENO, "\n", 1);
            disableRawMode();
            return;
        }

        // Backspace
        if(c == 127 || c == 8)
        {
            if(len > 0 && cursor > 0)
            {
                if(cursor != len)
                {
                    
                } else {
                    len--;
                    cursor--;
                    write(STDOUT_FILENO, "\b \b", 3); // Special characters are 3 bytes
                }
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
                        arrowKeyHistory(str, history, *history_i, refined_cwd);
                        len = strlen(str);
                        cursor = strlen(str);
                        // printf("UP"); 
                        break;
                    case 'B': 
                        if(*history_i > -1) (*history_i)--;
                        arrowKeyHistory(str, history, *history_i, refined_cwd);
                        len = strlen(str);
                        cursor = strlen(str);
                        // printf("DOWN"); 
                        break;
                    case 'C': // Needs limits on how far we can go
                        if(cursor == len)
                        {
                            break;
                        }
                        write(STDOUT_FILENO, "\x1b[C", 3); // Right
                        cursor++;
                        break;
                    case 'D': 
                        if(cursor == 0)
                        {
                            break;
                        }
                        write(STDOUT_FILENO, "\x1b[D", 3); // Left
                        cursor--;
                        break;
                }
            }
        }

        if(isprint(c) && len < size - 1)
        {
            //We need a way to take the string before then add a letter then concat everything after
            if(cursor != len)
            {
                str[cursor++] = c;
                write(STDOUT_FILENO, &c, 1);

                // addLetter(str, cursor, c, &len);
                // // len++;
                // cursor++;
            } else {
                str[len++] = c;
                cursor++;
                write(STDOUT_FILENO, &c, 1);
            }
        }
    }
}
#endif