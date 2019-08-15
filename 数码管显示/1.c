#include <REGX51.H>
#include <INTRINS.H>
#include <MATH.H>
#define uchar unsigned char
#define uint unsigned int
#define d1 0x01
#define d2 0x02
#define d3 0x04
#define d4 0x08
#define d5 0x10
#define d6 0x20
#define d7 0x40
#define d8 0x80
uchar nums[] = {
	/*0-9*/ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
	/*-*/ 0x40}; //字形码

int num = 0; //要显示的数字

/*主程入口*/
void main()
{
	P3_0 = 0; //关闭蜂鸣器
	EA = 1;
	ET1 = 1;
	EX0 = 1;
	IT0 = 1;
	TMOD = 0x10;
	TR1 = 1;
	TF1 = 1;
	//开启定时器T1，设置为16位模式
	//开启外部中断0，边沿激活模式
	for (;;)
		;
}
/*定时器T1用来刷新数码管显示*/
void timer1() interrupt 3
{
	static uchar pos = 1; //位选[1-6]
	uchar a = 0;
	TH1 = (65535 - 1000) / 256; //1ms定时
	TL1 = (65535 - 1000) % 256; //大约1ms刷新显示1位
	switch (pos++)				//显示num的1~6位
	{
	case 1:
		P2 = ~d1;
		if (num < 0)
		{
			a = 10;
		}
		else
		{
			a = 0;
		}
		break;
	case 2:
		P2 = ~d2;
		a = (abs(num) / 10000) % 10;
		break;
	case 3:
		P2 = ~d3;
		a = (abs(num) / 1000) % 10;
		break;
	case 4:
		P2 = ~d4;
		a = (abs(num) / 100) % 10;
		break;
	case 5:
		P2 = ~d5;
		a = (abs(num) / 10) % 10;
		break;
	case 6:
		P2 = ~d6;
		a = abs(num) % 10;
	default:
		pos = 1;
	}
	P1 = nums[a];
	for (a = 100; a; a--)
		;
	P1 = 0x00; //数码管消隐
}
/*按键中断处理*/
void keypress() interrupt 0
{
	uchar i;
	if (!(P3 & d3))
	{
		if (!(P3 & d6))
		{					 //k1按下
			if (num < 32768) //防止num溢出
			{
				num++;
			}
			else
			{
				num = -32767;
			}
		}
		if (!(P3 & d7))
		{ //k2按下
			if (num > -32767)
			{
				num--;
			}
			else
			{
				num = 32768;
			}
		}
		if (!(P3 & d8))
		{ //k3按下
			num = 0;
		}
		P3_0 = 1; //激活蜂鸣器
		for (i = 500; i; i--)
			;
		P3_0 = 0;
	}
}