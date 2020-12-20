#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>

int main(int argc, char* argv[]){
    int put = 0;
    int c = 0, last = 0;
    char buf;
    char *out = NULL, *in = NULL;
    int ans = 0;
    
    //save only last option
    while((c = getopt(argc, argv, "lsp"))!= -1){    
        last = c;
    }
    //./scat -> use lib copy loop
    if(last == 0){
        last = 'l';
    }
    switch(last){
        //C library copy loop
        case 'l':
            fprintf(stderr, "Using lib call...\n");
            for(;;){                
                buf = getc(stdin);                    
                if(buf == EOF){
                    fprintf(stderr,"\nEnd of file(or error getc): %s\n", strerror(errno));
                    break;
                }

                put = putc(buf, stdout);
                if(put == EOF){
                    fprintf(stderr,"Error in putc: %s\n", strerror(errno));
                    break;
                }
            }
            break;
        //system call copy loop
        case 's':
            fprintf(stderr, "Using system call...\n");
            for(;;){
                ans = read(STDIN_FILENO, &buf, 1);    
                if(ans == 0){
                    fprintf(stderr, "\nEOF\n");
                    break;
                }
                if(ans == -1){
                    fprintf(stderr,"Error in read: %s\n", strerror(errno));
                    break;
                }

                put = write(STDOUT_FILENO, &buf, 1);
                if(put == -1){
                    fprintf(stderr, "Error in write: %s\n", strerror(errno));
                    break;
                }
            }
            break;
        //sendfile copy loop
        case 'p':
            fprintf(stderr, "Using sendfile()...\n");
            for(;;){

                ans = sendfile(STDOUT_FILENO, STDIN_FILENO, NULL, 4096);
                //EOF reached
                if(ans == 0){
                    fprintf(stderr, "\nEOF\n");
                    break;
                }
                //error
                if(ans == -1){
                    fprintf(stderr, "Error in sendfile: %s\n", strerror(errno));
                    exit(3);
                }
                
            }
            break;
        //unknown option -> use lib copy loop
        case '?':
            fprintf(stderr, "Using lib call...\n");
            for(;;){                
                buf = getc(stdin);                    
                if(buf == EOF){
                    fprintf(stderr, "\nEnd of file(or error getc): %s\n", strerror(errno));
                    break;
                }

                put = putc(buf, stdout);
                if(put == EOF){
                    fprintf(stderr, "Error in putc: %s\n", strerror(errno));
                    break;
                }
            }
    }
    
    

    
    return 0;
}