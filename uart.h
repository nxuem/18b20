#define BAUD  9600
#ifdef STC15F104
#define __SOFT_UART 1
#endif

#ifdef STC11F04E
// 11F04E主频只有5.66MHz
#define FOSC  5660000L
#else
// 89RC52是11.0592MHz
#define FOSC  11059200L
#endif

void init_uart();
void putchar(char);

void print_num(unsigned char dat);
// 打印16进制数据
void print_hex(char data);
