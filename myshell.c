#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int prepare(void)
{
    return 0;
}

void run_process(char ** cmd, int flag) {
    pid_t pid = fork();
    int error;
    struct sigaction s;
    s.sa_handler = SIG_IGN;
    struct sigaction new;
    switch (pid) {

        case -1:
            printf("Fork failed");
            exit(EXIT_FAILURE);

        case 0:
            new.sa_handler = SIG_DFL;
            sigaction(SIGINT, &new, NULL);
            error = execvp(cmd[0], cmd);

            if (error == -1) {
                printf("%s : command not found \n", cmd[0]);
                exit(EXIT_FAILURE);
            }

        default:

            sigaction(SIGINT, &s, NULL);

            if (!flag) {
                int status;
                waitpid(pid, &status, 0);
            }

    }
}

void pipe_process(char ** cmd1, char ** cmd2)
{

    int fd[2], stat_val;
    pid_t child,gChild;

    if (pipe(fd) != 0)
        printf("Problem in creating the pipe");

    struct sigaction s;
    s.sa_handler = SIG_IGN;
    struct sigaction new;
    child = fork();
    switch(child)
    {
        case -1:
            perror("Fork Failed\n");
            exit(-1);

        case 0:
            if(!pipe(fd))
            {
                gChild=fork();
                switch(gChild)
                {
                    case -1:
                        fprintf(stderr,"Fork Failed\n");
                        exit(-1);

                    case 0:           
                        new.sa_handler = SIG_DFL;
                        sigaction(SIGINT, &new, NULL);
                        close(1);
                        dup2(fd[1],1);
                        close(fd[1]);
                        close(fd[0]);
                        execvp(cmd1[0],cmd1);
                        printf("Command not found\n");
                        exit(0);
                    default:
                        sigaction(SIGINT, &s, NULL);
                        close(0);
                        if(dup(fd[0]) == -1)
                            fprintf(stderr,"Fork Failed\n");
                        close(fd[0]);
                        close(fd[1]);
                        execvp(cmd2[0],cmd2);
                        printf("Command not found\n");
                        exit(0);
                }
            }
        default:
            waitpid(child,&stat_val,0);
            if(!WIFEXITED(stat_val))
                printf("Child terminated abnormally\n");
    }
}

int process_arglist(int count, char** arglist) {
    int flag = 0, index = 0;
    if (arglist[0] == NULL)
        return 0;

    // Check &
    if (!strcmp(arglist[count - 1], "&")) {
        arglist[count - 1] = NULL;
        run_process(arglist,1);
    }

    else{
        // Check |
        for (int i = 0; i < count - 1; ++i) {
            if(!strcmp(arglist[i], "|"))
            {
                flag = 1;
                index = i + 1;
                arglist[i] = NULL;
                break;
            }
        }
        if(flag)
        {
            pipe_process(arglist, arglist + index);
        }
        else {
            run_process(arglist, 0);

        }
    }

    return 1;

}

int finalize(void)
{
    return 0;
}
