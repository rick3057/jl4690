

/*
GPIO SET:
	LED7-TY IO	PA10-AMP_POWER PA11-MUTE
	AUX0-PA1PA2		PB11-BAT_DET		PB12-POWER_IO	PB3-IR	PB10-LED1
	PB9-ADKEY	PA9-MIC_ONOFF
	SD1-PB0_DAT_PB1_CMD_PB2_CLK	
	
1、开机时LOW-V(41)脚输出高电平
	POWER(26)脚输出高电平。
	MUTE(25)脚输出高电平，1秒后输出低电平
2、当检测电压低于1.90V时。
LED屏闪动5秒后
LOW-V(41)脚输出低电平。
POWER(26)脚输出低电平
MUTE(25)脚输出高电平
3、按遥控MUTE/暂停键时,外部MUTE脚不变（内部静音）
4、按遥控电源键关机时,MUTE脚输出高电平，
LOW-V/POWER脚不变。
因为MUTE脚是控制功放IC静音的。
5 、BTJC电量检测脚说明：
当电量检测脚是1.9V时，显示屏电量第一格闪动
当电量检测脚是2.1V时，显示屏电量第一格常亮
当电量检测脚是2.3V时，显示屏电量第二格常亮
当电量检测脚是2.5V时，显示屏电量第三格常亮
6 、27脚接话筒优先：
1、当41脚检测电压低电平时，音乐音量减到15级。
2、当41脚恢复高电平时，音乐音量续级加到最大�
�
*/


/*
*********************************************************************
                        DEBUG MACRO
*********************************************************************
*/

//  #define __DEBUG         ////<开启系统打印调试功能

/*
*********************************************************************
                        APPLICATION FUNCTION
*********************************************************************
*/
#define AC6901A_OFF_IO  // JSUO ADD  20161111


#define EARPHONE_MODE_EN    0

/*
*********************************************************************
                        COMMON MACRO
*********************************************************************
*/



#define EQ_EN			        1       ///<EQ模块开关
#define EQ_UART_BDG	    		0       ///<EQ uart online dbg switch
#define DAC_SOUNDTRACK_COMPOUND 0       ///<dac声道合并
#define DAC_AUTO_MUTE_EN		1       ///<自动mute
#define SYS_LVD_EN              1       ///<电量监测
#define KEY_TONE_EN     	    1       ///<按键音
#define UART_UPDATA_EN          0      ///<串口升级
#ifdef  MINI
    #define KEY_DOUBLE_CLICK                ///<按键双击功能
#endif

//=================================================
// 1-VOL SET JSUO
#define SYS_DEFAULT_VOL        30      ///<非0表示使用默认音量
#if  SYS_DEFAULT_VOL
#define SUPPORT_ZY_VOL_ADJUST  
#else 
#define RESET_DEFAULT_VOL   25 
#define RESET_MIN_VOL             10
#endif 

/*
*********************************************************************
                        DEVICE MACRO
*********************************************************************
*/
// 2- SD USB PC  EN	JSUO

    #define SDMMC0_EN       0
    #define SDMMC1_EN       1
    #define USB_DISK_EN     1
    #define USB_PC_EN        1


#if EARPHONE_MODE_EN
    #define VCOMO_EN 	1	//是否选择VCMO直推耳机
    #define DAC_CHANNEL_SLECT  DAC_L_R_CHANNEL
#else
    #define VCOMO_EN 	0	//是否选择VCMO直推耳机
    #define DAC_CHANNEL_SLECT  DAC_L_R_CHANNEL
#endif


/*
*********************************************************************
                        APP MACRO
*********************************************************************
*/

  #define SUPPORT_APP_RCSP_EN         0   ///<标准SDK，手机app 功能暂时不支持

#if SUPPORT_APP_RCSP_EN
    #define RCSP_LIGHT_HW               1   //light hardware support
#else
    #define RCSP_LIGHT_HW               0
#endif


/*
*********************************************************************
                        BT MACRO
*********************************************************************
*/
#include "bluetooth/bluetooth_api.h"

#define BT_MODE      NORMAL_MODE

#if BT_MODE==TEST_FCC_MODE
  #undef UART_UPDATA_EN
  #define UART_UPDATA_EN  0
#endif

#define BT_ANALOG_CFG   0//模拟配置
#define BT_XOSC         0//

#define BT_BACKMODE         0             ///<蓝牙后台


#if !BT_BACKMODE
    #define BT_HID_INDEPENDENT_MODE  0    ///<HID拍照的独立模式只支持非后台
#endif

#define BT_PHONE_NUMBER                    0          ///<来电报号
#if EARPHONE_MODE_EN
	#define AUTO_SHUT_DOWN_TIME 3*2*60          ///蓝牙无连接自动关机计时，u16类型，0表示不自动关机
#else
	#define AUTO_SHUT_DOWN_TIME 0          ///蓝牙无连接自动关机计时，u16类型，0表示不自动关机
#endif

/*
*********************************************************************
                       电源设置 
*********************************************************************
*/
#define    PWR_NO_CHANGE     0     //   0:  no change   
#define    PWR_LDO33         1     //   1:  LDOIN 5v -> VDDIO 3.3v -> DVDD 1.2v
#define    PWR_LDO15         2     //   2:  LDOIN 5v -> LDO   1.5v -> DVDD 1.2v, support bluetooth
#define    PWR_DCDC15        3     //   3:  LDOIN 5v -> DCDC  1.5v -> DVDD 1.2v, support bluetooth

#if EARPHONE_MODE_EN
	#define PWR_MODE_SELECT      PWR_DCDC15 
#else
	#define PWR_MODE_SELECT      PWR_LDO15
#endif

/*
*********************************************************************
                       低功耗设置 
*********************************************************************
*/
#define    BT_OSC           0
#define    RTC_OSCH         1
#define    RTC_OSCL         2

#if EARPHONE_MODE_EN
    #define SNIFF_MODE_CONF    SNIFF_EN
    #define BT_LOW_POWER_MODE  BT_POWER_DOWN_EN //BT_POWER_OFF_EN
    #define LOWPOWER_OSC_TYPE  BT_OSC
    #define LOWPOWER_OSC_HZ    24000000L
#else
    #define SNIFF_MODE_CONF    0//SNIFF_EN
    #define BT_LOW_POWER_MODE  0//BT_POWER_DOWN_EN //BT_POWER_OFF_EN
    #define LOWPOWER_OSC_TYPE  BT_OSC//RTC_OSCL//BT_OSC
    #define LOWPOWER_OSC_HZ    24000000L//32768L//24000000L
#endif

#define D2SH_DIS_SW         0          //d2sh dis sw  rtcvdd口没有拉出来要置1
#define BLE_BREDR_MODE      BT_BREDR_EN


/*
*********************************************************************
                       芯片封装配置 
*********************************************************************
*/

#define RTCVDD_TYPE              0          //RTCVDD口没有绑出来要置1,目前对应封装芯片AC6905
#define BTAVDD_TYPE              0          //BTAVDD没有绑出来要置1 ，目前对应封装芯片AC6907A,AC6908A

/*
*********************************************************************
    MUSIC MACRO
*********************************************************************
*/
#define MUSIC_DECRYPT_EN 		0	//SMP加密文件支持
#define MUSIC_AB_RPT_EN 		1	//AB断点支持


#define DEC_TYPE_MP3_ENABLE         ///<MP3
#define DEC_TYPE_SBC_ENABLE         ///<SBC

#ifndef MINI
    #define DEC_TYPE_WAV_ENABLE     ///<3K_code_space
    #define DEC_TYPE_FLAC_ENABLE    ///<5K_code_space
    #define DEC_TYPE_APE_ENABLE     ///<8K_code_space
    #define DEC_TYPE_WMA_ENABLE     ///<30K_code_space
    // #define DEC_TYPE_F1A_ENABLE     ///<30K_code_space
#endif



/*
*********************************************************************
                        FM MACRO
*********************************************************************
*/
// 3- FM EN	JSUO

#ifdef MINI
    #define FM_RADIO_EN         0       ///<MINI_BT
#else
    #define FM_RADIO_EN         1       ///<标准SDK
#endif

#if FM_RADIO_EN
    #define RDA5807                 0       ///<RDA5807FM
    #define BK1080                  0       ///<BK1080FM
    #define QN8035                  0       ///<QN8035FM
    #define FM_INSIDE               1       ///<芯片内部FM
#endif

/*
*********************************************************************
                        RTC MACRO
*********************************************************************
*/
  #define RTC_CLK_EN          0       ///<标准SDK RTC时钟模式

#if RTC_CLK_EN
    #define RTC_ALM_EN          1       ///<RTC闹钟模式
    #define RTC_CLK_CHECK_EN    0       ///<RTC晶振检测
#endif

/*
*********************************************************************
                        ECHO MACRO
*********************************************************************
*/
  #define ECHO_EN                      0       ///<混响功能 标准SDK
  #define ECHO_TASK_EN        0       ///<混响模式 标准SDK

/*
*********************************************************************
                        REC MACRO
*********************************************************************
*/
// 4- REC EN	JSUO

    #define REC_EN             1   ///<标准SDK录音功能

#if REC_EN

#define support_rec_stop_to_music
#define support_only_rec_key_to_rec_mode
#define support_music_key_mode
#define SUPPORT_RECPLAY_STATE

#define SUPPORT_REC_SHOW_TIME_BUG // SDK  NO SHOWTIME

//#define support_off_mode_change_voice
//#define support_fm_key_mode
//#define support_bt_key_mode
//#define support_line_key_mode

extern u16 rec_file_startcnt;
extern u16 rec_file_endcnt;
extern bool  muisc_to_rec_flag;
#ifdef support_rec_line_fm_rec_return_line_fm
extern u8   play_rec_last_file;   
#else 
extern  bool  play_rec_last_file;   
#endif 
#endif


#if 0 //SUPPORT_APP_RCSP_EN
    #undef  REC_EN
    #define REC_EN         0         ///<手机APP模式不支持录音
#endif

#if REC_EN
	#define BT_REC_EN		0       ///<蓝牙录音使能
	#define MIC_REC_EN		1       ///<MIC录音使能
	#define FM_REC_EN		0       ///<FM录音使能
	#define AUX_REC_EN		0       ///<AUX录音使能
#endif


/*
*********************************************************************
                        UI MACRO
*********************************************************************
*/

// 5- UI	EN	JSUO

   #define UI_ENABLE                1     ///<UI_显示
 

#if  UI_ENABLE
    ///二选一
    #define LCD_128X64_EN         0      ///<lcd 支持
    #define LED_7_EN                       1      ///<led 支持     
    #define LCD_SUPPORT_MENU     0       ///菜单显示
    #define LRC_LYRICS_EN                 0       ///LRC歌词显示


#define support_led7_user_pc0pa14pa15pc1pc2pc3pc4
#define LED_7PIN_MODE
#define LED_7PIN_REVERSE
	
//#define LED_ID_88N45
#ifdef LED_ID_88N45
#define LED_COM_MAX 7
#define LED_8NUMS 4
#define LED_8GRIDS 7
#define LED_ICON_NUMS 10
#endif

#define LED_ID_JW4018D_1B
#ifdef LED_ID_JW4018D_1B
#define LED_COM_MAX 7
#define LED_8NUMS 4
#define LED_8GRIDS 7
#define LED_ICON_NUMS 10
#define SUPPORT_SD40X18_1SRB_15_OFF_BAT_ICON
#endif


#define SUPPORT_FLASH_ICON
#define SUPPORT_FLASH_CHAR

#define SUPPORT_BT_LED_PRORAM


#ifdef SUPPORT_FLASH_CHAR
#define SUPPORT_LED_BLUE_STRING_FLASH
#define SUPPORT_MUTE_FLASH
#define MUTE_FLASH_ALL_SCREEN
//#define MUTE_FLASH_ZERO_VOL
#endif
#define SUPPORT_EQ_SHOW_CHAR
#define SUPPORT_IDLE_SHOW_STBY

//#define support_aux_led_disp_line     //屏上显示"LINE"
#define support_led_disp_pause
#define support_mute_led_all_flish    //MUTE闪屏
#define SUPPORT_NO_SHOW_LOAD	
 #endif 


//*********************************************************************
 

//-------ADKEY--------------------------------------------------------


#define muteh_key_mode   1

#if muteh_key_mode
#define KEY_AD_EN       1   ///<AD按键使能
#define KEY_IO_EN       0   ///<IO按键使能
#define KEY_IR_EN       1   ///<红外遥控使能,PA9
#define KEY_TCH_EN      0   ///<触摸按键使能
#endif 

#if KEY_AD_EN  
//#define adk_user_pa9
#define adk_user_pb9
#endif 

#if  0//KEY_IO_EN         ///<IO按键使能
#define support_io_scan_user_PR2               //使用 PR2 扫描按键
#define support_poweroff_wakeup_io_PR2   
#define support_k7036_scna_mode
#define support_690x_ic_rtc_stanby    //IC RTC-待机模式
#define support_power_off_voice           //有关机语音 
#endif 


//#define adk_10k_15key_mode_zy_2rec_mic_rgb
#define adk_10k_15key_mode_zy_2rec_mic_S501


//--------------------------
#define support_bt_key_mode


#define ADK_USE_15KEY_MODE

#ifdef ADK_USE_15KEY_MODE
#define adkey_max_num	15

#define PULL_UP_RESISTANCE 10   //上拉电阻
#define KEYE_RES 0  //PP
#define KEYD_RES 1 	 //REC/RECPLAY
#define KEYC_RES 2  	 // 
#define KEYB_RES 3.3 //mode		0.8
#define KEYA_RES 4.7  //null  		1v
#define KEY9_RES 5.6 //null		1.2
#define KEY8_RES 7.5 //bt  		1.4
#define KEY7_RES 10   //repeat		1.65
#define KEY6_RES 12  // 			1.8
#define KEY5_RES 15  //eq			2.0
#define KEY4_RES 20 //next/v+		2.2	
#define KEY3_RES 33 //prev/v-		2.5
#define KEY2_RES 56 //mic		2.8
#define KEY1_RES 68 // mute		2.87
#define KEY0_NOKEY_RES 100	 //stop	 3v
#endif

//#define adk_user_6_key_mode_and_power
//#define adk_use_22k_6key_mode
#ifdef adk_use_22k_6key_mode
//#define LONG_PP_KEY_AS_FM_SCAN
//#define SUPOORT_LONG_PP_AS_BT_CALL_REJECT
//#define SUPOORT_LONG_MODE_AS_BT_CONNECT_CTL
//#define SUPOORT_LONG_PP_AS_BT_CONNECT_CTL
#endif

//-------IR---------------------------------------

#if KEY_IR_EN         ///<红外遥控使能,PA9
//#define  IR_IO_PA_X     4  // PA14
#define  IR_IO_PB_X     3  // PB3
//#define  IR_IO_PC_X     14  // PC14
//#define  IR_IO_PD_X     14  // PD14

//#define IR_USE_DOUBLE_TY_00FF_807F_22KEY
//#define IR_USE_DOUBLE_ZY2119_00FF_807F_22KEY
//#define  ir_max_num     22
//#define  IR_NUMS     2


#define IR_ZY_DOUBLE_GENERAL_TY_00FF_HD7369_807F_30KEY_REC_S501
#ifdef IR_ZY_DOUBLE_GENERAL_TY_00FF_HD7369_807F_30KEY_REC_S501

#define  ir_max_num     30
#define IR_NUMS 2
#define SUPPORT_MUSIC_FOLDER_OPERATION // folder prev next repeat key
#define support_fm_key_mode
#define support_music_key_mode

#endif

#define   support_msg_stop_key 
#define   SUPPORT_IR_USE_SINGLE_MSG 
#define   support_stanby_mod 

#endif 

//-------LED---------------------------------------

#define LED_EN                  1       ///<LED指示使能
//#define  PP_LED_USER_PB9
//#define  PP_LED_USER_PB10
#define  PP_LED_USER_PR2

//-------MUTE---------------------------------------
//MUTE IO 
//    #define   PA_ENABLE
#ifdef PA_ENABLE   
//        #define  pa_mute_use_pin_PA11_PA10    // pa_mute_use_pin_PR0 //
        #define  pa_mute_use_pin_PA11// pa_mute_use_pin_PR0 //
//       #define   mute_io_L_is_mute   //  低MUTE   不定义为高MUTE    
      #define support_vol_is_0_mute_amp	//音量到0 MUTE 功放
//      #define support_pause_stop_mute_amp
//      #define  support_bt_pause_mute_amp		  	
//        #define support_xa9901_ab_d_mode
#endif 


//-------AB/D---------------------------------------
//#define support_amP_ab_d
//#define amp_ab_d_user_PB6  


//-------LINE DET---------------------------------------

//LINE INPUT IO SELECT
#define    line_user_PA1PA2  //AUX0 


// AUX DETECT SET
#define AUX_DETECT_EN      0
 #if  AUX_DETECT_EN
    #define  support_line_chack_user_PA3
    #define  support_line_unline_to_next_mode   //CHANGE_WORKMODE AUX UNLINE TO NEXT MODE
  //  #define AUX_OUT_WITHOUT_CNT  //PLUS OUT DELAY    防抖
#endif 

//#define  SUPPORT_MULTI_INPUT_MODE
#ifdef SUPPORT_MULTI_INPUT_MODE
extern u8 bMultiExternInputSel;
#define    line2_user_PA3PA4  //AUX1
#define    SUPPOR_INPUT_AUX1  //AUX1\
//#define    SUPPOR_INPUT_AUX2  //AUX2
//#define    SUPPOR_MP3_NOP_IN_AUX_MODE
enum
{
    INPUT_AUX0_MODE=0,
    INPUT_AUX1_MODE,
    INPUT_MAX_MODE,
    INPUT_AUX2_MODE,
    INPUT_NOP_MODE,
};

#endif

//-------POWER ON SET ---------------------------------------
//#define  power_on_no_usb_sd_to_bt 
#define  power_on_no_usb_sd_to_line   
//#define  usb_sd_out_to_bt_mode   
#define  usb_sd_out_to_line_mode   



//-------GENERAL  SET----------------------------------
#define SUPPORT_BT_AUTO_PLAY	0  // BT MUSIC AUTO PLAY
#define SUPPORT_OFF_PHONE_CALL   1 // OFF PHONE 
//#define SUPPORT_OFF_MODE_VOICE     // OFF MUSIC LINE FM MODE VOICE
#define SUPPORT_REPEAT_ONLY_USE_ONE_ALL
#define SUPPORT_AUX_OUT_GAIN_ADD
#define SUPPORT_FM_SCAN_PREVIEW_DISABLE


//-------ELSE ---------------------------------------
//#define  support_msg_light_key
#ifdef support_msg_light_key
#define Light_max_mode   2
#define  support_default_light_on
#endif 

#if KEY_TONE_EN
#define  support_vol_max_tone_en  0
#endif 

// 频谱显示
//#define support_led_eq_disp    //支持发光管频谱显示
//#define support_led_eq_user_PA14PA15PC0PC1PC2  
//#define support_led_eq_is_5led_user_k7036    //
//----------------------------------------------

#define MEDIA_MODE_USB_SD_DETECT 0
#if  MEDIA_MODE_USB_SD_DETECT	
u32 curr_device_using;
#endif

//---FM  STATION  ADJUST-------------------------
#define FM_SCAN_MODE_ZY_DATA_AMP

//----------------------------------------------

//MIC检测
#define MIC_CHECK_ENABLE	1
#if MIC_CHECK_ENABLE
 #define MIC_ONOFF_KEY	1
// #define  support_mic_check_user_PB11
// #define  support_mic_check_user_PA9
 #define  support_mic_check_user_PB10
//#define   KOK_DETECT_USE_IO_ADC_VALUE
// #define  support_on_io_dn_10k
// #define  support_high_mic_in
//#define MIC_CHECK_OFF_DELAY_TIME  
#define MIC_CHECK_TIMER  20


#define KOK_DETECT_AUTO_VOL_SET
#define AUTO_VOL_SET_DEC  15

#define SUPPORT_MIC_ONOFF_SHOW  
#ifdef SUPPORT_MIC_ONOFF_SHOW
extern u8 bLedShowOnoffTimer;
#endif
//  #define support_high_mic_in 
extern bool mic_sw;  // 0-off  1-on
 extern bool auto_vol_sw; // 0 off   1 on
 extern  u8 store_vol; // 
 extern  u8 bPowerOnAmpMuteTimer; // 

#endif 


//----------------------------------------------
	
//*****BAT DETECT *****//
 #define SUPPORT_BAT_DETECT
#ifdef SUPPORT_BAT_DETECT
#define SUPPORT_BAT_DETECT_USE_IO
#define DETECT_EXTERNAL_BAT_USE_PB11
#define LED_SCREEN_SHOW_BAT_STATUS

//#define SUPPORT_BAT_DETECT_LOW_POWER_MODE_1
#define SUPPORT_BAT_DETECT_LOW_POWER_MODE_8
#define SUPPORT_LED_LOW_SHOW_LDO


#define SUPPORT_LOW_POWER_OFF_USE_EXTERN_IO_CONTROL
#ifdef SUPPORT_LOW_POWER_OFF_USE_EXTERN_IO_CONTROL
#define LOW_POWER_OFF_USE_EXTERN_IO_PB12
//#define SUPPORT_LOW_POWER_LED_SHOW_BAT_FLASH
#define SUPPORT_LOW_POWER_USE_LED_SHOW
#define SUPPORT_LOW_POWER_LED_ALL_SCREEN_FLASH
#define SUPPORT_LOW_POWER_DISABLE_MUTE_KEY
#define LOW_POER_VOLTOGE_VAL   19
extern u16 low_off_cnt;
extern volatile u8 low_power_flag;
#endif

//-----------AMP POWER ON OFF------------------------------------
//#define   AMP_POWER_ENABLE
#ifdef AMP_POWER_ENABLE   
        #define  AMP_POWER_IO_PA10
#endif 
//---------POWER ON  MUTE 1S  AND LOW POWER MUTE 5S--------------------
#define   AMP_MUTE_ENABLE
#ifdef AMP_MUTE_ENABLE   
#define  AMP_POWER_ON_MUTE_TIMER  3
#define  AMP_MUTE_IO_PA11
#endif 

#define  SUPPORT_LOW_POWER_DELAY_DET
#define  POWER_ON_LOW_POWER_DELAY_VAL	30

#endif

#define SUPPORT_RGB_LED_MODE
#ifdef SUPPORT_RGB_LED_MODE
#define SUPPORT_RGB_IO_PA9PA4PA10
#define SUPPORT_IO_ANGLOG_OUTPUT_PWM
#define SUPPORT_TIME3_AS_RGB
#define SUPPORT_USE_RGB_ALL_ANGLOG
#endif

#define SUPPORT_MODE_VOICE_ONOFF  // AUX MODE  LONG 3S  
#ifdef SUPPORT_MODE_VOICE_ONOFF		 
#define SUPPORT_DEFAULT_VOCE_ON
extern u8 bModeVoiceOnoffFlag;	
#endif



