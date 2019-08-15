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
	/*-*/ 0x40};   //字形码
int numH = 1234;   //数码管前4位
int numL = 5678;   //数码管后4位
uchar p1 = 0xfe;   //控制LED哪几位亮
uchar mask = 0x00; //控制LED闪烁

sbit m1 = P3 ^ 0;
sbit m2 = P3 ^ 1;

/*主程入口*/
void main()
{
	static uchar pos = 1; //位选[1-8]
	uchar a = 0;
	EA = 1;
	ET0 = 1;
	ET1 = 1;
	TMOD = 0x11;
	m1 = 0;
	m2 = 0;
	TR0 = 1;
	TR1 = 1;
	TF0 = 1;
	TF1 = 1;
	//开启定时器T0T1，边沿激活模式
	for (;;)
	{

		P1 = p1 | mask;
		switch (pos++) //按位操作数码管，a指每一位的数值，pos指哪一位
		{
		case 1:
			P2 = ~d1;
			a = (abs(numH) / 1000) % 10;
			break;
		case 2:
			P2 = ~d2;
			a = (abs(numH) / 100) % 10;
			break;
		case 3:
			P2 = ~d3;
			a = (abs(numH) / 10) % 10;
			break;
		case 4:
			P2 = ~d4;
			a = abs(numH) % 10;
			break;
		case 5:
			P2 = ~d5;
			a = (abs(numL) / 1000) % 10;
			break;
		case 6:
			P2 = ~d6;
			a = (abs(numL) / 100) % 10;
			break;
		case 7:
			P2 = ~d7;
			a = (abs(numL) / 10) % 10;
			break;
		case 8:
			P2 = ~d8;
			a = abs(numL) % 10;
		default:
			pos = 1;
		}
		m2 = 1;		  //解锁位选锁存器，输出位选码
		m2 = 0;		  //加锁
		m1 = 1;		  //解锁段选锁存器
		P2 = nums[a]; //输出段选码
		for (a = 100; a; a--)
			;
		P2 = 0x00; //消隐
		m1 = 0;	//加锁
	}
}
/*定时器T0，控制numL自增*/
void timer0() interrupt 1
{
	static uint t = 0;			//计次
	TH0 = (65535 - 1000) / 256; //1ms定时
	TL0 = (65535 - 1000) % 256;
	if (t == 100)
	{
		if (numL < 5732)
		{
			numL++;
			t = 0;
		}
		else
		{
			TR0 = 0;   //关闭定时器T0
			p1 = 0x00; //打开所有LED
		}
	}
	else
	{
		t++;
	}
}
/*定时器T1，控制流水灯和闪烁，最后显示学号*/
void timer1() interrupt 3
{
	static uint t1 = 0, t2 = 0; //计次
	TH1 = (65535 - 1000) / 256; //1ms定时
	TL1 = (65535 - 1000) % 256;
	if (numL < 5732)
	{
		if (t1 >= 400)
		{
			p1 = _crol_(p1, 1);
			t1 = 0;
		}
		else
		{
			t1++;
		}
	}
	else
	{
		if (t2 < 3000)
		{
			t2++;
			if (t1 >= 200)
			{
				mask = ~mask;
				p1 = _crol_(p1, 1);
				t1 = 0;
			}
			else
			{
				t1++;
			}
		}
		else
		{
			TR1 = 0;   //关闭定时器T1
			p1 = 0xff; //关闭所有LED
			numH = 181;
			numL = 4044; //显示学号
		}
	}
}