

/*
GPIO SET:
	IR-PA11 	LED-PC5	
	AUX0-PA1PA2	 AUX1-PA3PA3				MIC-PB13	MUTE-PA11
	(预留PA10-DATA PA9-CLK PR1-STB)	M1-5:PB12-PB9PB3
	SD1-PB0_DAT_PB1_CMD_PB2_CLK	
	LED7PIN-PC4_1PA15PA14PC0
*/

/*
*********************************************************************
                        DEBUG MACRO
*********************************************************************
*/

 //#define __DEBUG         ////<开启系统打印调试功能

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
#define SYS_LVD_EN              0       ///<电量监测
#define KEY_TONE_EN     	    1       ///<按键音
#define UART_UPDATA_EN          0      ///<串口升级
#ifdef  MINI
    #define KEY_DOUBLE_CLICK                ///<按键双击功能
#endif

//=================================================
// 1-VOL SET JSUO
#define SYS_DEFAULT_VOL        30      ///<非0表示使用默认音量
#if  SYS_DEFAULT_VOL
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
    #define FM_RADIO_EN         0       ///<标准SDK
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

    #define REC_EN             0   ///<标准SDK录音功能

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

//#define SUPPORT_NO_USE_PANEL
   #define UI_ENABLE                1     ///<UI_显示
 

#if  UI_ENABLE
    ///二选一
    #define LCD_128X64_EN         0      ///<lcd 支持
    #define LED_7_EN                       1      ///<led 支持     
    #define LCD_SUPPORT_MENU     0       ///菜单显示
    #define LRC_LYRICS_EN                 0       ///LRC歌词显示




#define SUPPORT_PT6312_MODE
#ifdef SUPPORT_PT6312_MODE
//#define TEST_PANEL_LED
//#define TEST_SHOW_ALL

#define TM1628_MODE 0x03
//#define TM1628_DATA_CLK_STB_USE_PC4PC3PC2
#define TM1628_DATA_CLK_STB_USE_PC2PC3PC4
//#define TM1628_DATA_CLK_STB_USE_PA10PA9PR1
//#define TM1628_DATA_CLK_STB_USE_PR1PA9PA10


#define LED_ID_YDL2001_VFN_35
#ifdef LED_ID_YDL2001_VFN_35
#define LED_COM_MAX 10
#define LED_8NUMS 5
#define LED_8GRIDS 7
#define LED_ICON_NUMS 32
#define LED_EQ_NUMS  32
#endif


#define LED_SHOW_SCREEN_FREQUENCE
#ifdef LED_SHOW_SCREEN_FREQUENCE
#define SUPPORT_EQ_FREQUENCE
//#define LED_EQ_BAR_SHOW
//#define EQ_ADC_VALUE_DETECT  
#define SUPPORT_POWER_ON_FREQUENCE_DELAY_SHOW  
 extern u8 bFrequencePowerOnDelayVal;


 //#define EQ_DETECT_PIN_USE_PA9
//#define EQ_DETECT_PIN_USE_PA10
#define SUPPORT_EQ_FREQUENCE_TIMER_SCAN


#define support_led_rgy_running
#define support_be_mode_off_half_sec


#endif

#endif







//#define LED_7PIN_MODE
#ifdef LED_7PIN_MODE

#define support_led7_user_pc0pa14pa15pc1pc2pc3pc4
#define LED_7PIN_REVERSE
	

//#define LED_ID_4210TAB
#ifdef LED_ID_4210TAB
#define LED_COM_MAX 7
#define LED_8NUMS 4
#define LED_8GRIDS 7
#define LED_ICON_NUMS 7
#endif

//#define LED_ID_8848
#ifdef LED_ID_8848
#define LED_COM_MAX 7
#define LED_8NUMS 4
#define LED_8GRIDS 7
#define LED_ICON_NUMS 8
#endif


#endif

#define SUPPORT_FLASH_ICON
#define SUPPORT_FLASH_CHAR

#define SUPPORT_BT_LED_PRORAM


#ifdef SUPPORT_FLASH_CHAR
//#define SUPPORT_LED_BLUE_STRING_FLASH
#define SUPPORT_MUTE_FLASH
#define MUTE_FLASH_ALL_SCREEN
//#define MUTE_FLASH_ZERO_VOL
#endif
//#define SUPPORT_EQ_SHOW_CHAR
#define SUPPORT_EQ_ZY_JY_SHOW_CHAR
//#define LED_SHOW_DEVICE_PLUGIN
#define SUPPORT_BT_MODE_SHOW_ICON_PLAY_PAUSE

//#define support_aux_led_disp_line     //屏上显示"LINE"
#define support_led_disp_pause
#define support_mute_led_all_flish    //MUTE闪屏
#define SUPPORT_NO_SHOW_LOAD	
 #endif 


//*********************************************************************
 

//-------ADKEY--------------------------------------------------------


#define muteh_key_mode   1

#if muteh_key_mode
#define KEY_AD_EN       0   ///<AD按键使能
#define KEY_IO_EN       1   ///<IO按键使能
#define KEY_IR_EN       1   ///<红外遥控使能,PA9
#define KEY_TCH_EN      0   ///<触摸按键使能
#endif 






#ifdef KEY_IO_EN
//#define support_io_scan_user_PB8PB9PB11

#define support_io_scan_user_PB12PB11PB10PB9PB3
#define io_scnkey_jl2017bt_ydl2001_mode
#define adk_user_9_key_mode

#endif 



#if KEY_AD_EN  
#define adk_user_pr1

// 7k5-bt
//#define adk_22k_15key_mode_zy_jy_308d

//--------------------------
#define support_bt_key_mode


#define ADK_USE_15KEY_MODE

#ifdef ADK_USE_15KEY_MODE
#define adkey_max_num	15

//  pp-0r     eq-15k next-24k prev -36k  mode-51k repeat-91k stop 150k 220k-power

#define PULL_UP_RESISTANCE 10   //上拉电阻10K
#define KEYE_RES 0  //PP
#define KEYD_RES 1 // 
#define KEYC_RES 2  // 
#define KEYB_RES 3.3  // 
#define KEYA_RES 5.1  //
#define KEY9_RES 7.5 //
#define KEY8_RES 10 //   
#define KEY7_RES 15  // EQ
#define KEY6_RES 24  //next/v+
#define KEY5_RES 36  //prev/v-
#define KEY4_RES 51 //mode
#define KEY3_RES 91 //REPEAT
#define KEY2_RES 150 //STOP
#define KEY1_RES 220  //STANDBY
#define KEY0_NOKEY_RES 330

#endif

//#define adk_user_6_key_mode_and_power
//#define adk_use_22k_6key_mode
#ifdef adk_use_22k_6key_mode
//#define LONG_PP_KEY_AS_FM_SCAN
//#define SUPOORT_LONG_PP_AS_BT_CALL_REJECT
//#define SUPOORT_LONG_MODE_AS_BT_CONNECT_CTL
//#define SUPOORT_LONG_PP_AS_BT_CONNECT_CTL
#endif
#endif 


//-------IR---------------------------------------

#if KEY_IR_EN         ///<红外遥控使能,PA9
#define  IR_IO_PA_X     11  // PA11
//#define  IR_IO_PB_X     3  // PB3
//#define  IR_IO_PC_X     14  // PC14
//#define  IR_IO_PD_X     14  // PD14

//#define IR_USE_DOUBLE_TY_00FF_807F_22KEY
//#define IR_USE_DOUBLE_ZY2119_00FF_807F_22KEY
//#define  ir_max_num     22
//#define  IR_NUMS     2


#define IR_JY_DOUBLE_00FF_807F_26KEY
#define  ir_max_num     27
#define IR_NUMS 2


#define   support_msg_stop_key 
#define   SUPPORT_IR_USE_SINGLE_MSG 
#define   support_stanby_mod 
#endif 

//-------LED---------------------------------------

#define LED_EN                  1       ///<LED指示使能
//#define  PP_LED_USER_PB9
#define  PP_LED_USER_PC5

//-------MUTE---------------------------------------
//MUTE IO 
 //   #define   PA_ENABLE
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

//-------POWER ON SET ---------------------------------------
//#define  power_on_no_usb_sd_to_bt 
#define  power_on_no_usb_sd_to_line   
//#define  usb_sd_out_to_bt_mode   
#define  usb_sd_out_to_line_mode   



//-------GENERAL  SET----------------------------------
#define SUPPORT_BT_AUTO_PLAY	1  // BT MUSIC AUTO PLAY
#define SUPPORT_OFF_PHONE_CALL   1 // OFF PHONE 
#define SUPPORT_OFF_MODE_VOICE     // OFF MUSIC LINE FM MODE VOICE
#define SUPPORT_REPEAT_ONLY_USE_ONE_ALL


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

#define  SUPPORT_MULTI_INPUT_MODE
#ifdef SUPPORT_MULTI_INPUT_MODE
extern u8 bMultiExternInputSel;
#define   line2_user_PA3PA4  //AUX1
#define   SUPPOR_INPUT_AUX1  //AUX1
#define  SUPPORT_AUX1_SHOW_DVD
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



