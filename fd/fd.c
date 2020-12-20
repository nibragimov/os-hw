#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <fnmatch.h>
#include <dirent.h>

int cnt = 0;
char *pattern = NULL;
int opt = 0, tflag = 0;
//t argument flags
short dval = 0, fval = 0, lval = 0, sval = 0, eval = 0, pval = 0, xval = 0,
patFlag = 0, pathFlag = 0, patMatch = 0; 

char *dirs[100], *paths[100];

void printAll(char *arr[]){
    for(int i = 0; i < cnt; i++){            
        printf("%s/", arr[i]);
    }
}

void print(char *name){
    for(int i = 0; i < cnt; i++){            
        printf("%s/", dirs[i]);
    }
    printf("%s\n", name);

}
void parse(DIR *d){
    
    struct dirent *dp;

    errno = 0;
    while(1){
        dp = readdir(d);
        //dp == NULL or returns error
        if(!dp){
            free(dirs[cnt]);
            cnt--;
            break;
        }
        
        char *name = dp->d_name;
        //name is equal to .* 
        if(name[0] == '.') continue;
        int cond = 0;
        if(tflag){
            if(dval){
                cond |= DT_DIR;
            }
            if(sval){
                cond |= DT_SOCK;
            }
            if(fval){
                cond |= DT_REG;
            }
            if(pval){
                cond |= DT_FIFO;
            }
            if(lval){
                cond |= DT_LNK;
            }
            if(xval){
                cond |= DT_REG;
            }
            if(eval){
                cond |= DT_REG;
            }
        }

        

        if(patFlag){
            int f = fnmatch(pattern, name, 0);
            if(f == 0){
                patMatch = 1;
            }
            else if(f == FNM_NOMATCH){
                patMatch = 0;
            }
            else{
                perror("fnmatch");
                exit(EXIT_FAILURE);
            }

        }

    
        //print dirs till cnt
        if(patFlag){
            if(patMatch){
                if(tflag){
                    if(dval && dp->d_type == DT_DIR){
                        print(name);
                    }
                    if(fval && dp->d_type == DT_REG){
                        print(name);
                    }
                    if(pval && dp->d_type == DT_FIFO){
                        print(name);
                    }
                    if(sval && dp->d_type == DT_SOCK){
                        print(name);
                    }
                    if(lval && dp->d_type == DT_LNK){
                        print(name);
                    }
                    //empty file
                    if(eval && dp->d_type == DT_REG && dp->d_reclen == 0){
                        print(name);
                    }
                }else{
                    print(name);
                }

            }

        }
        else{
            if(tflag){
                if(dval && dp->d_type == DT_DIR){
                    print(name);
                }
                if(fval && dp->d_type == DT_REG){
                    print(name);
                }
                if(pval && dp->d_type == DT_FIFO){
                    print(name);
                }
                if(sval && dp->d_type == DT_SOCK){
                    print(name);
                }
                if(lval && dp->d_type == DT_LNK){
                    print(name);
                }
                //empty file
                if(eval && dp->d_type == DT_REG && dp->d_reclen == 0){
                    print(name);
                }
            }else{
                print(name);
            }
        }

        //ignore if pattern does not match, but accept matching dirs
        if(patFlag && !patMatch){
            continue;
        }
        if(patMatch){
            patMatch = 0;
        }

        if(dp->d_type == DT_DIR){ 
            //allocate memory for dirs
            dirs[cnt] = (char *) malloc(sizeof(char) * strlen(name) + 1);
            if(!dirs[cnt]){ 
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strncpy(dirs[cnt], name, strlen(name));
            
           
            //fprintf(stderr, "%s\n", name);
            DIR* dir = opendir(name);
            if(!dir){
                //fprintf(stderr, "opendir: %s\n", strerror(errno));
                perror("opendir");
                (void) closedir(d);
                exit(EXIT_FAILURE);
            }
            if(chdir(name) == -1) {
                perror("chdir");
                exit(EXIT_FAILURE);
            }
            cnt++;
            parse(dir);
            
        }


    }
    //error in readdir
    if(errno != 0){
        perror("error reading");
        exit(EXIT_FAILURE);
    }    
    

}

int main(int argc, char *argv[]){
    // tflag = 1;
    // fval = 1;
    


    int ind;
    //char *ftype;
    while(1){
        if((opt = getopt(argc, argv, "t:")) == -1){
            break;
            perror("getopt");
            return EXIT_FAILURE;
        }
        switch(opt){
            case 't':
                tflag = 1;
                if(strlen(optarg) == 1){
                    switch(optarg[0]){
                        case 'd': dval = 1; break;
                        case 'f': fval = 1; break;
                        case 'l': lval = 1; break;
                        case 's': sval = 1; break;
                        case 'e': eval = 1; break;
                        case 'p': pval = 1; break;
                        case 'x': xval = 1; break;
                    }
                }
                else{
                    fprintf(stderr, "invalid optarg\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case '?':
                if(optopt == 't'){
                    fprintf(stderr, "Option -t requires an argument.\n");    
                }
                else{
                    fprintf(stderr, "Valid options: -t\n");
                }    
                exit(EXIT_FAILURE);
                break;
        }
    }
    //look for pattern, if its not . , / treat it as pattern
    //fprintf(stderr,"%d\n",optind);
    
    if(argc != optind){
        char temp = argv[optind][0];
        if(temp != '/' || temp != '.'){
            patFlag = 1;
            pattern = argv[optind];
            optind++;
        }
    }
    
    //look for paths, and add them to paths var
    for(int ind = optind; ind < argc; ind++){
        pathFlag = 1;
        paths[ind - optind] = argv[ind]; 
    }
    
    // for(int i = 0; i < argc - optind; i++){
    //     fprintf(stderr, "%s ", paths[i]);
    // }
    // fprintf(stderr, "\n");
    
    
    if(pathFlag){
        for(int i = 0; i < argc - optind; i++){
            dirs[cnt] = (char *) malloc(sizeof(char) * strlen(paths[i]) + 1);
            if(!dirs[cnt]){ 
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strncpy(dirs[cnt], paths[i], strlen(paths[i]));

            cnt++;
            DIR *d = opendir(paths[i]);
            if(!d){
                perror("opendir");
                exit(EXIT_FAILURE);
            }
            parse(d);
            (void) closedir(d);
        }
        return 0;
    }

    //doing for noargs noptions
    char *path = ".";
    DIR *d = opendir(path);

    //char *prev = (char*) malloc(sizeof(char) * 100);
    //prev[0] = '/';
    if(!d){
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    parse(d);

    (void) closedir(d);

    return 0;
}
    