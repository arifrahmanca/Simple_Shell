#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

/*
Splits command string into tokens separated by " " , "\n" or "\t"
Tokens are stored in arg
The function returns the position of last argument, or -1 if there's no command
*/
int arguments(char* command,char** arg)
{
        arg[0]=strtok(command," \n\t"); //ignore " " and "\n"
        if (arg[0]==NULL){ // if no arguments
              return -1; //no command
        }
        int i;
        for (i=1;i<5; i++){
            arg[i]=strtok(NULL," \n\t"); // next token from command until returns NULL
            if (arg[i]==NULL)
              break;
        }
        return (i-1); // pos of last arg
}


/*
If arg has "&" ,delete "&" and return 1
Else return 0
*/
int ampersand(char** arg)
{
    if (strcmp((*arg),"&")==0) // "&" token
    {
        (*arg)=NULL;
        return 1;
    }
    else if (((*arg)[strlen(*arg)-1])=='&') // if token has '&' as last character
    {
        ((*arg)[strlen(*arg)-1])='\0'; //last character
        return 1;
    }
    return 0;
}


int main()
{
    char command[100]; // Command (100 characters max)

    char directory[100]; //Current directory
   // chdir("/home"); // Default directory
    getcwd(directory,sizeof(directory));
    int index=0;
    int boolVar=0;
    while(1){
        if(boolVar==0){
            printf("mysh:~$ "); //  PRINT MySHell: "Directory"$
            fgets(command,sizeof(command),stdin); //new command
        }else{
            boolVar=0;printf("\n");
        }

        if (strchr(command,'|')==NULL){
            char* arg[6]; // Maximum arguments 5 (arg[5]=NULL)
            arg[5]=NULL;
            int last_arg;
            last_arg=arguments(command,arg);
        //  ----------  COMMAND EXIT   ----------
            if (strcmp(arg[0],"exit")==0){
                exit(EXIT_SUCCESS);
            }

            //  ----------  FORK PROCESS   ----------
            else{
                int background;
                background=ampersand(&(arg[last_arg])); //reference for edit

                //   FORK
                pid_t id=fork();
                if (id==0){            // child process
                    int exe_status;
                    exe_status=execvp(arg[0],arg); // arg[0] -> file path name or name (PATH Variable)
                    if (exe_status==-1){
                        printf("%s: command not found\n",arg[0]);
                    }
                    exit(EXIT_SUCCESS);
                }
                else if (id>0){        // parent process
                    int child_status;

                    signal(SIGCHLD,SIG_IGN);

                    if (!background)   // not background child process
                    {
                        wait(&child_status);
                    }
                }
                else{                   // fork error
                    exit(EXIT_FAILURE);
                }
            }
        }
  //******************     PIPE     ************************
        else{
            char* command_tok[2];

            command_tok[0]=strtok(command,"|"); // left commnad
            command_tok[1]=strtok(NULL,"|");   // right command

            if (command_tok[0]==NULL || command_tok[1]==NULL)
            {
                printf("Syntax error\n");
                continue;   // new command
            }

            char* arg_tok[2][6];
            arg_tok[0][5]=NULL;
            arg_tok[1][5]=NULL;
            int last_arg_tok[2];

            last_arg_tok[0]=arguments(command_tok[0],arg_tok[0]);
            last_arg_tok[1]=arguments(command_tok[1],arg_tok[1]);

            if (last_arg_tok[0]==-1 || last_arg_tok[1]==-1) // no left or right command
            {
                printf("Syntax error\n");
                continue;
            }

            int background;
            background=ampersand(&(arg_tok[1][last_arg_tok[1]])); // right command's last token

            int fd[2]; // file descriptors
            pipe(fd);

            pid_t id[2];
            int i;
            for (i=0;i<2;i++)
            {
                id[i]=fork();

                if (id[i]==0) //  child process
                {
                    if (i==0) //left process
                    {
                        close(fd[0]);
                        dup2(fd[1],1);
                    }
                    else   // right process
                    {
                        close(fd[1]);
                        dup2(fd[0],0);
                    }
                    int exe_status;
                    exe_status=execvp(arg_tok[i][0],arg_tok[i]); // arg[0] -> file path name or name (PATH Variable)
                    if (exe_status==-1){
                        printf("%s: command not found\n",arg_tok[i][0]);
                    }
                    exit(EXIT_SUCCESS);
                }
                else if (id[i]>0)  // parent process
                {
                    if (i==0)
                      continue;   // fork right child

                    // Children have been executed
                    close(fd[0]);
                    close(fd[1]);

                    signal(SIGCHLD,SIG_IGN);

                    int child_status;

                    if (!background)   // not background child process
                    {
                        wait(&child_status);
                    }
                }
                else
                {                   // fork error
                    exit(EXIT_FAILURE);
                }
            }


        }

    }
    exit(EXIT_SUCCESS);
}