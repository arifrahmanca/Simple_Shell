#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_LINE		80

char* history[10][MAX_LINE/2 + 1];
int hisIndex[10];
int buffIndex = 0;
void create_history(void);
void remove_history(void);
void output_history(void);
char** get_history(char** args, int *interval);

int main(void)
{
	char *args[MAX_LINE/2 + 1];
    int should_run = 1;

    create_history();
    while (should_run){
        printf("mysh:$ ");
        fflush(stdout);

        pid_t pid;

        char line[MAX_LINE + 1];
        char *ch = line;
        int i = 0;
        if(scanf("%[^\n]%*1[\n]", line) < 1) {
            if(scanf("%1[\n]", line) < 1) {
                printf("Failure of standard input.\n");
                return 1;
            }
        }

        while(*ch ==' ' || *ch == '\t')
            ch++;
        while(*ch != '\0'){
            char *temp = (char*)malloc((MAX_LINE + 1)*sizeof(char));
            args[i] = (char*)malloc((MAX_LINE + 1)*sizeof(char));
            int j = sscanf(ch, "%[^ \t]", args[i]);
            ch += strlen(args[i]);
            if(j < 1){
                printf("Command is invalid.\n");
                return 1;
            }
            j = sscanf(ch, "%[ \t]", temp);
            if(j > 0)
                ch += strlen(temp);
            i++;
            free(temp);
        }
        int waitTime = 1;
        if(strlen(args[i - 1]) == 1 && args[i - 1][0] == '&') {
            waitTime = 0;
            free(args[i - 1]);
            args[i - 1] = NULL;
        } else {
            args[i] = NULL;
        }
        if(strcmp(args[0],"exit") == 0){
            remove_history();
            return 0;
        }

        if(args[1] == NULL && strcmp(args[0],"history") == 0) {
            output_history();
        }
        char **ch2 = get_history(args, &waitTime);

        pid = fork();
        if(pid < 0) {
            printf("Fork creation failed.\n");
            return 1;
        } else if(pid == 0) {
            execvp(ch2[0], ch2);
            return 1;
        } else {
            if(waitTime) {
                while(wait(NULL) != pid);
            }
            else {
                printf("%d\n", pid);
            }
        }
    }

	return 0;
}
char** get_history(char **args, int *interval) {
    int i;
    if(args[1] == NULL && strcmp(args[0],"!!") == 0) {
        if(buffIndex > 0){
            strcpy(args[0], history[(buffIndex - 1)%10][0]);
            for(i = 1; history[(buffIndex - 1)%10][i] != NULL; i++) {
                args[i] = (char*)malloc((MAX_LINE + 1)*sizeof(char));
                strcpy(args[i], history[(buffIndex - 1)%10][i]);
            }
            args[i] = NULL;
            *interval = hisIndex[(buffIndex - 1)%10];
        } else {
            printf("History is blank.\n");
            return args;
        }
    }   

    for(i = 0; i < (MAX_LINE/2+1) && history[buffIndex%10][i] != NULL; i++)
        free(history[buffIndex%10][i]);
    for(i = 0; args[i] != NULL; i++) {
        history[buffIndex%10][i] = args[i];
    }
    history[buffIndex%10][i] = args[i];
    hisIndex[buffIndex%10] = *interval;
    return history[(buffIndex++)%10];
}
void create_history(void) {
    int i,j;
    for(i = 0; i < 10; i++) {
        for(j = 0; j < (MAX_LINE/2 + 1); j++) {
            history[i][j] = NULL;
        }
        hisIndex[i] = 0;
    }
}
void remove_history(void) {
    int i,j;
    for(i = 0; i < 10 && i < buffIndex; i++) {
        for(j = 0; history[i][j] != NULL; j++) {
                free(history[i][j]);
        }
    }
}
void output_history(void) {
    int i,j;
    for(i = 0; i < 10 && i < buffIndex; i++) {
        int buffIndex;
        if(buffIndex > 10)
            buffIndex = buffIndex - 9 + i;
        else
            buffIndex = i + 1;
        printf("[%d] ", buffIndex);
        for(j = 0; history[(buffIndex - 1)%10][j] != NULL; j++) {
            printf("%s ", history[(buffIndex - 1)%10][j]);
        }
        if(hisIndex[(buffIndex - 1)%10] == 0) {
            printf("&");
        }
        printf("\n");
    }
}
