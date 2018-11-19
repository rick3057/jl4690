/*******************************************************************************************
File Name:  IRFF00.h

Version:    1.00

Discription: ���ⰴ���� 0xFF00

Author:     Caibingquan

Email:      change.tsai@gmail.com

Date:       2012.06.04

Copyright:(c) 2012 , All Rights Reserved.
*******************************************************************************************/



/*      
power pp   MUTE
 00      04       08

      V+ /05	 

PREV     NEXT    
01         09    	 

     V-/ 06 	        

mode     eq     0B
03          07  	 1

1        2		3
40	  44		 48

4	  5		6
41	 45 		49

7	  8		9
42	  46 		4a

*/

#ifdef support_double_ir
#define IR_SYS_CODE2			0xFE01
const u8 IRTabFF00_2[] = 	   /*user code is 0xFF00*/
#else 
#define IR_SYS_CODE			0xFE01
const u8 IRTabFF00[] = 	   /*user code is 0xFF00*/
#endif 
{
/*0x00*/IRC_STANBY,  //0
/*0x01*/IRC_PREV,// 1
/*0x02*/0XFF,//02
/*0x03*/IRC_MODE,//03
/*0x04*/IRC_PLAY_PAUSE,//04
/*0x05*/IRC_VOL_UP,//05
/*0x06*/IRC_VOL_DN,//06
/*0x07*/IRC_EQ,//07
/*0x08*/  IRC_MUTE,//08
/*0x09*/IRC_NEXT,//09
/*0x0a*/  0XFF,//10
/*0x0b*/ IRC_NUM0,//11
/*0x0c*/IRC_NUM1,//12
/*0x0d*/IRC_REPEAT,//13
/*0x0e*/ 0XFF,//14
/*0x0f*/0XFF,//15
/*0x10*/0XFF,//16
/*0x11*/0XFF,//17
/*0x12*/0XFF,//18
/*0x13*/0XFF,//19
/*0x14*/0XFF,//20
/*0x15*/IRC_VOL_UP,//21
/*0x16*/IRC_NUM0,//22
/*0x17*/ 0XFF,//23
/*0x18*/ IRC_NUM2,//24
/*0x19*/IRC_ADD100,//25
/*0x1a*/ 0XFF,//26
/*0x1b*/0XFF,//27
/*0x1c*/IRC_NUM5,//28
/*0x1d*/0XFF,//29
/*0x1e*/0XFF,//30
/*0x1f*/0XFF,//31

//NKEY_20, NKEY_21, NKEY_22, NKEY_23, NKEY_24, NKEY_25, NKEY_26, NKEY_27, NKEY_28, NKEY_29, NKEY_2A, NKEY_2B, NKEY_2C, NKEY_2D, NKEY_2E, NKEY_2F,
//NKEY_30, NKEY_31, NKEY_32, NKEY_33, NKEY_34, NKEY_35, NKEY_36, NKEY_37, NKEY_38, NKEY_39, NKEY_3A, NKEY_3B, NKEY_3C, NKEY_3D, NKEY_3E, NKEY_3F,

/*0x40*/ IRC_NUM1,//64
/*0x41*/ IRC_NUM4,// 65
/*0x42*/ IRC_NUM7,//66
/*0x43*/IRC_PLAY_PAUSE,//67
/*0x44*/IRC_NUM2,//68
/*0x45*/IRC_NUM5,//69
/*0x46*/ IRC_NUM8,//70
/*0x47*/ IRC_MUTE,//71
/*0x48*/ IRC_NUM3,//72
/*0x49*/IRC_NUM6,//73
/*0x4a*/IRC_NUM9,//74
/*0x4b*/0XFF,//75
/*0x4c*/0XFF,//76
/*0x4d*/0XFF,//77
/*0x4e*/0XFF,//78
/*0x4f*/0XFF,//79
/*0x50*/0XFF,//80
/*0x51*/0XFF,//81
/*0x52*/ IRC_NUM8,//82
/*0x53*/0XFF,//83
/*0x54*/0XFF,//84
/*0x55*/ 0XFF,//85
/*0x56*/ 0XFF,//86
/*0x57*/0XFF,//87
/*0x58*/0XFF,//88
/*0x59*/0XFF,//89
/*0x5a*/IRC_NUM6,//90
/*0x5b*/0XFF,//91
/*0x5c*/0XFF,//92
/*0x5d*/0XFF,//93
/*0x5e*/IRC_NUM3,//94
/*0x5f*/0XFF,//95
};


