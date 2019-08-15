#include <REG51.H>
#include <ABSACC.H>
#include <INTRINS.H>
#include <MATH.H>
#include <STDLIB.H>

/*
**作者：Chr_
**邮箱：chr@chrxw.com
**
**遵循 GPL v3.0协议
*/

#define uchar unsigned char
#define uint unsigned int
/*取8位的某一位*/
#define d1 0x01
#define d2 0x02
#define d3 0x04
#define d4 0x08
#define d5 0x10
#define d6 0x20
#define d7 0x40
#define d8 0x80
/*键盘键位*/
#define k1 31
#define k2 32
#define k3 33
#define k4 21
#define k5 22
#define k6 23
#define k7 11
#define k8 12
#define k9 13
#define k0 42
#define kdiv 14
#define kmul 24
#define ksub 34
#define kadd 44
#define kclr 41
#define kequ 43
/*8255输出*/
#define PA XBYTE[0x7f00]
#define PB XBYTE[0x7f01]
#define PC XBYTE[0x7f02]

#define MAXTRYS 10; //最大尝试次数

void coreloop(void);
void flashdisplay(void);
void sleep(uint j);
void presskey(uchar keyid);

const uchar words[] = {
	/*0-9*/ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f,
	/*AbCdEFHIJLNOpqrSU*/ 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x76, 0x30, 0x0e, 0x38, 0x37, 0x5c, 0x73, 0x67, 0x50, 0x6d, 0x3e,
	/*-_.空*/ 0x40, 0x08, 0x80, 0x00}; //字形码常量

uchar keynumL = 0;
uchar keynumH = 0; //猜数字答案
uchar inputnumL = 0;
uchar inputnumH = 0; //输入的值
uchar tempinputnumL = 0;
uchar tempinputnumH = 0; //输入的值的缓存
uchar trysleft = 0;
uchar dispbuffer[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uchar randseed = 0;
uchar flag = 0x00; //[光标位置H|光标位置L|赢|输|输入数值|重置游戏|退出游戏|开始游戏]
/*主程入口*/
void main(void)
{
	EA = 1;
	EX0 = 1;
	ET1 = 1;
	TR1 = 1;
	TF1 = 1;
	IT0 = 1;
	P1 = 0x0f;			  //键盘准备接受输入
	XBYTE[0x7f03] = 0x80; //8255A初始化
	coreloop();
}
/*主循环，使用flag变量控制状态*/
void coreloop()
{
	uchar pos;	 //位选[0-9][匹配度4位][输入4位][剩余次数2位]
	uchar i, j, k; //循环计次

	for (;;)
	{
		switch (flag & 0x3f) //[光标位置H|光标位置L|赢|输|输入数值|重置游戏|退出游戏|开始游戏]
		{
		case 0x02: //[0001|xxxx]
			/*退出游戏*/
			flag = 0x00;
			inputnumH = 0;
			inputnumL = 0; //清空输入
			tempinputnumH = 0;
			tempinputnumL = 0; //清空输入缓存
			break;
		case 0x00: //[0000|0000]
			/*闲置状态*/
			for (i = 0; i <= 9; i++) //清空dispbuffer
			{
				dispbuffer[i] = 27;
			}
			dispbuffer[3] = 12;		 //C
			dispbuffer[4] = 16;		 //H
			dispbuffer[5] = 24;		 //R
			dispbuffer[6] = 28;		 //_
			for (k = 0; k < 11; k++) //滚动显示10位
			{
				for (j = 0; (j < 15) & !(flag & d1); j++) //延时
				{
					flashdisplay();
				}
				pos = dispbuffer[9];
				for (i = 9; i > 0; i--) //dispbuffer右移
				{
					dispbuffer[i] = dispbuffer[i - 1];
				}
				dispbuffer[0] = pos;
			}
			break;
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07: //[0000|01xx]
			/*重置游戏*/
			for (k = 0; k < 10; k++)
			{
				dispbuffer[k] = k;
			}
			for (k = 0; k < 4; k++) //生成4位随机数，保存在dispbuffer最后4个元素中
			{
				srand(randseed);
				i = (rand() % (9 - k + 1));
				pos = dispbuffer[i];
				for (j = i; j <= 8 - k; j++) //dispbuffer左移，取出一位放在末尾
				{
					dispbuffer[j] = dispbuffer[j + 1];
				}
				dispbuffer[9 - k] = pos;
			}
			trysleft = MAXTRYS;
			keynumH = 10 * dispbuffer[7] + dispbuffer[6];
			keynumL = 10 * dispbuffer[8] + dispbuffer[9]; //储存4位随机数字
			inputnumH = 0;
			inputnumL = 0; //清空输入
			tempinputnumH = 0;
			tempinputnumL = 0; //清空输入缓存
			flag = 0x01;
			break;
		case 0x01: //[0000|0001]
			/*开始游戏状态*/
			dispbuffer[0] = 0; //数字正确位置正确
			dispbuffer[1] = 0; //数字正确位置错误
			if (inputnumH > 0 & inputnumL > 0)
			{
				dispbuffer[2] = keynumH / 10;
				dispbuffer[3] = keynumH % 10;
				dispbuffer[4] = keynumL / 10;
				dispbuffer[5] = keynumL % 10; //答案
				dispbuffer[6] = inputnumH / 10;
				dispbuffer[7] = inputnumH % 10;
				dispbuffer[8] = inputnumL / 10;
				dispbuffer[9] = inputnumL % 10; //输入
				for (i = 2; i <= 5; i++)		//判断几位猜对
				{
					for (j = 6; j <= 9; j++)
					{
						if (dispbuffer[i] == dispbuffer[j]) //数字相同
						{
							if (i == j - 4)
							{ //位置相同
								dispbuffer[0]++;
							}
							else
							{ //位置不同
								dispbuffer[1]++;
							}
							continue;
						}
					}
				}
			}
			for (pos = 0; pos <= 9; pos++) //刷新数码管显示
			{
				switch (pos)
				{
				case 0:
					PB = ~d1;
					PC = 0xff;
					i = dispbuffer[0];
					break;
				case 1:
					PB = ~d2;
					i = 10; //A
					break;
				case 2:
					PB = ~d3;
					i = dispbuffer[1];
					break;
				case 3:
					PB = ~d4;
					i = 11; //b
					break;
				case 4:
					PC = ~d1;
					PB = 0xff;
					i = trysleft / 10;
					break;
				case 5:
					PC = ~d2;
					i = trysleft % 10;
					break;
				case 6:
					PB = ~d5;
					PC = 0xff;
					i = tempinputnumH / 10;
					break;
				case 7:
					PB = ~d6;
					i = tempinputnumH % 10;
					break;
				case 8:
					PB = ~d7;
					i = tempinputnumL / 10;
					break;
				case 9:
					PB = ~d8;
					i = tempinputnumL % 10;
					break;
				default:
					pos = 0;
				}
				i = words[i];
				if (pos >= 6)
				{
					if (!(flag & d8))
					{
						if (!(flag & d7))
						{ //00
							if (pos == 6)
							{
								i |= words[29];
							}
						}
						else
						{ //01
							if (pos == 7)
							{
								i |= words[29];
							}
						}
					}
					else
					{
						if (!(flag & d7))
						{ //10
							if (pos == 8)
							{
								i |= words[29];
							}
						}
						else
						{ //11
							if (pos == 9)
							{
								i |= words[29];
							}
						}
					}
				}
				PA = i;
				sleep(6);
			}
			break;
		case 0x09: //[0000|1001]
			/*输入数值*/
			dispbuffer[6] = tempinputnumH / 10;
			dispbuffer[7] = tempinputnumH % 10;
			dispbuffer[8] = tempinputnumL / 10;
			dispbuffer[9] = tempinputnumL % 10;
			k = 0;
			for (i = 6; i <= 9; i++)
			{
				for (j = 6; j <= 9; j++)
				{
					if (dispbuffer[i] == dispbuffer[j]) //检测是否有重复数字
					{
						if (i != j) //检测是否同一位
						{
							k++;
							break;
						}
					}
				}
			}
			if (k == 0)
			{ //数字无重复
				inputnumH = tempinputnumH;
				inputnumL = tempinputnumL;
				if (trysleft)
				{
					trysleft--;
					tempinputnumH = 0;
					tempinputnumL = 0;
					if (inputnumH == keynumH & inputnumL == keynumL)
					{
						flag = 0x20; //游戏结束，赢
					}
					else if (trysleft == 0)
					{
						flag = 0x10; //游戏结束，输
					}
					else
					{
						flag = 0x01; //flag恢复
					}
				}
				else
				{
					flag = 0x10; //游戏结束，输
				}
			}
			else
			{				 //数字重复，不做处理
				flag = 0x01; //flag恢复
			}
			break;
		case 0x20: //[0010|0000]
			/*赢*/
			dispbuffer[0] = keynumH / 10;
			dispbuffer[1] = keynumH % 10;
			dispbuffer[2] = keynumL / 10;
			dispbuffer[3] = keynumL % 10; //左边显示答案
			dispbuffer[4] = trysleft / 10;
			dispbuffer[5] = trysleft % 10;			  //生命
			dispbuffer[6] = 26;						  //U
			dispbuffer[7] = 26;						  //U
			dispbuffer[8] = 17;						  //I
			dispbuffer[9] = 20;						  //N
			for (j = 0; (j < 10) & !(flag & d1); j++) //延时
			{
				flashdisplay();
			}
			break;
		case 0x10: //[0001|0000]
			/*输*/
			dispbuffer[0] = keynumH / 10;
			dispbuffer[1] = keynumH % 10;
			dispbuffer[2] = keynumL / 10;
			dispbuffer[3] = keynumL % 10; //左边显示答案
			dispbuffer[4] = trysleft / 10;
			dispbuffer[5] = trysleft % 10;			  //生命
			dispbuffer[6] = 19;						  //L
			dispbuffer[7] = 21;						  //O
			dispbuffer[8] = 25;						  //S
			dispbuffer[9] = 14;						  //E
			for (j = 0; (j < 10) & !(flag & d1); j++) //延时
			{
				flashdisplay();
			}
			break;
		default:
			flag = 0x00;
			break;
		}
	}
}
/*刷新数码管*/
void flashdisplay()
{
	uchar i, j;
	for (i = 0; i <= 9 & !(flag & d1); i++)
	{
		switch (i)
		{
		case 0:
			PB = ~d1;
			PC = 0xff;
			break;
		case 1:
			PB = ~d2;
			break;
		case 2:
			PB = ~d3;
			break;
		case 3:
			PB = ~d4;
			break;
		case 4:
			PC = ~d1;
			PB = 0xff;
			break;
		case 5:
			PC = ~d2;
			break;
		case 6:
			PB = ~d5;
			PC = 0xff;
			break;
		case 7:
			PB = ~d6;
			break;
		case 8:
			PB = ~d7;
			break;
		case 9:
			PB = ~d8;
			break;
		default:
			i = 0;
		}
		j = dispbuffer[i];
		PA = words[j];
		sleep(6);
	}
}
/*响应按键事件*/
void presskey(uchar keyid)
{
	char input = -1;
	switch (keyid)
	{
	case k9: //9键
		input = 9;
		break;
	case k8: //8键
		input = 8;
		break;
	case k7: //7键
		input = 7;
		break;
	case k6: //6键
		input = 6;
		break;
	case k5: //5键
		input = 5;
		break;
	case k4: //4键
		input = 4;
		break;
	case k3: //3键
		input = 3;
		break;
	case k2: //2键
		input = 2;
		break;
	case k1: //1键
		input = 1;
		break;
	case k0: //0键
		input = 0;
		break;
	case kdiv: ///键
		if (flag & d1)
		{
			flag = 0x10;
		}
		break;
	case kmul: //\*键
		if (flag & d1)
		{
			tempinputnumH = 0;
			tempinputnumL = 0;
			flag &= 0x3f; //光标重置
		}
		break;
	case ksub: //-键
		if (flag & d1)
		{
			if (!(flag & d8))
			{
				if (!(flag & d7))
				{ //00
					flag |= d7;
					flag |= d8;
				}
				else
				{ //01
					flag &= ~d7;
				}
			}
			else
			{
				if (!(flag & d7))
				{ //10
					flag |= d7;
					flag &= ~d8;
				}
				else
				{ //11
					flag &= ~d7;
				}
			}
		}
		break;
	case kadd: //+键
		if (flag & d1)
		{
			if (!(flag & d8))
			{
				if (!(flag & d7))
				{ //00
					flag |= d7;
				}
				else
				{ //01
					flag &= ~d7;
					flag |= d8;
				}
			}
			else
			{
				if (!(flag & d7))
				{ //10
					flag |= d7;
				}
				else
				{ //11
					flag &= ~d7;
					flag &= ~d8;
				}
			}
		}
		break;
	case kclr: //C键
		flag = 0x04;
		break;
	case kequ: //=键
		if (flag & d1)
		{
			flag = 0x09; //进入数值输入模式
		}
		break;
	default:
		break;
	}
	if (input >= 0) //响应数字输入
	{
		if (flag & d1)
		{
			if (!(flag & d8))
			{
				if (!(flag & d7))
				{ //00
					input *= 10;
					input += tempinputnumH % 10;
					tempinputnumH = input;
				}
				else
				{ //01
					input += (tempinputnumH / 10) * 10;
					tempinputnumH = input;
				}
			}
			else
			{
				if (!(flag & d7))
				{ //10
					input *= 10;
					input += tempinputnumL % 10;
					tempinputnumL = input;
				}
				else
				{ //11
					input += (tempinputnumL / 10) * 10;
					tempinputnumL = input;
				}
			}
			presskey(kadd);
		}
		else if ((flag & d5) | (flag & d6))
		{
			flag = 0x00;
		}
	}
}
/*按键中断判断*/
void keypress() interrupt 0
{
	uchar keypos = 0; //row行, col列;
	uchar tmp;
	if (!(P3 & d3))
	{
		if (!(P1 & d1))
		{
			keypos = 10;
		}
		else if (!(P1 & d2))
		{
			keypos = 20;
		}
		else if (!(P1 & d3))
		{
			keypos = 30;
		}
		else if (!(P1 & d4))
		{
			keypos = 40;
		}
		tmp = P1 & 0x0f;

		P1 = 0x0f | d5;
		if ((P1 & 0x0f) != tmp)
		{
			keypos += 1;
		}
		else
		{
			P1 = 0x0f | d6;
			if ((P1 & 0x0f) != tmp)
			{
				keypos += 2;
			}
			else
			{
				P1 = 0x0f | d7;
				if ((P1 & 0x0f) != tmp)
				{
					keypos += 3;
				}
				else
				{
					P1 = 0x0f | d8;
					if ((P1 & 0x0f) != tmp)
					{
						keypos += 4;
					}
				}
			}
		}
		presskey(keypos);
		P1 = 0x0f; //准备接受键盘输入
		IE0 = 0;   //清除中断标志，防止一直执行
	}
}
/*定时器T1用来产生随机数*/
void timer1() interrupt 3
{
	TH1 = (65535 - 1000) / 256; //1ms定时
	TL1 = (65535 - 1000) % 256;
	if (randseed < 0xff)
	{
		randseed++;
	}
	else
	{
		randseed = 0;
	}
}
/*简单延时函数*/
void sleep(uint j)
{
	uchar i;
	for (; j; j--)
	{
		for (i = 110; i; i--)
			;
	}
}