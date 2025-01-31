#include "sdk_cfg.h"
#include "common/app_cfg.h"
#include "common/msg.h"
#include "common/includes.h"
#include "common/common.h"
#include "rtos/task_manage.h"
#include "play_sel/play_sel.h"
#include "sdmmc/sd_host_api.h"
#include "encode/dev_write_api.h"
#include "encode/mp2_encode_api.h"
#include "encode/encode.h"
#include "record.h"
#include "dac/dac_api.h"
#include "ui/ui_api.h"
#include "rec_key.h"
#include "sys_detect.h"
#include "rec_ui.h"
#include "key_drv/key.h"
#include "led7_drv.h"
#include "clock_api.h"
#include "clock.h"
#include "led.h"
#include "dac/dac.h"
/* #include "music_ui.h" */
/* #include "music.h" */
/* #include "linein.h" */

#define BT_REC_SR		8000L		//不能修改
#define FM_REC_SR		32000L		//
#define AUX_REC_SR		48000L
#define MIX_REC_SR		48000L

extern void *malloc_fun(void *ptr,u32 len,char *info);
extern void free_fun(void **ptr);
extern volatile u8 new_lg_dev_let;

#if REC_EN

#ifdef support_rec_stop_to_music
#ifdef support_rec_line_fm_rec_return_line_fm
 u8   play_rec_last_file;   
#else 
  bool  play_rec_last_file;   
#endif 
#endif 



#ifdef SUPPORT_SCG_VOICE
	bool bVoiceOnoff;
#endif







#define REC_FILE    1  //1：录MP3文件       0：录WAV文件

const u16 enc_bitrate_h[]=
{
    ///>(44100), (48000), (32000) 采样率可配置以下比特率
    32,48,56,64,80,96,112,128
};
const u16 enc_bitrate_l[]=
{
    ///>(44.1K/2,/4),  (48k/2,/4),  (32K/2,/4) 采样率可配置以下比特率
    8,16,24,32,40,48,56,64,80,96,112,128
};

const char rec_folder_name[] = "/JL_REC";                   //录音文件夹  //仅支持一层目录

#if REC_FILE
const char rec_file_name[] =   "/JL_REC/FILE0000.MP3";      //MP3录音文件名（含路径）
#else
const char rec_file_name[] =   "/JL_REC/FILE0000.WAV";      //ADPCM录音文件名（含路径）
#endif

u32 rec_fname_cnt;       //录音文件名计数
u32 rec_total_number;    //录音文件数计数

REC_CTL *g_rec_ctl;
RECORD_OP_API *rec_mic_api;

/* #if REC_BT_EN */
/* s16 *g_bt_buf_ptr; */
/* #endif */

extern ENC_OPS *get_mp2_ops();
extern ENC_OPS *get_enadpcm_ops();
/* extern ENC_OPS_t *get_mp2_opst(); */
/* extern ENC_OPS_t *get_enadpcm_opst(); */

ENC_OPS *(*enc_ops_list[MAX_FORMAT])(void) AT(.const_tab);
ENC_OPS *(*enc_ops_list[MAX_FORMAT])(void) =
{
    NULL,       //标记

#if ENCODE_MP2_EN
    get_mp2_ops,
#endif

#if ENCODE_ADPCM_EN
    get_enadpcm_ops,
#endif
};

/* ENC_OPS_t *(*enc_ops_list_t[MAX_FORMAT])(void) AT(.const_tab); */
/* ENC_OPS_t *(*enc_ops_list_t[MAX_FORMAT])(void) = */
/* { */
/* NULL,       //标记 */

/* #if ENCODE_MP2_EN */
/* get_mp2_opst, */
/* #endif */

/* #if ENCODE_ADPCM_EN */
/* get_enadpcm_opst, */
/* #endif */
/* }; */

#if REC_BT_EN
/*----------------------------------------------------------------------------*/
/**@brief  蓝牙录音数据源获取
   @param  无
   @return 无
   @note   void bt_rec_get_data(s16 *buf,u32 len,)//录音获取pcm数据函数
*/
/*----------------------------------------------------------------------------*/
void bt_rec_get_data(s16 *buf, u32 len)//录音获取pcm数据函数
{

    u8 bt_rec_buf[0x40];//dac_buf_len
    s16 *dp = bt_rec_buf;
    u32 rec_point;
    if((rec_bt_api != NULL) && (rec_bt_api->rec_ctl != NULL))
    {
        if(rec_bt_api->rec_ctl->enable == ENC_STAR)
        {
            for(rec_point = 0; rec_point < 0x20; rec_point++)
            {
                dp[rec_point] = buf[rec_point*2];//all left channel data
            }
            rec_bt_api->rec_ctl->ladc.save_ladc_buf(&rec_bt_api->rec_ctl->ladc, dp, 0, 0x40);

            for(rec_point = 0; rec_point < 0x20; rec_point++)
            {
                dp[rec_point] = buf[rec_point*2+1];//all right channel data
            }
            rec_bt_api->rec_ctl->ladc.save_ladc_buf(&rec_bt_api->rec_ctl->ladc, dp, 1, 0x40);
        }
    }
}
#endif


/*----------------------------------------------------------------------------*/
/**@brief  录音通道初始化、文件初始化
   @param  mode: 录制通道
   @return 无
   @note   RECORD_OP_API *rec_init(u8 mode)
*/
/*----------------------------------------------------------------------------*/
static RECORD_OP_API *rec_init(u8 mode)
{
    s16 ret;
    u32 rec_file_num;
    RECORD_OP_API *rec_op_api = NULL;

    rec_op_api = malloc_fun(rec_op_api, sizeof(RECORD_OP_API), 0);
    ASSERT(rec_op_api);

    rec_op_api->fop_api = enc_fop_api_init();   //文件操作信息初始化
    // // printf("enc_fop_api_init %x------\n", rec_op_api->fop_api);

    rec_op_api->fw_api = fw_api_init();     //读写接口初始化
    // printf("------fw_api_init %x------\n", rec_op_api->fw_api);

    // // puts("encode_fs_open-----\n");
    ret = encode_fs_open(rec_op_api, DEV_SEL_SPEC, new_lg_dev_let, (char *)rec_folder_name, FA_CREATE_NEW);  //初始化文件系统
    // printf("--encode_fs_open ret = %x  \n", ret);

    if(-FILE_OP_ERR_LGDEV_NO_FIND == ret)
    {
        // printf("--DEV_SEL_FIRST--\n");
        ret = encode_fs_open(rec_op_api, DEV_SEL_FIRST, new_lg_dev_let, (char *)rec_folder_name, FA_CREATE_NEW);  //初始化文件系统
    }

    if(ret == FR_EXIST)
    {
#if REC_FILE
        encode_get_fileinfo(rec_op_api,(char *)rec_folder_name,"MP3",&rec_file_num, &rec_total_number);
#else
        encode_get_fileinfo(rec_op_api,(char *)rec_folder_name,"WAV",&rec_file_num, &rec_total_number);
#endif
        // printf("rec_total_number = %d\n",rec_total_number);
        // printf("rec_file_num = %d\n",rec_file_num);
        rec_fname_cnt = rec_file_num+1;

        if((rec_fname_cnt >= 9999)&&(rec_total_number >= 9999))//以超出可使用文件名数
        {
            goto _err_exit;
        }
    }
    else if(ret != FR_OK)//初始化失败，释放资源
    {
        // printf("rec_folder_creater fail,ret = %d\n",ret);
        goto _err_exit;
    }
    else
    {
        // printf("rec_folder_creater succ\n");
        rec_fname_cnt = 0;       //文件夹不存在，文件数清零
    }

    // puts("enc_run_creat----\n");
    enc_run_creat();        //创建录音线程

    // puts("rec_enc_init----\n");
    if(mode == ENC_MIC_CHANNEL)
    {
#if REC_FILE
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, enc_bitrate_h[7], LADC_SR48000, MP2_FORMAT); //申请录音资源并启动录音
#else
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, 1024, LADC_SR48000, ADPCM_FORMAT); //申请录音资源并启动录音
#endif
    }
    else if(mode == ENC_FM_CHANNEL)
    {
        ///FM录音必须32K采样率
#if REC_FILE
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, enc_bitrate_h[7], LADC_SR32000, MP2_FORMAT); //申请录音资源并启动录音
#else
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, 1024, LADC_SR32000, ADPCM_FORMAT); //申请录音资源并启动录音
#endif
    }
    else if(mode == ENC_LINE_LR_CHANNEL)
    {
        // // printf("-------ENC_ENC_LINE_LR_CHANNEL\n");
#if REC_FILE
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, enc_bitrate_h[7], LADC_SR48000, MP2_FORMAT); //申请录音资源并启动录音
#else
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, 1024, LADC_SR48000, ADPCM_FORMAT); //申请录音资源并启动录音
#endif
    }
    else if(mode == ENC_BT_CHANNEL)
    {
        ///蓝牙通话录音必须8K采样率
#if REC_FILE
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, enc_bitrate_l[5], LADC_SR8000, MP2_FORMAT); //申请录音资源并启动录音
#else
        rec_op_api->rec_ctl = init_enc(rec_op_api, mode, 1024, LADC_SR8000, ADPCM_FORMAT); //申请录音资源并启动录音
#endif
    }
    g_rec_ctl = rec_op_api->rec_ctl;

    // printf("----rec_op_api %x----\n",rec_op_api);
    return rec_op_api;

_err_exit:
    // printf("----err_exit----\n");
    encode_fs_close(rec_op_api);
    // printf("----encode_fs_close----\n");
    free_fop(rec_op_api);
    // printf("----free_fop----\n");
    free(rec_op_api);
    // printf("----return NULL;----\n");
    return NULL;
}

static void start_rec(RECORD_OP_API ** rec_api)
{
    RECORD_OP_API * rec_op_api;
    s16 ret;
    u16 err_cnt;
    u32 tmp;
    char rec_file_name_temp[sizeof(rec_file_name)];

    if(!rec_api)
        return;

    memcpy(rec_file_name_temp, rec_file_name, sizeof(rec_file_name));

    rec_op_api = *rec_api;
    err_cnt = 0;    //err_reset

_get_file_name:
    if(rec_fname_cnt > 9999)
    {
        rec_fname_cnt = 0;
    }

    tmp = rec_fname_cnt/1000;
    rec_file_name_temp[12] = (char)(tmp+'0');

    tmp = rec_fname_cnt%1000/100;
    rec_file_name_temp[13] = (char)(tmp+'0');

    tmp = rec_fname_cnt%100/10;
    rec_file_name_temp[14] = (char)(tmp+'0');

    tmp = rec_fname_cnt%10;
    rec_file_name_temp[15] = (char)(tmp+'0');

    // printf("--encode_file_open %s--\n",rec_file_name_temp);
    ret = encode_file_open(rec_op_api,rec_file_name_temp,FA_CREATE_NEW|FA_WRITE);  //初始化文件
    if(FR_OK == ret)//文件创建成功
    {
    
        rec_op_api->rec_ctl->curr_device = (u8)file_operate_ctl(FOP_GET_PHYDEV_INFO,rec_op_api->fop_api,0,0);
//        music_ui.ui_curr_device = (u32)file_operate_ctl(FOP_GET_PHYDEV_INFO,rec_op_api->fop_api,0,0);
//            os_time_dly(5);
//        // printf("--start_rec-- device %d--- \n",music_ui.ui_curr_device);
            os_time_dly(50);
        rec_op_api->rec_ctl->file_info.file_del_size = 1; ///0:不自动删除文件 , 非0:删除文件 删除成功自动归0
        os_taskq_post_msg(ENC_RUN_TASK_NAME, 1, rec_op_api); //启动录音
            os_time_dly(50);

        while(rec_op_api->rec_ctl->enable != ENC_STAR)//等待录音启动线程
        {
            os_time_dly(1);
        }
        UI_menu(MENU_REFRESH);
    }
    else if(FR_EXIST == ret)////文件已经存在
    {
        // printf("--%s__FR_EXIST--\n", rec_file_name_temp);

        err_cnt++;
        if(err_cnt >= 9999)
        {
            // printf("file_open err 9999\n");
            exit_rec_api(rec_api); //停止录音和释放资源
        }

        rec_fname_cnt++;
        goto _get_file_name;
    }
    else//其他错误
    {
        // printf("file_open ret :%x\n",ret);
        exit_rec_api(rec_api); //停止录音和释放资源
    }
}


/*----------------------------------------------------------------------------*/
/**@brief  RECORD 消息处理流程
   @param  rec_api_p：record 参数指针
   @param  msg：record 消息
   @return 无
   @note   void rec_msg_deal(RECORD_OP_API **rec_api_p, int *msg)
*/
/*----------------------------------------------------------------------------*/
void rec_msg_deal(RECORD_OP_API **rec_api_p, int *msg)
{
    switch(msg[0])
    {
    case MSG_REC_START:
        puts("MSG_REC_START\n");

	{
		         if(*rec_api_p == NULL)
		         {
		             // printf("curr_task_name %s\n", curr_task->name);
		 
		             if(unlikely(0))
		                 ;
		 #if MIC_REC_EN
		             else if(compare_task_name(REC_TASK_NAME))
		                 (*rec_api_p) = rec_init(ENC_MIC_CHANNEL);
		 #endif
		 
		 #if FM_REC_EN
		             else if(compare_task_name(FM_TASK_NAME))
		             {
		                 set_sys_freq(FM_REC_SYS_Hz);
		                 (*rec_api_p) = rec_init(ENC_FM_CHANNEL);
		             }
		 #endif
		 
		 #if AUX_REC_EN
		             else if(compare_task_name(LINEIN_TASK_NAME))
		             	{
		 			 #ifdef 	line_user_dacr				 	
		  		                 (*rec_api_p) = rec_init(ENC_DAC_RIGHT_CHANNEL);
		 		 	#elif defined line_user_dacl
		  		                 (*rec_api_p) = rec_init(ENC_DAC_LEFT_CHANNEL);		 
		 			 #else
		  		                 (*rec_api_p) = rec_init(ENC_LINE_LR_CHANNEL);			 
		 			 #endif
		             	}
		 #endif
		 
		 #if BT_REC_EN
		             else if(compare_task_name("BtStackTask"))
		                 (*rec_api_p) = rec_init(ENC_BT_CHANNEL);
		 #endif
		 
		             else
		                 (*rec_api_p) = NULL;//err
		         }
		 
		         if(*rec_api_p)
		         {
		             if((*rec_api_p)->rec_ctl->enable == ENC_STOP)
		             {


				
//				cbuf_write(&rec_mic_api,0x00,1024);
			
	                 puts("start_rec\n");
		                 SET_UI_REC_OPT(UI_REC_OPT_START);
		                 start_rec(rec_api_p); //启动录音
		             }
		         }
				 
    	}	
        	UI_menu(MENU_REFRESH);
        break;


    case MSG_REC_STOP:
        // puts("REC_MSG_REC_STOP\n");
        if(*rec_api_p)
        {
            stop_rec(*rec_api_p); //停止录音和释放资源
            rec_fname_cnt++;//录音文件名序号
            (*rec_api_p)->rec_ctl->lost_frame.black_lost_frame = 0;
            (*rec_api_p)->rec_ctl->lost_frame.front_lost_frame = 0;
        }
//            UI_menu(MENU_MAIN);
	        SET_UI_REC_OPT(UI_REC_OPT_STOP);
	        UI_menu(MENU_REFRESH);
		
		 #ifdef support_fm_line_recing_disp_time
		 	if(compare_task_name("FMRadioTask")) //不是MIC录音显示REC 因为前面已经显示了
		 		{
		 		 SET_UI_MAIN(MENU_FM_MAIN);//显示REC
		 		  }else 
		 		  if(compare_task_name("LineinTask"))	//不是MIC录音显示REC 因为前面已经显示了
		 		{
		 		 SET_UI_MAIN(MENU_AUX_MAIN);//显示REC
		 		  } 	   
		 #endif 

	 #ifdef support_rec_stop_to_music
	 #ifdef support_only_mic_rec_stop_to_music
	   if(compare_task_name(REC_TASK_NAME))
	 #endif 
	    	{
	         play_rec_last_file=1;   
	         os_taskq_post("MainTask", 1, MSG_MUSIC_MODE_KEY);
	 	}
	 
	 #endif 
		
        break;

    case MSG_REC_PP:
        if(*rec_api_p)
        {
            if((*rec_api_p)->rec_ctl->enable == ENC_STAR)
            {
                (*rec_api_p)->rec_ctl->enable = ENC_PAUS;
                puts("enc_pause\n");
                SET_UI_REC_OPT(UI_REC_OPT_PAUSE);
            }
            else if((*rec_api_p)->rec_ctl->enable == ENC_PAUS)
            {
                (*rec_api_p)->rec_ctl->enable = ENC_STAR;
                puts("enc_star\n");
                SET_UI_REC_OPT(UI_REC_OPT_START);
            }
            else
            {
                // puts("MSG_ENCODE_PP_err\n");
            }
        }
        UI_menu(MENU_REFRESH);
        break;

    case MSG_ENCODE_ERR:
        // enc_ puts("REC_MSG_ENCODE_ERR\n");
        if(*rec_api_p)
        {
            exit_rec_api(rec_api_p); //停止录音和释放资源
        }
        SET_UI_REC_OPT(UI_REC_OPT_ERR);
        UI_menu(MENU_REFRESH);
        break;

    case SYS_EVENT_DEV_OFFLINE:
        // enc_ puts("enc_SYS_EVENT_DEV_OFFLINE--\n");
        if(*rec_api_p)
        {
            enc_puts("DEV_OFFLINE-00\n");
            if((*rec_api_p)->fop_api->cur_lgdev_info->lg_hdl->phydev_item == (void *)msg[1])
            {
                enc_puts("DEV_OFFLINE-01\n");

                /* if((*rec_api_p)->rec_ctl->enable != ENC_STOP) */
                /* { */
                /* (*rec_api_p)->rec_ctl->enable = ENC_STOP; */
                /* SET_UI_REC_OPT(UI_REC_OPT_ERR); */
                /* } */

                /* UI_menu(MENU_REFRESH); */

                exit_rec_api(rec_api_p); //停止录音和释放资源
		 #ifdef support_rec_stop_to_music    //  处理录音时拔出设备
		 		  if(compare_task_name(REC_TASK_NAME))
		 			{		
		 			os_taskq_post("MainTask", 1, MSG_MUSIC_MODE_KEY);
		 			 break;	
		 			}
		 					
		 #endif 		
		 
		 #ifdef support_fm_line_recing_disp_time
		 		if(compare_task_name("FMRadioTask")) //不是MIC录音显示REC 因为前面已经显示了
		 			{
		 			 SET_UI_MAIN(MENU_FM_MAIN);//显示REC
		 			  }else 
		 			  if(compare_task_name("LineinTask"))	//不是MIC录音显示REC 因为前面已经显示了
		 			{
		 			 SET_UI_MAIN(MENU_AUX_MAIN);//显示REC
		 			  } 	   
		 #endif 
            }
            else
            {
                ///<非解码设备，删除该逻辑设备
                enc_puts("DEV_OFFLINE-02\n");
                break;
            }
        }
        else
        {
            UI_menu(MENU_REFRESH);
        }

        break;

    default:
        break;
    }
}

void exit_rec_deal(void *p)
{
    exit_rec(p);
    set_sys_freq(SYS_Hz);
}

/*----------------------------------------------------------------------------*/
/**@brief  RECORD 控制任务
   @param  p：任务间参数传递指针
   @return 无
   @note   static void rec_task(void *p)
*/
/*----------------------------------------------------------------------------*/
u8 bRecFileNoiseCnt;

static void rec_task(void *p)
{
    int msg[3];
    u32 tone_flag = 0;
    u32 rec_time;

    puts("\n************************RECORD TASK********************\n");

	SET_UI_MAIN(MENU_RECODE_MAIN);//显示REC
	UI_menu(MENU_RECODE_MAIN);	


#ifdef support_off_mode_change_voice			
    os_taskq_post_msg(REC_TASK_NAME, 1, SYS_EVENT_PLAY_SEL_END); //触发AUX模块初始化
#elif defined SUPPORT_SCG_VOICE	
	if(bVoiceOnoff==1)	
		{
		     tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_MP3); //RECORD模式提示音播放
		}
	else
		{
		     os_taskq_post_msg(REC_TASK_NAME, 1, SYS_EVENT_PLAY_SEL_END); //触发AUX模块初始化
		}
	
#elif defined SUPPORT_DOUBLE_MODE_VOICE	
	if(!bModeVoiceSelFlag)
	    tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_EN_MP3); //RECORD模式提示音播放
	else				
	    tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_MP3); //RECORD模式提示音播放
#elif defined SUPPORT_MODE_VOICE_ONOFF	
	if(!bModeVoiceOnoffFlag)
	    tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_MP3); //RECORD模式提示音播放
	else				
	    tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_MP3); //RECORD模式提示音播放	
	    
#elif defined SUPPORT_MODE_VOICE_EN_CH_ONOFF	

 	if(bModeVoiceSelVal==0)
	     {
    			os_taskq_post_msg(REC_TASK_NAME, 1, SYS_EVENT_PLAY_SEL_END); //触发AUX模块初始化
	     }
	     else
		if(bModeVoiceSelVal==2)
		{
  			  tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_EN_MP3); //RECORD模式提示音播放
		}
		else
	     {
   		   tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_MP3); //RECORD模式提示音播放
	     }

	    
#else 
    tone_play_by_name(REC_TASK_NAME, 1,BPF_REC_MP3); //RECORD模式提示音播放
 #endif 	
 

    led_fre_set(5,0);

     workmode=REC_WORK_MODE;



#ifdef support_only_rec_key_to_rec_mode		
   muisc_to_rec_flag=0;
#endif 


    while(1)
    {
        memset(msg, 0x00, sizeof(msg));
        os_taskq_pend(0, ARRAY_SIZE(msg), msg);
        clear_wdt();
        switch(msg[0])
        {
        case SYS_EVENT_DEL_TASK:
            enc_puts("RECORD_SYS_EVENT_DEL_TASK\n");
            if (os_task_del_req_name(OS_TASK_SELF) == OS_TASK_DEL_REQ)
            {
//                UI_menu(MENU_WAIT);
                ui_close_rec();
                exit_rec(&rec_mic_api); //停止录音和释放资源
//                play_sel_stop();   //关闭提示音
	 	 #ifdef SUPPORT_REC_MODE_EXTERN_IO_VOLTOGE   
	 	 	SET_REC_EXTERN_IO(0);
	 	 #endif    
               os_time_dly(50);		 
                dac_channel_off(DAC_DIGITAL_AVOL, FADE_ON);
                os_task_del_res_name(OS_TASK_SELF);
            }
            break;

        case SYS_EVENT_PLAY_SEL_END: //提示音结束
            enc_puts("RECORD_SYS_EVENT_PLAY_SEL_END\n");
            play_sel_stop(); //关闭提示音

            tone_flag = 1;
        case MSG_REC_INIT:

	 #ifdef SUPPORT_REC_MODE_EXTERN_IO_VOLTOGE   
	 	SET_REC_EXTERN_IO(1);
	 #endif

              enc_puts("MSG_REC_INIT\n");
		dac_channel_on(DAC_DIGITAL_AVOL, FADE_ON);
		ui_open_rec(&rec_mic_api, sizeof(RECORD_OP_API**));
		SET_UI_MAIN(MENU_RECODE_MAIN);//显示REC
		UI_menu(MENU_RECODE_MAIN);	
				
	 #ifdef support_only_rec_key_to_rec_mode		
	              os_taskq_post_msg("RECTask", 1, MSG_REC_START);
	 #endif 	
	 
            break;

#if LCD_SUPPORT_MENU
        case MSG_ENTER_MENULIST:
            UI_menu_arg(MENU_LIST_DISPLAY,UI_MENU_LIST_ITEM);
            break;
#endif

        case MSG_HALF_SECOND:

            if(updata_enc_time(rec_mic_api))
            {
                UI_menu(MENU_HALF_SEC_REFRESH);
            }
			
            break;

        default:
            if(tone_flag)
                rec_msg_deal(&rec_mic_api, msg); //record 流程
            break;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief  ENCODE任务创建
   @param  priv：任务间参数传递指针
   @return 无
   @note   static void encode_task_init(void *priv)
*/
/*----------------------------------------------------------------------------*/
static void rec_task_init(void *priv)
{
    u32 err;

    //初始化ENCODE APP任务
    err = os_task_create_ext(rec_task,
                             (void*)0,
                             TaskEncodePrio,
                             30
#if OS_TIME_SLICE_EN > 0
                             ,1
#endif
                             ,REC_TASK_NAME
                             ,1024*4
                            );

    //按键映射
    if(OS_NO_ERR == err)
    {
	 #ifdef SUPPORT_TM1628_KEY_READ
	         key_msg_register(REC_TASK_NAME, rec_led_table,adkey_msg_rec_table, iokey_msg_rec_table, irff00_msg_rec_table, NULL);
	 #else    
	         key_msg_register(REC_TASK_NAME, adkey_msg_rec_table, iokey_msg_rec_table, irff00_msg_rec_table, NULL);
	 #endif
    }
}


/*----------------------------------------------------------------------------*/
/**@brief  ENCODE 任务删除退出
   @param  无
   @return 无
   @note   static void encode_task_exit(void)
*/
/*----------------------------------------------------------------------------*/
static void rec_task_exit(void)
{
    if (os_task_del_req(REC_TASK_NAME) != OS_TASK_NOT_EXIST)
    {
        os_taskq_post_event(REC_TASK_NAME, 1, SYS_EVENT_DEL_TASK);
        do
        {
            OSTimeDly(1);
        }
        while(os_task_del_req(REC_TASK_NAME) != OS_TASK_NOT_EXIST);
        enc_puts("del_encode_task: succ\n");
    }
}


/*----------------------------------------------------------------------------*/
/**@brief  ENCODE 任务信息
   @note   const struct task_info rec_task_info
*/
/*----------------------------------------------------------------------------*/
TASK_REGISTER(rec_task_info) =
{
    .name = REC_TASK_NAME,
    .init = rec_task_init,
    .exit = rec_task_exit,
};
#endif
