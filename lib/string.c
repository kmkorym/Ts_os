#include "string.h"
int string_equal(const char*s1,const char*s2){
    if(!s1&&!s2){
        return 1;
    }
    if(!s1||!s2){
        return 0;
    }
    while(*s1&&*s2){
        if(*s1++!=*s2++){
            return 0;
        }
    }
    if(*s1!=*s2){
        return 0;
    }
    return 1;
}


void strcpy(const char*src,char*dst){
    if(!src || !dst){
        return; // or print error?
    }
    while(*dst++=*src++);
}

// copy from address src to address dst
void memcpy(char*src,char*dst,uint32_t n){
    while(n--){*dst++=*src++;}
}

void strncpy(char*src,char*dst,uint32_t n){
    if(!src || !dst){
        return; // or print error?
    }
    while(n--){*dst++=*src++;}
}


//TODO: process leadning pattern
//ignore leading pattern now 
//return addr of begin of pat
char* strfd(const char*s,const char*pat){
    int i,j;
    if(!*pat){
        return NULL;
    }
    for(i=0;s[i];++i){
        for(j=0;pat[j];++j){
            if(!s[i+j] || s[i+j]!=pat[j]){
                break;
            }
        }
        if(!pat[j]){
            return (char*)&s[i];
        }
    }
    return NULL;
}


uint32_t strlen(const char*s){
    if(!s){
        return 0;
    }
    uint32_t cnt = 0;
    while(*s++){
        cnt++;
    }
    return cnt;
}


// return next token position after pattern,or NULL if there is no next
char* strtok(const char*s,const char*del,char *buffer){
    s = no_leading(s,del);
    char *next = strfd(s,del);
    if(!next){
        strcpy(s,buffer);
        return NULL;
    }
    //copy content to buffer
    while(s!=next){
        *buffer++=*s++;
    }
    *buffer=0;
    // decide next position
    next+=strlen(del);
    //if pattern exist at end of string, next is NULL
    //handle multiple consecutive pattern
    next = no_leading(next,del);
    if(!*next){
        return NULL;
    }
    return next;
}

char* no_trailing(const char*s,const char*pat){
    uint32_t l = strlen(pat);
    char *p = (char*)s+strlen(s)-l;
    
    
    while(s<=p && (p==strfd(p,pat))){
        p-=l;
    }    
    return p;
}

char* no_leading(const char*s,const char*pat){
    uint32_t l = strlen(pat);
    while(*s && (s==strfd(s,pat))){
        s+=l;
    }    
    return (char*)s;
}


uint32_t atoi(const char*s,uint8_t base){
    uint32_t sum=0;
    while(*s){
        sum*=base;
        if(*s<='9' && *s>='0' ){
            sum+=*s-'0';
        }else{
            sum+=*s-'A'+10;
        }
        ++s;
    }
    return sum;
}







int is_alpha(char c){
    if( (c>='A' && c <='Z')|| (c>='a' && c<='z')){
       return 1;
    }
    return 0;
}