#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>



char* readall(int fd){
    
    int n = 100, ans = 0, i = 1;

    char buf[n];

    for(int j = 0; j < n; j++){
        buf[j] = '\0';
    }
    size_t len = 0;
    char* str = (char *) malloc(sizeof(char) *  n);

    while(1){
        
        ans = read(fd, &buf, n);
        if(ans == 0){
            //fprintf(stderr, "EOF\n");
            break;
        }
        if(ans == -1){
            fprintf(stderr, "error reading: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        //reallocate memory
        len += ans;
        if(len >= i * n){
            i++;
            str = (char *) realloc(str, sizeof(char) * n * i);
        }

        str = strncat(str, buf, ans);
        
        
    }

    return str;    
}   

char* fetch(){
    pid_t pid;
    int pipefd[2], status;
    char *cmd[] = {"curl", "-s", "http://numbersapi.com/random/math?min=1&max=100&fragment&json", (char*) 0};
    char buf;
    char *json;
    //create a pipe
    if(pipe(pipefd) == -1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    //create a child process
    pid = fork();
    if(pid == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    //child process -> exec curl, write on write end of pipe 
    if(pid == 0){


        if(dup2(pipefd[1], STDOUT_FILENO) == -1){
            perror("dup2\n");
            _exit(EXIT_FAILURE);
        }
        close(pipefd[0]);
        close(pipefd[1]);
        
        if(execvp("curl", cmd) == -1){
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        _exit(EXIT_SUCCESS);
    }
    //parent process -> read from read end of pipe
    else{
        close(pipefd[1]);
        
        //wait for child process
        do{
            pid = waitpid(-1, &status, WUNTRACED);
            if(pid == -1){
                perror("waitpid failure");
                exit(EXIT_FAILURE);
            }
            if(WIFEXITED(status)){
                //fprintf(stderr, "child exited, status: %d\n", WEXITSTATUS(status));
        
            }
        }while(!WIFEXITED(status));
        json = readall(pipefd[0]);
    }
    
    return json;
}

char* getText(char* json){
    //char copyjson[400];
    
    char* copyjson = (char*) malloc(sizeof(char) * (strlen(json)+1));
    strcpy(copyjson, json);
    //char* old_pos = copyjson;

    char* pos = strstr(copyjson, "\"text\":");
    if(pos == NULL) return NULL;
    //put at the beginning of text
    for(int i = 0; i < 9; i++){
        pos++;
    }
    int cnt = 0;
    while(1){
        if(pos[cnt] == '\"'){
            break;
        }
        cnt++;
    }
    char * copypos = (char*) malloc(sizeof(char) * (cnt+1));
    strncpy(copypos, pos, cnt);
    free(copyjson);
    //pos[cnt] = '\0';

    return copypos;
}


long getNumber(char *json){
    
    char* copyjson = (char*) malloc(sizeof(char) * (strlen(json)+1));
    strcpy(copyjson, json);

    char* pos = strstr(copyjson, "\"number\":");
    if(pos == NULL) return -1;
    //put at the beginning of text
    for(int i = 0; i < 10; i++){
        pos++;
    }
    int cnt = 0;
    while(1){
        if(pos[cnt] == ','){
            break;
        }
        cnt++;
    }

    char * copypos = (char*) malloc(sizeof(char) * (cnt+1));
    strncpy(copypos, pos, cnt);
    free(copyjson);
    long num = atol(copypos);
    free(copypos);
    
    return num;

}

unsigned play(unsigned n, unsigned score, char *text, long answer){
    unsigned pts = 8;
    char *buf = NULL;
    ssize_t nread;
    size_t len = 0;

    printf("Q%d: %s?\n", n, text);
    while(pts != 0){
        printf("%u pt>", pts);
        nread = getline(&buf, &len, stdin);
        if(nread == -1){
            perror("getline\n");
            exit(EXIT_FAILURE);
        }
        buf[nread - 1] = '\0';
        long userAns = atol(buf);
        
        if(userAns == answer){
            printf("Congratulations, your answer %ld is correct.\n", userAns);
            break;
        }
        else if(userAns > answer){
            printf("Too large, try again.\n");
        }
        else{
            printf("Too small, try again.\n");
        }
        
        pts /= 2;
    }

    score += pts;
    printf("Your total score is %u/%u points.\n", score, n * 8);

    return score;

}

int main(){
    printf("Answer questions with numbers in the range [1..100].\n");
    printf("You score points for each correctly answered question.\n");
    printf("If you need multiple attempts to answer a question, the\n");
    printf("points you score for a correct answer go down.\n");
    
    for(int i = 1;!feof(stdin); i++){

        char* json = fetch();
        //fprintf(stderr, "json doc: %s\n", json);

        char* text = getText(json);
        //fprintf(stderr, "json text: %s\n", text);

        long answer = getNumber(json);
        //fprintf(stderr, "json num: %ld\n", answer);

        unsigned score = play(i, score, text, answer);

        //fprintf(stderr, "json doc: %s\n", json);
        free(json);
        free(text);
    }
    
    return 0;
}