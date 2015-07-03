/**
*　　　　　　　　┏┓　　　┏┓+ +
*　　　　　　　┏┛┻━━━┛┻┓ + +
*　　　　　　　┃　　　　　　　┃
*　　　　　　　┃　　　━　　　┃ ++ + + +
*　　　　　　　┃　████━████ ┃+
*　　　　　　　┃　　　　　　　┃ +
*　　　　　　　┃　　　┻　　　┃
*　　　　　　　┃　　　　　　　┃ + +
*　　　　　　　┗━┓　　　┏━┛
*　　　　　　　　　┃　　　┃
*　　　　　　　　　┃　　　┃ + + + +
*　　　　　　　　　┃　　　┃　　　　Code is far away from bug
*　　　　　　　　　┃　　　┃　　　　with the animal protecting
*　　　　　　　　　┃　　　┃ + 　　　　神兽保佑,代码无bug
*　　　　　　　　　┃　　　┃
*　　　　　　　　　┃　　　┃　　+
*　　　　　　　　　┃　 　　┗━━━┓ + +
*　　　　　　　　　┃ 　　　　　　　┣┓
*　　　　　　　　　┃ 　　　　　　　┏┛
*　　　　　　　　　┗┓┓┏━┳┓┏┛ + + + +
*　　　　　　　　　　┃┫┫　┃┫┫
*　　　　　　　　　　┗┻┛　┗┻┛+ + + +
*/

/*****************************************************
 *  自动温度采集探头
 *  
 *  STC89C52       DS18B20@P3.7
 *  STC11F04E      DS18B20@P3.3 (Pin 7)
 *  STC15W204S     DS18B20@P3.3
 *  STC15F104(E/W) DS18B20@P3.3 
 *
 *  9600,n,8,1 
 *  每秒输出一次时标、测温值、ROM地址
 *
 *  Yao Fei  feiyao@me.com
 */

#include <stc12.h>

#include "uart.h"
#include "ds18b20.h"

typedef unsigned char uchar;

// for Keil C compatible
#define __nop__    __asm  nop __endasm

// 小数部分  1/16 = 0.0625 
char const __code digis[16]= {0, 6, 13, 19, 
			      25, 31, 38, 44, 
			      50, 56, 63, 69,
			      75, 81, 88, 94};

char flag;   // 是否采样标志
// 采集到的温度
char TPH, TPL;
char rom[4][8];  //Max 4 DS18B20

// 初始化定时器
#define HZ    100
#define T0MS  (65536 - FOSC/12/HZ)

void init_timer0()
{
#ifdef STC11F04E
	TMOD = 1;  // 标准8051 模式1,16比特
#else
	TMOD = 0;  // Timer0 for 100Hz
		   // Mode 0, 16bit auto load
#endif
	TL0 = T0MS & 255;
	TH0 = T0MS>>8;
	TR0   = 1;
	ET0   = 1;
}

// Timer 0 handler
uchar times = 0;
void timer0() __interrupt 1 __using 2
{
#ifdef STC11F04E  // 标准8051不能自动装载
	TL0 = T0MS & 255;
	TH0 = T0MS>>8;
#endif
	times++;
	if (HZ == times) {
		flag = 0x55;
		times = 0;
	}

#ifdef STC15F104
	P3_2 = !P3_2;
#else
	P1_1 = !P1_1; // for 100Hz test
#endif
}

#ifndef STC15F104
// UART interrupt handler
void serial() __interrupt 4 __using 3
{
	uchar c;
	
	if (RI) {
		c = SBUF;
		RI = 0;

		if ( 't' == c ) 
			flag = 0x55;
		else
			flag = 0;
	} 
}
#endif

/*
 * 主程序
 */
int main()
{
	char h, l;
	uchar hour=0, minu=0, sec=0; //时分秒

	init_uart();
	init_timer0();

	if (0 == StartDS18B20()) {
		DS18B20_ReadRom(rom[0]);
	}
	
	while(1) {
		// 恶心的前后台，中断触发标志
		if (flag) {
			flag = 0;

			h = StartDS18B20();

			// 先打印时标
			print_num(hour);
			putchar(':');
			print_num(minu);
			putchar(':');
			print_num(sec);
			putchar('\t');

			if (h==0) {
				ReadTemp(rom[0]);
				
				h = (TPH<<4) + ((TPL>>4) & 0x0f);
				
				if (h<0) 
					l = digis[16 - (TPL&0xf)];
				else
					l = digis[TPL&0xf];
				
			
				if (h<0) {
					putchar('-');
					h = -h;
				}
				print_num(h);
				
				putchar('.');
				print_num(l);

				putchar('\t');
				
				// print ROM
				for (l=0; l<8; l++) {
					print_hex(rom[0][l]);
					if (7==l)
						putchar('\t');
					else
						putchar(':');
				}
			}

			print_num(times);
			putchar('\n');
			
			// update h:m:s
			if (59 == sec++) {
				sec = 0;
				if (59 == minu++) {
					minu = 0;
					if (23 == hour++) { 
						hour = 0;
					}
				}
			}
		}
	}
}

