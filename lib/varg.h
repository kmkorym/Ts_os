#define round_up_pow_2(n,x)( (n+ (1<<x)-1)& ~((1<<x)-1))
#define INT_SIZE_2_POW 2
#define va_start(arg_addr,typ)({\
    char* p = (char*)(arg_addr);\
    p = va_next(p,typ);\
    p;\
})


#define va_next(va_ptr,typ)(va_ptr+round_up_pow_2(sizeof(typ),INT_SIZE_2_POW))
