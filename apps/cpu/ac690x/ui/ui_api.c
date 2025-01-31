#include "sdk_cfg.h"
#include "ui/ui_api.h"
#include "ui/led/led7_drv.h"
#include "ui/lcd/lcd_drv_api.h"
#include "ui/lcd/lcd_disp.h"
#include "key_drv/key.h"
#include "common/msg.h"
#include "common/app_cfg.h"
#include "rtc_setting.h"
#include "dac/dac_api.h"
#include "ui/lcd/lcd_ui_menu.h"
#include "common/common.h"
#ifdef SUPPORT_PT2313
#include "dev_manage\pt2313.h"
#endif

UI_VAR UI_var;   //UI 显示变量

#if UI_ENABLE

UI_DISP_API * ui_disp_api;

#if LED_7_EN
static const UI_DISP_API ui_led7 =
{
    ///--------common------///
    .ui_string_menu = led7_show_string_menu,
    .ui_IR_number = led7_show_IR_number,
    .ui_vol_set = led7_show_volume,
    .ui_clear =led7_clear_icon,

    ///--------music------///
    .ui_music_main =led7_show_music_main,
    .ui_eq_set = led7_show_eq,
    .ui_file_num =led7_show_filenumber,
    .ui_play_mode =led7_show_playmode,

    ///--------fm------///
    .ui_FM_main = led7_show_fm_main,
    .ui_FM_channel=led7_show_fm_station,

    ///--------rtc------///
    .ui_RTC_main=led7_show_RTC_main,
    .ui_RTC_set=led7_show_RTC_main,
    .ui_ALM_set=led7_show_alarm,

    ///--------rec------///
#if REC_EN
    .ui_REC_main=led7_show_rec_main,
#endif
    ///--------aux------///
    .ui_AUX_main=led7_show_linin_main,

    ///--------bt------///
    .ui_BT_main=led7_show_string_menu,

    ///--------pc------///
    .ui_PC_main=led7_show_string_menu,

    
    .ui_menu_list  = (void *)led7_show_string_menu,
#if defined( support_line_pause_disp_pause)  ||defined(support_fm_pause_disp_pause)
    .ui_aux_fm_pause=led7_show_linin_fm_pause,
#endif

//------------------customer option  add---------------------
#ifdef LED_SHOW_DEVICE_PLUGIN	 
    .ui_menudevice_main = led7_show_device_main,
#endif

#ifdef support_menu_mode
    .ui_menuvol_main = led7_show_vol_menu_main,
#endif
#ifdef SUPPORT_RGB_LED_MODE
    .ui_menurgb_main = led7_show_rgb_led_mode_main,
#endif


#ifdef SUPPORT_MUSIC_FOLDER_OPERATION
    .ui_folder_show_main = led7_folder_show_main,
#endif






//------------------customer option  add---------------------

};
#endif

#if LCD_128X64_EN
static const UI_DISP_API ui_lcd12864 =
{
    ///--------common------///
    .ui_string_menu = lcd_disp_string_menu,
    .ui_IR_number = lcd_dis_IR_number,
    .ui_vol_set = lcd_dis_vol,
    .ui_clear =lcd_clear,

    ///--------music------///
    .ui_music_main =lcd_music_main,
    .ui_eq_set = (void *)lcd_music_main,
    .ui_file_num =lcd_music_main,
    .ui_play_mode =(void *)lcd_music_main,

    ///--------fm------///
    .ui_FM_main = (void *)lcd_fm_main,
    .ui_FM_channel=(void *)lcd_fm_main,

    ///--------rtc------///
    .ui_RTC_main=lcd_rtc_main,
    .ui_RTC_set=lcd_rtc_set,
    .ui_ALM_set=lcd_alarm_set,

    ///--------rec------///
    .ui_REC_main=lcd_rec_main,

    ///--------aux------///
    .ui_AUX_main=lcd_aux_main,

    ///--------bt------///
    .ui_BT_main=lcd_bt_main,

    ///--------pc------///
    .ui_PC_main=lcd_pc_main,

#if LCD_SUPPORT_MENU
    /*menu list*/
    .ui_menu_list  = enter_menulist_display,
#else
    /*menu list*/
    .ui_menu_list  = (void *)lcd_clear,
#endif
};
#endif

static void Task_UI(void *p_arg)
{
    //void UI_menu_api(u8 menu)
    int msg[6];
    u32 err;
//    p_arg = p_arg;

    while(1)
    {
        //puts("ui pend...\n");
        //puts("p_in\n");
        err = os_taskq_pend(50, ARRAY_SIZE(msg), msg);//500ms_reflash

        //puts("p_out\n");

//        if((flag & 7) == 0)
//        {
//            GET_SYS_MEM_STAT();
//        }
//        flag++;

        if(err == OS_TIMEOUT)//time_out
        {
            ui_menu_api(MENU_MAIN,UI_NO_ARG);
            continue;
        }

//        printf("ui msg = %x,%x,%x\n",msg[0],msg[1],msg[2]);

        switch(msg[0])//action
        {
        case UI_MSG_NON:
            break;

        case UI_MSG_DIS_MAIN:
            UI_var.bMainMenu = msg[1];
            ui_menu_api(MENU_MAIN,UI_NO_ARG);
            break;

        case UI_MSG_REFLASH:
            ui_menu_api(msg[1],UI_NO_ARG);
            break;

        case UI_MSG_OTHER:
            ui_menu_api(msg[1],msg[2]);
            break;

        case UI_MSG_MENULIST:
        default:
            ui_menu_api(msg[1],UI_NO_ARG);
            break;
        }
    }
}

static void ui_task_init(void)
{
    puts("--------ui_init--------\n");

    os_task_create_ext(Task_UI,
                       (void*)0,
                       TaskUIPrio,
                       30,
#if OS_TIME_SLICE_EN > 0
                       1,
#endif
                       UI_TASK_NAME,
                       UI_TASK_STACK_SIZE
                      );

    /*
        os_task_create(Task_UI,
                   (void*)0,
                   TaskUIPrio,
                   30,
    #if OS_TIME_SLICE_EN > 0
                   1,
    #endif
                   UI_TASK_NAME
                  );
    */

}

//主界面刷新显示，显示参数存在 UI_var.ui_buf_adr
u32 ui_menu_reflash(u8 menu)
{
    u32 err;
    err = os_taskq_post(UI_TASK_NAME, 2, UI_MSG_REFLASH, menu);
    return err;
}

//非主界面刷新显示，需要参数，如音量
u32 ui_menu_arg_do(u8 menu,u32 arg)
{
    u32 err;
    err = os_taskq_post(UI_TASK_NAME, 3, UI_MSG_OTHER, menu, arg);
    return err;
}

//主界面刷新显示,如果当前界面不是互斥界面就刷新
u32 ui_menu_spec(u8 NewMenu,u32 CurMenu)
{
    u32 err = 0;
    if(UI_var.bCurMenu != CurMenu)
    {
        err = os_taskq_post(UI_TASK_NAME, 2, UI_MSG_REFLASH, NewMenu);
    }
    return err;
}

void ui_init_api(void)
{

#if LED_7_EN
    led7_init();
    ui_disp_api = (UI_DISP_API *)&ui_led7;
#endif

#if LCD_128X64_EN
    lcd_init();
    ui_disp_api = (UI_DISP_API*)&ui_lcd12864;
#endif

    ui_task_init();

#ifndef SUPPORT_NO_SHOW_LOAD    
	SET_UI_MAIN(MENU_WAIT);
#endif
    SET_UI_BUF_ADR(NULL);
    SET_UI_BUF_LEN(0);
    SET_UI_SYS_VOL(&dac_ctl.sys_vol_l);

#ifndef SUPPORT_NO_SHOW_LOAD    
UI_menu(MENU_WAIT);
#endif

    os_time_dly(5);//
}

/*----------------------------------------------------------------------------*/
/**@brief   UI 显示界面处理函数
   @param   menu：需要显示的界面
   @return  无
   @note    void UI_menu_api(u8 menu)
*/
/*----------------------------------------------------------------------------*/
void ui_menu_api(u8 menu, s32 arg)
{
    //printf("ui_api:main = 0x%x,cur = 0x%x,menu = 0x%x\n",UI_var.bMainMenu,UI_var.bCurMenu,menu);
    UI_var.param_menu = menu;
    /*界面属性-非主界面自动返回*/
    if (menu == MENU_MAIN)
    {
        if (UI_var.bMenuReturnCnt < UI_RETURN)
        {
            UI_var.bMenuReturnCnt++;
            if (UI_var.bMenuReturnCnt == UI_RETURN)
            {

                #ifdef support_menu_mode
				   menumode=0;
                #endif
			
//                ui_disp_api->ui_clear();
                if (UI_var.bCurMenu == MENU_INPUT_NUMBER)
                {
                    os_taskq_post_msg(keymsg_task_name, 1, MSG_INPUT_TIMEOUT);
                }
                else
                {
                    UI_var.bCurMenu = UI_var.bMainMenu;
                }
            }
            else
            {
                return;//非主界面，被动刷新
            }
        }
        else
        {
            /*等待界面不重复刷新界面*/
            if (UI_var.bCurMenu == UI_var.bMainMenu)
            {
                return;
            }
            UI_var.bCurMenu = UI_var.bMainMenu;
        }
    }
    else
    {
        if (menu >= MENU_200MS_REFRESH)    //仅在当前界面为主界面时刷新界面,例如：在主界面刷新播放时间
        {
            if (UI_var.bCurMenu != UI_var.bMainMenu)
            {
                return;
            }
        }
        else
        {
//            ui_disp_api->ui_clear();
            /*非主界面需要启动返回计数器*/
            if (menu != UI_var.bMainMenu)
            {
                UI_var.bMenuReturnCnt = 0;
            }
            UI_var.bCurMenu = menu;
            if (menu != MENU_INPUT_NUMBER)
            {
                input_number = 0;
                input_number_cnt= 0;
            }
        }
    }

#if LED_7_EN
    led7_setX(0);
#endif

    ui_disp_api->ui_clear();

//printf("-----UI_var.bCurMenu = %x\n",UI_var.bCurMenu);

    switch(UI_var.bCurMenu)
    {
    /*-----System Power On UI*/
    case MENU_POWER_UP:
    case MENU_WAIT:
#ifdef support_stanby_mod
    case MENU_STANBY_MAIN:
    case MENU_STANBY_OFF:
#endif

#if RTC_ALM_EN
    case MENU_ALM_UP:
#endif

#if 1//REC_EN
    case MENU_REC:
    case MENU_REC_DEL:
    case MENU_REC_END:
#endif  
    case MENU_ON:
    case MENU_OFF:
    case MENU_VOICE_CH:
    case MENU_VOICE_EN:

        ui_disp_api->ui_string_menu(UI_var.bCurMenu);
        break;

    case MENU_PC_MAIN:
    case MENU_PC_VOL_UP:
    case MENU_PC_VOL_DOWN:
        ui_disp_api->ui_PC_main(UI_var.bCurMenu);
        break;

    case MENU_BT_MAIN:
        ui_disp_api->ui_BT_main(UI_var.bCurMenu);
        break;

    case MENU_AUX_MAIN:
	 #ifdef SUPPORT_MP3_IDEL_MODE_IN_LINEIN_MODE
	 	case MENU_MP3_IDLE_MAIN:
	 #endif
	 #ifdef SUPPORT_AC3_MODE_IN_LINEIN_MODE
	 	case MENU_AC3_MAIN: 
	 #endif
	 #ifdef SUPPORT_DVD_MODE_IN_LINEIN_MODE
	 	case MENU_DVD_MAIN: 
	 #endif
	 #ifdef SUPPORT_CD_MODE_IN_LINEIN_MODE
	 	case MENU_CD_MAIN: 
	 #endif	


        ui_disp_api->ui_AUX_main(UI_var.bCurMenu);
        break;

    /*-----Common Info UI*/
    case MENU_MAIN_VOL:
        ui_disp_api->ui_vol_set((s32)*UI_var.sys_vol);
        break;
		


  #ifdef LED_SHOW_DEVICE_PLUGIN	 
     case MENU_DEVICE:
        ui_disp_api->ui_menudevice_main();
        break;
#endif

#ifdef support_menu_mode
    case MENU_VOL_MENU:
        ui_disp_api->ui_menuvol_main();
        break;
#endif

    case MENU_INPUT_NUMBER:
        ui_disp_api->ui_IR_number(arg);
        //led7_show_IR_number();
        break;

    /*-----Music Related UI*/
    case MENU_MUSIC_MAIN:
        ui_disp_api->ui_music_main();
        //led7_show_music_main();
        break;
    case MENU_MUSIC_PAUSE:
        ui_disp_api->ui_music_main();
        break;
    case MENU_FILENUM:
        ui_disp_api->ui_file_num();
        //led7_show_filenumber();
        break;
    case MENU_SET_EQ:
        ui_disp_api->ui_eq_set(arg);
        //led7_show_eq();
        break;

    case MENU_SET_PLAY_MODE:
        ui_disp_api->ui_play_mode(arg);
        //led7_show_playmode();
        break;

    /*-----FM Related UI*/
    case MENU_FM_MAIN:
    case MENU_FM_DISP_FRE:
        puts("--fm_dis_main---\n");
        ui_disp_api->ui_FM_main();
        //led7_show_fm_main();
        break;
    case MENU_FM_FIND_STATION:
    case MENU_FM_CHANNEL:
        ui_disp_api->ui_FM_channel();
        //led7_show_fm_station();
        break;

#if defined( support_line_pause_disp_pause)  ||defined(support_fm_pause_disp_pause)
    case MENU_AUX_PAUSE:
        ui_disp_api->ui_aux_fm_pause();
#endif
#if RTC_CLK_EN
    case MENU_RTC_MAIN:
    //RTC_setting_var.bMode = 0;    //模式与界面同步返回
    case MENU_RTC_SET:
        ui_disp_api->ui_RTC_main();
        //led7_show_RTC_main();
        break;

#if RTC_ALM_EN
    case MENU_ALM_SET:
        ui_disp_api->ui_ALM_set();
        //led7_show_alarm();
        break;
#endif/*RTC_ALM_EN*/



#if _MENU_LIST_SUPPORT_
    case MENU_LIST_DISPLAY:
        ui_disp_api->ui_menu_list(arg);
        break;
#endif


#endif/*RTC_CLK_EN*/

#if REC_EN
    case MENU_RECODE_MAIN:
        ui_disp_api->ui_REC_main();
        //led7_show_rec_main();
        break;
#endif/*REC_EN*/

    case MENU_RECODE_ERR:
        break;
#ifdef SUPPORT_RGB_LED_MODE
    case MENU_RGB_MODE:
          ui_disp_api->ui_menurgb_main();
      break;
#endif


#ifdef SUPPORT_MUSIC_FOLDER_OPERATION
	case MENU_PREV_FOLDER:
		ui_disp_api->ui_folder_show_main();
	break;
#endif






    default:
        break;
    }

#if LCD_128X64_EN
    draw_lcd_buf();
#endif
}
#endif
