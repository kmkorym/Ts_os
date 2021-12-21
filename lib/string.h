#include "../kernel/common.h"
int str_equal_range(const char*s1,const char*s2,int start,int end);
int string_equal(const char*s1,const char*s2);
void strcpy(const char*src,char*dst);
void strncpy(char*src,char*dst,uint32_t n);
void memcpy(char*src,char*dst,uint32_t n);
char* strfd(const char*s,const char*pat);
uint32_t strlen(const char*s);
char* strtok(const char*s,const char*del,char *buffer);
int is_alpha(char c);
char* no_leading(const char*s,const char*pat);
char* no_trailing(const char*s,const char*pat);
uint32_t atoi(const char*s,uint8_t base);
char upper_case(char c);
int str_pad(char *s,char value,int size);
void memset(char *buf,char value,int size);
void strcat(char *s1,char *s2,char *buf);
int itoa(int a,char *buf,int buf_size);