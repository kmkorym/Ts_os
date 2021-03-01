#include "../kernel/common.h"
int string_equal(const char*s1,const char*s2);
void strcpy(const char*src,char*dst);
void strncpy(char*src,char*dst,uint32_t n);
char* strfd(const char*s,const char*pat);
uint32_t strlen(const char*s);
char* strtok(const char*s,const char*del,char *buffer);
int is_alpha(char c);
char* no_leading(const char*s,const char*pat);
char* no_trailing(const char*s,const char*pat);
uint32_t atoi(const char*s,uint8_t base);
