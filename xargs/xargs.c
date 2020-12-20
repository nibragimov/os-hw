#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    
    //xargs [options] [command [initial-arguments]]
    int opt = 0, num = 0, myargi = 0, status;

    int tflag = 0;

    char* line = NULL, *myargv[100];

    char* echo = "/bin/echo";
   
    size_t len = 100;

    pid_t pid, wpid;
    //parse options
    while(1){
        if((opt = getopt(argc, argv, "tn:")) == -1){
            fprintf(stderr, "Error with getopt or EOF: %s\n", strerror(errno));
            break;
        }
        switch (opt){
            case 'n':
                num = atoi(optarg);
                if(num < 0){
                    fprintf(stderr, "Invalid -n arg\n");
                    exit(EXIT_FAILURE);
                }
                fprintf(stderr, "num = %d\n", num);
                break;
            case 't':
                tflag = 1;
                break;
            case '?':
                fprintf(stderr, "Valid options: -n and -t\n");
                exit(EXIT_FAILURE);
                break;
        }
    }
    //if no args, have "/bin/echo" for command
    if(optind == argc){
        myargv[0] = echo;
    }
    //parse additional args and append to command strings   
    for(int i = optind; i < argc; i++){
        fprintf(stderr, "Arg: %s\n", argv[i]);
        myargv[i - optind] = argv[i];
        fprintf(stderr, "myargv: %s\n", myargv[i - optind]);
        myargi = i - optind;
    }
    //read lines and construct arg lists
    int ind = myargi;
    while(getline(&line, &len, stdin) > 0){
        ind++;
        //remove newline
        line[strcspn(line, "\n")] = 0;

        myargv[ind] = (char *) malloc(sizeof(char) * strlen(line));
        strcpy(myargv[ind], line);
        //debugging
        //fprintf(stderr, "myargv: %s\n", myargv[ind]);

        //-n option with arg
        if(num > 0){
            
            for(int i = 0; i < num - 1; i++){
                if(getline(&line, &len, stdin) == -1){
                    break;
                }
                line[strcspn(line, "\n")] = 0;
                ind++;
                myargv[ind] = (char *) malloc(sizeof(char) * strlen(line));
                strcpy(myargv[ind], line);

                //fprintf(stderr, "myargv: %s\n", myargv[ind]);
                
            }
        }
        //no -n option
        else{
            while(getline(&line, &len, stdin) > 0){
                line[strcspn(line, "\n")] = 0;
                ind++;
                myargv[ind] = (char *) malloc(sizeof(char) * strlen(line));
                strcpy(myargv[ind], line);

                //fprintf(stderr, "myargv: %s\n", myargv[ind]);
            }
        }
        

        // -t option -> print the command before execution
        if(tflag){
            for(int i = 0; i <= ind; i++){
                printf("%s ", myargv[i]);
            }
            printf("\n");
        }
        myargv[ind + 1] = NULL;
        //create a child process and exec the command
        pid = fork();
        if(pid == -1){
            fprintf(stderr, "error in fork: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            fprintf(stderr, "fork success\n");
            execvp(myargv[0], myargv);
            
            fprintf(stderr, "error in exec\n");
            _exit(EXIT_FAILURE);
            
        }
        //wait for child process until it does its job
        do{
            wpid = waitpid(-1, &status, WUNTRACED);
            if(wpid == -1){
                perror("waitpid failure");
                exit(EXIT_FAILURE);
            }
            if(WIFEXITED(status)){
                fprintf(stderr, "child exited, status: %d\n", WEXITSTATUS(status));
        
            }
        }while(!WIFEXITED(status));

        //deallocate memory 
        for(int i = myargi + 1; i <= ind; i++){
            //fprintf(stderr, "%s ", myargv[i]);
            free(myargv[i]);
        
        }
        ind = myargi;
        
    }





    return 0;
}