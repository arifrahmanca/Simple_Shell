#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>


int arguments(char* command,char** arg)
{
        arg[0]=strtok(command," \n\t"); 
        if (arg[0]==NULL){ 
              return -1; 
        }
        int i;
        for (i=1;i<5; i++){
            arg[i]=strtok(NULL," \n\t"); 
            if (arg[i]==NULL)
              break;
        }
        return (i-1); 
}


int ampersand(char** arg)
{
    if (strcmp((*arg),"&")==0) 
    {
        (*arg)=NULL;
        return 1;
    }
    else if (((*arg)[strlen(*arg)-1])=='&') 
    {
        ((*arg)[strlen(*arg)-1])='\0'; 
        return 1;
    }
    return 0;
}


int main()
{
    char command[100]; 

    char directory[100]; 
    chdir("/home"); 
    getcwd(directory,sizeof(directory));
    int index=0;
    int boolVar=0;
    while(1){
        if(boolVar==0){
            printf("mysh:~%s$ ",directory); 
            fgets(command,sizeof(command),stdin); 
        }else{
            boolVar=0;printf("\n");
        }

        if (strchr(command,'|')==NULL){
            char* arg[6]; 
            arg[5]=NULL;
            int last_arg;
            last_arg=arguments(command,arg);
        //  ----------  COMMAND EXIT   ----------
            if (strcmp(arg[0],"exit")==0){
                exit(EXIT_SUCCESS);
            }

        //  ----------   COMMAND CD    ----------
            else if (strcmp(arg[0],"cd")==0){
                if (arg[1]==NULL || strcmp(arg[1],"~")==0){ 
                    chdir("/home");  
                }else{
                    strcat(directory,"/");
                    strcat(directory,arg[1]); 
                    chdir(directory); 
                }
                getcwd(directory,sizeof(directory)); 
            }
            //  ----------  FORK PROCESS   ----------
            else{
                int background;
                background=ampersand(&(arg[last_arg])); 

                //   FORK
                pid_t id=fork();
                if (id==0){            
                    int exe_status;
                    exe_status=execvp(arg[0],arg); 
                    if (exe_status==-1){
                        printf("%s: command not found\n",arg[0]);
                    }
                    exit(EXIT_SUCCESS);
                }
                else if (id>0){        
                    int child_status;

                    signal(SIGCHLD,SIG_IGN);

                    if (!background)   
                    {
                        wait(&child_status);
                    }
                }
                else{                   
                    exit(EXIT_FAILURE);
                }
            }
        }
  //******************     PIPE     ************************
        else{
            char* command_tok[2];

            command_tok[0]=strtok(command,"|"); 
            command_tok[1]=strtok(NULL,"|");   

            if (command_tok[0]==NULL || command_tok[1]==NULL)
            {
                printf("Syntax error\n");
                continue;   
            }

            char* arg_tok[2][6];
            arg_tok[0][5]=NULL;
            arg_tok[1][5]=NULL;
            int last_arg_tok[2];

            last_arg_tok[0]=arguments(command_tok[0],arg_tok[0]);
            last_arg_tok[1]=arguments(command_tok[1],arg_tok[1]);

            if (last_arg_tok[0]==-1 || last_arg_tok[1]==-1) 
            {
                printf("Syntax error\n");
                continue;
            }

            int background;
            background=ampersand(&(arg_tok[1][last_arg_tok[1]])); 

            int fd[2]; 
            pipe(fd);

            pid_t id[2];
            int i;
            for (i=0;i<2;i++)
            {
                id[i]=fork();

                if (id[i]==0) 
                {
                    if (i==0) 
                    {
                        close(fd[0]);
                        dup2(fd[1],1);
                    }
                    else   
                    {
                        close(fd[1]);
                        dup2(fd[0],0);
                    }
                    int exe_status;
                    exe_status=execvp(arg_tok[i][0],arg_tok[i]); 
                    if (exe_status==-1){
                        printf("%s: command not found\n",arg_tok[i][0]);
                    }
                    exit(EXIT_SUCCESS);
                }
                else if (id[i]>0)  
                {
                    if (i==0)
                      continue;  

                    close(fd[0]);
                    close(fd[1]);

                    signal(SIGCHLD,SIG_IGN);

                    int child_status;

                    if (!background)  
                    {
                        wait(&child_status);
                    }
                }
                else
                {                   
                    exit(EXIT_FAILURE);
                }
            }


        }

    }
    exit(EXIT_SUCCESS);
}
