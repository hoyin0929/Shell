/****************************************************************
 * Name        :                                                *
 * Class       :  CSC 415                                       *
 * Date        :                                                *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

/* CANNOT BE CHANGED */
#define BUFFERSIZE 256
/* --------------------*/
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)

#define ARGVMAX 64
#define PIPECNTMAX 10
#define NAME "Ho Yin Mak"
#define DELIMITER " \t\r\n\a"


void shell_start()
{
    printf("Welcome to myShell by %s\n", NAME);

    char* userName = getenv("USER");
    printf("User: %s\n", userName);
}

void shell_directory()
{
    char cwd[BUFFERSIZE], new_cwd[BUFFERSIZE];
    getcwd(cwd, sizeof(cwd));
    char* homedir = getenv("HOME");
    strncpy(new_cwd, &cwd[strlen(homedir)+1], strlen(cwd)+1);
    printf("%s~/%s >> ", PROMPT, new_cwd);
}

char* shell_read()
{
    int line = BUFFERSIZE;
    int pos = 0;

    char* buffer = malloc(sizeof(char) * line);
    int character;

    if(!line)
    {
        perror("Failed to allocate");
        exit(0);
    }

    while(1)
    {
        character = getchar();

        if(character == EOF || character == '\n')
        {
            buffer[pos] = '\0';
            return buffer;
        }
        else
        {
            buffer[pos] = character;
        }
        pos++;
    }
}

char** shell_split(char* line)
{
    int bufferSize = ARGVMAX;
    int pos = 0;
    char* token;
    char** tokens = malloc(bufferSize * sizeof(char*));

    if(!tokens)
    {
        perror("Failed to allocate");
    }

    token = strtok(line, DELIMITER);

    while(token != NULL)
    {
        tokens[pos] = token;
        pos++;
        token = strtok(NULL, DELIMITER);
    }
    tokens[pos] = NULL;

    return tokens;
}

enum Pipe_Redirect {PIPE, REDIRECT, NPNR};

enum Pipe_Redirect parse_cmd(int argc, char** argv, char** cmd1, char** cmd2){
    enum Pipe_Redirect state = NPNR;

    int split = -1;

    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "|") == 0)
        {
            state = PIPE;
            split = i;
            for(i = 0; i < split; i++)
            {
                cmd1[i] = argv[i];
            }
        }
        else if(strcmp(argv[i], ">>") == 0)
        {
            state = REDIRECT;
            split = i;
            for(int i = 0; i < split; i++)
            {
                cmd2[i] = argv[i];
            }
        }
        else if(strcmp(argv[i], "<<") == 0)
        {
            state = REDIRECT;
            split = i;
            for(int i = 0; i < split; i++)
            {
                cmd2[i] = argv[i];
            }
        }
        else if(strcmp(argv[i], ">") == 0)
        {
            state = REDIRECT;
            split = i;
            for(int i = 0; i < split; i++)
            {
                cmd2[i] = argv[i];
            }
        }
        else if(strcmp(argv[i], "<") == 0)
        {
            state = REDIRECT;
            split = i;
            for(int i = 0; i < split; i++)
            {
                cmd2[i] = argv[i];
            }
        }
    }
    if(state == PIPE)
    {
        for(int i = split; i < argc; i++)
        {
            cmd1[i] = argv[i];
        }
    }
    else if(state == REDIRECT)
    {
        for(int i = split; i < argc; i++)
        {
            cmd2[i] = argv[i];
        }

        cmd2[argc] = NULL;
        cmd1[argc] = NULL;
    }
    return state;
}

void pipe_cmd(char* args[], int argc)
{
    char* cmd1[ARGVMAX];
    char* cmd2[ARGVMAX];
    int file_des[2];
    pipe(file_des);
    pid_t pid;
    int flag = 0;
    int count = 0;

    for(int i = 0; i < argc; i++)
    {
        if(strcmp(args[i], "|") == 0)
        {
            flag = i;
        }
    }
    for(int i = 0; i < flag; i++)
    {
        cmd1[i] = args[i];
    }
    for(int i = flag+1; i < argc; i++)
    {
        cmd2[count] = args[i];
        count++;
    }

    cmd1[flag] = NULL;
    cmd2[count] = NULL;

    if(fork() == 0)
    {
        dup2(file_des[0], 0);
        close(file_des[1]);
        execvp(cmd2[0], cmd2);
        perror("Execution Error");
    }
    else if((pid = fork() == 0))
    {
        dup2(file_des[1], 1);
        close(file_des[0]);
        execvp(cmd1[0], cmd1);
        perror("Execution Error");
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}

void redirect_cmd(char* args[], int argc)
{
    int fd;
    int file_des[2];
    pipe(file_des);
    pid_t pid;
    int flag = 0;
    int count = 0;
    long filecount;
    char byte;

    char* cmd[] = {"echo", NULL};

    char* input[ARGVMAX];
    char* output[ARGVMAX];

    for(int i = 0 ; i < argc ; i ++)
    {
        if(strcmp(args[i], ">>") == 0)
        {
            flag = i;
        }
        else if(strcmp(args[i], "<<") == 0)
        {
            flag = i;
        }
        else if(strcmp(args[i], ">") == 0)
        {
            flag = i;
        }
        else if(strcmp(args[i], "<") == 0)
        {
            flag = i;
        }

    }

    if(strcmp(args[flag], ">")==0 || strcmp(args[flag], ">>")==0)
    {
        for(int i = 0; i < flag; i++)
        {
            input[i] = args[i];
        }
        for(int i = flag+1; i < argc; i++)
        {
            output[count] = args[i];
            count++;
        }
        input[flag] = NULL;
        output[count] = NULL;
    }
    else if(strcmp(args[flag], "<") == 0 || strcmp(args[flag], "<<") == 0)
    {
        for(int i = 0; i < flag; i++)
        {
            output[i] = args[i];
        }
        for(int i = flag+1; i < argc; i++)
        {
            input[count] = args[i];
            count++;
        }

        output[flag] = NULL;
        input[count] = NULL;
    }


    if(strcmp(args[flag], ">") == 0)
    {
        if(fork() == 0)
        {
            fd = open(output[0] , O_RDWR | O_CREAT, 0644);

            if(fd < 0)
            {
                printf("Failed to open file %s\n", output[0]);
            }

            dup2(file_des[0], 0);
            close(file_des[1]);

            while((filecount = read(0, &byte, 1)) > 0)
            {
                write(fd, &byte, 1);
            }
            close(fd);

            execvp(cmd[0], cmd);
        }
        else if((pid = fork()) == 0)
        {
            dup2(file_des[1],1);
            close(file_des[0]);
            execvp(input[0] , input);
            perror("Execution Error");
        }
        else
        {
            waitpid(pid, NULL, 0 );
        }

    }

    if(strcmp(args[flag], ">>") == 0)
    {
        if(fork() ==0)
        {
            fd = open(output[0] , O_RDWR | O_CREAT | O_APPEND, 0644);

            if(fd < 0)
            {
                printf("Failed to open file %s\n", output[0]);
            }

            dup2(file_des[0], 0);
            close(file_des[1]);
            while((filecount = read(0, &byte, 1)) > 0 )
            {
                write(fd, &byte, 1);
            }

            execvp(cmd[0], cmd);
        }
        else if ((pid = fork()) == 0)
        {
            dup2(file_des[1],1);
            close(file_des[0]);
            execvp(input[0] , input);
            perror("Execution Error");
        }
        else
        {
            waitpid(pid, NULL, 0 );
        }
    }

    if(strcmp(args[flag], "<") == 0)
    {
        if(fork() == 0)
        {
            fd = open(input[0] , O_RDONLY, 0);
            dup2(fd, 0);
            close(fd);
            execvp(output[0], output);
        }
        else if((pid = fork()) == 0)
        {
            dup2(file_des[1],1);
            close(file_des[0]);
            execvp(cmd[0], cmd);
        }
        else
        {
            waitpid(pid, NULL, 0);
        }
    }
}

int arg_count(char** argv)
{
    int count = 0;
    while(*argv != NULL)
    {
        count++;
        argv++;
    }
    return count;
}

void builtin_cmd(int argc, char** argv)
{
    char cwd[BUFFERSIZE];
    char* list_builtin[] = {"exit", "cd", "pwd"};
    size_t num_builtin = sizeof(list_builtin)/sizeof (char*);
    int args = 0;
    pid_t pid;
    const char* amp = "&";
    int get_amp = -1;

    for (int i = 0; i < num_builtin; i++)
    {
        if (strcmp(argv[0], list_builtin[i]) == 0)
        {
            args = i + 1;
            break;
        }
        args = 4;
    }

    switch(args)
    {
        case 1:
            printf("Exiting myShell..\n");
            sleep(1);
            exit(0);
            break;

        case 2:
            if(argv[1] == NULL)
            {
                chdir(getenv("HOME"));
            }
            if(chdir(argv[1]) == -1)
            {
                printf("%s: no such directory\n", argv[1]);
            }
            else
            {
                chdir(argv[0]);
            }
            break;

        case 3:
            getcwd(cwd, sizeof(cwd));
            printf("%s\n", cwd);
            break;

        case 4:
            if(strcmp(argv[argc-1], amp) == 0)
            {
                get_amp = 1;
            }
            pid = fork();

            if(pid < 0)
            {
                perror("Fail to fork");
            }
            else if(pid == 0)
            {
                if(get_amp == 1)
                {
                    argv[argc-1] = NULL;
                    argc--;
                }
                execvp(argv[0], argv);

                if(execvp(argv[0], argv) == -1)
                {
                    perror("Execution Error");
                }
            }

            else if(get_amp == -1)
            {
                waitpid(pid, NULL, 0);
            }
            break;
        default:
            break;
    }
}

void shell_loop()
{
    char* line;
    char** argv;
    int argc;
    enum Pipe_Redirect shell_cmd;

    char* cmd1[ARGVMAX];
    char* cmd2[ARGVMAX];

    do
    {
        shell_directory();
        line = shell_read();
        argv = shell_split(line);
        argc = arg_count(argv);

        shell_cmd = parse_cmd(argc, argv, cmd1, cmd2);

        if(shell_cmd == PIPE)
        {
            pipe_cmd(cmd1, argc);
        }
        else if(shell_cmd == REDIRECT)
        {
            redirect_cmd(cmd2, argc);
        }
        else
        {
            builtin_cmd(argc, argv);
        }

        free(line);
        free(argv);
    }while(1);
}

int main(int argc, char** argv)
{
    shell_start();
    shell_loop();

    return 0;
}
