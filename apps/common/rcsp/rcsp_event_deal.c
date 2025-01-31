/*
 *********************************************************************************************************
 *                                                BC51
 *                                      Remote Control System Protocol (RCSP)
 *                                             CODE
 *
 *                          (c) Copyright 2015-2016, ZHUHAI JIELI
 *                                           All Rights Reserved
 *
 * File : *
 * By   : jamin.li
 * DATE : 8/20/2015 build this file
 *********************************************************************************************************
 */
#include "rcsp/rcsp_head.h"
#include "rcsp/rcsp_interface.h"
#include "dac/dac_api.h"
#include "dac/dac.h"
#include "fm/fm_radio.h"
#include "fm/fm_api.h"
#include "music/music_info.h"

#if SUPPORT_APP_RCSP_EN

//static task_frame_parm_t frame_buf;

static volatile u8 rcsp_com_run_flag;
static volatile u8 rcsp_deal_msg_flag;
static volatile u8 rcsp_send_data_state;
u8  rcsp_cur_task_status;
u8  rcsp_eq_sw_flag = 1;
u8  rcsp_conn_apk_type;   //0-not conn,1-spp, 2-ble
u8  rcsp_com_err_count;

u32 rcsp_com_report_flag;
u8 *rcsp_send_tmp_buf;
u8 *rcsp_send_BigData_buf;

const s8 user_eq_seg_gain_default[6][10] =
{
    {-2,0,2,4,-2,-2,0,0,4,4},///POP
    {0, 8, 8, 4, 0, 0, 0, 0, 2, 2},///ROCK
    {-2,0,0,2,2,0,0,0,4,4},///COUNTRY
    {0,0,0,4,4,4,0,2,3,4},///JAZZ
    {4, 2, 0,-3,-6,-6,-3, 0, 3, 5},///CLASSIC
    {0,0,0,0,0,0,0,0,0,0},///user
};

s8 user_eq_seg_gain[6][10] =
{
    {-2,0,2,4,-2,-2,0,0,4,4},///POP
    {0, 8, 8, 4, 0, 0, 0, 0, 2, 2},///ROCK
    {-2,0,0,2,2,0,0,0,4,4},///COUNTRY
    {0,0,0,4,4,4,0,2,3,4},///JAZZ
    {4, 2, 0,-3,-6,-6,-3, 0, 3, 5},///CLASSIC
    {0,0,0,0,0,0,0,0,0,0},///user
};

const u8 rcsp_vol_max_value = MAX_SYS_VOL_L;


void Post_appstart_state_to_rcsp(void);
void Post_appstart_config_to_rcsp(void);
void Post_Mode_EQ_config_to_rcsp(void);
void Post_Music_PlayMode_config_to_rcsp(void);
void Post_Common_FrameStatus_to_rcsp(void);
void rcsp_connect_current_task(char *task_name);

void Post_Udisk_StartInfo_to_rcsp(void);
void Post_Light_StartInfo_to_rcsp(void);
void Post_Linein_StartInfo_to_rcsp(void);
void Post_Music_StartInfo_to_rcsp(void);
void Post_Radio_StartInfo_to_rcsp(void);
void Post_Bt_StartInfo_to_rcsp(void);

static const char support_app_group[][16] =
{
    { RDEF_BTSTACK_TASK_NAME },
    { RDEF_MUSIC_TASK_NAME },
    { RDEF_LIGHT_TASK_NAME },
    { RDEF_RADIO_TASK_NAME },
#if 1
    { RDEF_LINEIN_TASK_NAME },
#endif // 1
    { RDEF_UDISK_TASK_NAME },
};

static const rcsp_apptask_cbk_t support_app_msg_callback_group[] =
{
    rcsp_bt_callback,
    rcsp_music_callback,
    rcsp_light_callback,//独立模式 callback
    rcsp_radio_callback,
#if 1
    rcsp_linein_callback,
#endif // 0
    rcsp_udisk_callback,

};

#if 0
static const char support_app_group[][16] =
{
    { RDEF_BTSTACK_TASK_NAME },
    { RDEF_MUSIC_TASK_NAME },
    { RDEF_RADIO_TASK_NAME },
#if 1
    { RDEF_LINEIN_TASK_NAME },
#endif // 1
    { RDEF_UDISK_TASK_NAME },
    { RDEF_LIGHT_TASK_NAME },
};

static const rcsp_apptask_cbk_t support_app_msg_callback_group[] =
{
    rcsp_bt_callback,
    rcsp_music_callback,
    rcsp_radio_callback,
#if 1
    rcsp_linein_callback,
#endif // 0
    rcsp_udisk_callback,
    rcsp_light_callback,//独立模式 callback

};

#endif

void BigData_buf_malloc(u16 size)
{
    if(rcsp_send_BigData_buf != NULL)
    {
        free(rcsp_send_BigData_buf);
        rcsp_send_BigData_buf = NULL;
    }
    rcsp_send_BigData_buf = malloc(size);
}

void BigData_buf_free(void)
{
    if(rcsp_send_BigData_buf != NULL)
    {
        free(rcsp_send_BigData_buf);
        rcsp_send_BigData_buf = NULL;
    }
}

void get_eq_table_value(s8 *eq_buf)
{
    int cnt;
    s8 *eq_table = (void*)user_eq_seg_gain;
    for(cnt = 0; cnt < 10*6; cnt++)
    {
        eq_buf[cnt] = eq_table[cnt];
    }
}

void rcsp_msg_deal_setflag(u32 deal_msg,u32 set_value)
{
    u8 bit_map = 0xff;

    switch(deal_msg)
    {
    case MSG_RSCP_REPORT_FRAME:
        bit_map = RCSP_MSG_DEAL_REPORT_FRAME;
        break;

    case MSG_RSCP_REPORT_CTL_STATE:
        bit_map = RCSP_MSG_DEAL_CTL_STATE;
        break;

    case MSG_RSCP_CHANGE_WORDMODE_ACTION:
        bit_map = RCSP_MSG_DEAL_CHANGE_MODE;
        break;

    case MSG_RSCP_REPORT_LIGHT_STATE:
        bit_map = RCSP_MSG_DEAL_LIGHT_STATE;
        break;

    case MSG_RSCP_REPORT_LONGFRAME:
        bit_map = RCSP_MSG_DEAL_LONG_FRAME;
        break;

    case MSG_RSCP_REPORT_ID3_INFO:
        bit_map = RCSP_MSG_DEAL_GET_ID3;
        break;

    default:
        break;
    }

    if(bit_map != 0xff)
    {
        if(set_value)
        {
            rcsp_deal_msg_flag |= bit_map;
        }
        else
        {
            rcsp_deal_msg_flag &= (~bit_map);
            if(RCSP_MSG_DEAL_LONG_FRAME == bit_map)
            {
                BigData_buf_free();
            }
        }
    }
}

void rcsp_msg_deal_wait(u32 deal_msg)
{
    u8 bit_map = 0xff;

    switch(deal_msg)
    {
    case MSG_RSCP_REPORT_FRAME:
        bit_map = RCSP_MSG_DEAL_REPORT_FRAME;
        break;

    case MSG_RSCP_REPORT_CTL_STATE:
        bit_map = RCSP_MSG_DEAL_CTL_STATE;
        break;

    case MSG_RSCP_CHANGE_WORDMODE_ACTION:
        bit_map = RCSP_MSG_DEAL_CHANGE_MODE;
        break;

    case MSG_RSCP_REPORT_LIGHT_STATE:
        bit_map = RCSP_MSG_DEAL_LIGHT_STATE;
        break;

    case MSG_RSCP_REPORT_LONGFRAME:
        bit_map = RCSP_MSG_DEAL_LONG_FRAME;
        break;

    case MSG_RSCP_REPORT_ID3_INFO:
        bit_map = RCSP_MSG_DEAL_GET_ID3;
        break;

    default:
        break;
    }

    if(bit_map != 0xff)
    {
        while((rcsp_deal_msg_flag & bit_map) != 0)
        {
            os_time_dly(1);
        }
    }
}


extern void ble_server_disconnect(void);
static void rscp_msg_deal_callback(u32 deal_msg,u32 send_result)
{
    P_FUNCTION
#if 1
    if(send_result != 0)
    {
        rcsp_com_err_count++;
        if(rcsp_com_err_count > 5)
        {
            rcsp_err_puts("---rcsp com err,disconn com\n");
        	rcsp_fw_disconnect();
		}
    }
    else
    {
        rcsp_com_err_count = 0;
    }
#endif

    rcsp_msg_deal_setflag(deal_msg,0);

}


//------------------------------------------------------------------------------------------------

void packet_frame_data(task_frame_parm_t *frame_p,u8 *value_buf_addr,char *id_str,u8 code_type,u8 *data,u16 len)
{
    frame_p->code_type = code_type;
    memcpy(frame_p->id_str,id_str,4);
    frame_p->value_len = len;

    if(value_buf_addr != NULL)
    {
        frame_p->value_buf = value_buf_addr;
        if(value_buf_addr != data)
        {
            memcpy(value_buf_addr,data,len);
        }
    }
    else
    {
        frame_p->value_buf = data; //data必须为静态内存
    }

}

static rcsp_apptask_cbk_t get_task_callback(const char *name)
{
    u32 cnt;
    u32 size = sizeof(support_app_group)/16;

    for(cnt = 0; cnt < size; cnt++)
    {
        if(strcmp(support_app_group[cnt],name) == 0)
        {
            return support_app_msg_callback_group[cnt];
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------------------------

static void rcsp_support_app_register(void)
{
    u16 cnt;
    cnt = sizeof(support_app_group)/16;
    rcsp_register_support_app(cnt,(void*)support_app_group);
}


void rcsp_event_com_start(u8 type)
{
    P_FUNCTION

    if(rcsp_com_run_flag != 0)
    {
        rcsp_err_puts("com already run\n");
        return;
    }

    rcsp_com_start(type);

    rcsp_com_report_flag = 0;
    rcsp_deal_msg_flag = 0;
    rcsp_com_run_flag = 1;
    rcsp_conn_apk_type = type;
    rcsp_com_err_count = 0;

    rcsp_send_BigData_buf = NULL;
    rcsp_send_tmp_buf = malloc(RCSP_SEND_TMP_BUF_SIZE);
    CHECK_MALLOC(rcsp_send_tmp_buf,1);

    rcsp_support_app_register();
    rcsp_register_msg_cbk(rscp_msg_deal_callback);

    rcsp_app_start(curr_task->name);

    rcsp_connect_current_task((void*)curr_task->name);

}

void rcsp_event_com_stop(void)
{
    P_FUNCTION

    if(rcsp_com_run_flag == 0)
    {
        rcsp_err_puts("com not run\n");
        return;
    }

    rcsp_com_run_flag = 0;
    rcsp_com_stop();
    

    //make sure buffer not be used
    while(rcsp_send_data_state != 0)
    {
        rcsp_tag_puts("---wait send_state end!!!\n");
        os_time_dly(1);
    }

    free(rcsp_send_tmp_buf);
    rcsp_send_tmp_buf = NULL;
    BigData_buf_free();
    rcsp_conn_apk_type = 0;
}


void rcsp_set_task_status_do(u8 status)
{
    if(rcsp_cur_task_status != status)
    {
        rcsp_cur_task_status = status;
        rcsp_com_report_flag |= (RCSP_REPORT_STATUS_CONTROL);
    }
}

void rcsp_apptask_start_deal_do(const char *name)
{
    char *task_name;

    P_FUNCTION

    task_name = (void*)name;
    rcsp_apptask_cbk_t cbk = get_task_callback(task_name);
    if(cbk == NULL)
    {
        rcsp_err_puts("task not support!!!\n");
        cbk = get_task_callback(RDEF_BTSTACK_TASK_NAME);
    }
    rcsp_db_printf("start task= %s, cbk = 0x%x\n",task_name,(u32)cbk);
    Post_msg_to_rcsp_Task_do(MSG_RSCP_CHANGE_WORDMODE_ACTION, (u32)task_name, (u32)cbk);
    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_LIGHT_STATE, (u32)light_info_tab.button, (u32)rcsp_light_callback);//公共模式callback

    rcsp_com_report_flag = RCSP_REPORT_START_STATE;

}


void rcsp_connect_current_task(char *task_name)
{
    if(0 == strcmp(task_name,RDEF_MUSIC_TASK_NAME))
    {
        rcsp_report_status(RCSP_REPORT_START_CONFIG);
    }
    else if(0 == strcmp(task_name,RDEF_RADIO_TASK_NAME))
    {
        rcsp_report_status(RCSP_REPORT_START_CONFIG);
    }
    else if(0 == strcmp(task_name,RDEF_UDISK_TASK_NAME))
    {        
        rcsp_report_status(RCSP_REPORT_START_CONFIG);
    }
	else if(0 == strcmp(curr_task->name,RDEF_BTSTACK_TASK_NAME))
    {
        ;
    }
    else if(0 == strcmp(curr_task->name,RDEF_LIGHT_TASK_NAME))
    {
        ;
    }
    else
    {
        ;
    }

}

//------------------------------------------------------------------------------------------------
void Post_appstart_state_to_rcsp(void)
{
    int cnt;
    char *next_task_name;

    P_FUNCTION

    next_task_name = (void*)curr_task->name;
    for(cnt = 0; cnt < sizeof(support_app_group)/16; cnt++)
    {
        if(0 == strcmp(next_task_name,support_app_group[cnt]))
        {
            break;
        }
    }

    if(cnt == sizeof(support_app_group)/16)
    {
        rcsp_err_puts("not support task\n");
        return;
    }

    rcsp_db_printf("rcsp: start task, <%s>\n", next_task_name);

    if(0 == strcmp(next_task_name,RDEF_BTSTACK_TASK_NAME))
    {
        rcsp_report_status(RCSP_REPORT_START_CONFIG);
    }
    else if(0 == strcmp(next_task_name,RDEF_MUSIC_TASK_NAME))
    {
	   	; //wait config
    }
    else if(0 == strcmp(next_task_name,RDEF_RADIO_TASK_NAME))
    {
        ;//wait config
    }
    else if(0 == strcmp(next_task_name,RDEF_LINEIN_TASK_NAME))
    {
        rcsp_report_status(RCSP_REPORT_START_CONFIG);
    }
    else if(0 == strcmp(next_task_name,RDEF_UDISK_TASK_NAME))
    {
        ;//wait config
    }
    else if(0 == strcmp(next_task_name,RDEF_LIGHT_TASK_NAME))
    {
        rcsp_report_status(RCSP_REPORT_START_CONFIG);
    }
    else
    {
        ;
    }

}


void Post_appstart_config_to_rcsp(void)
{
    int cnt;
    char *next_task_name;

    P_FUNCTION

    next_task_name = (void*)curr_task->name;
    for(cnt = 0; cnt < sizeof(support_app_group)/16; cnt++)
    {
        if(0 == strcmp(next_task_name,support_app_group[cnt]))
        {
            break;
        }
    }

    if(cnt == sizeof(support_app_group)/16)
    {
        return;
    }

///    rcsp_db_printf("rcsp: start task, <%s>\n", next_task_name);

    ///rcsp_set_task_status(STAUS_PLAY);
    if(0 == strcmp(next_task_name,RDEF_BTSTACK_TASK_NAME))
    {
        Post_Bt_StartInfo_to_rcsp();
    ///    rcsp_set_task_status(STAUS_PAUSE);
    }
    else if(0 == strcmp(next_task_name,RDEF_MUSIC_TASK_NAME))
    {
        Post_Music_StartInfo_to_rcsp();
    }
    else if(0 == strcmp(next_task_name,RDEF_RADIO_TASK_NAME))
    {
        Post_Radio_StartInfo_to_rcsp();
    }
    else if(0 == strcmp(next_task_name,RDEF_LINEIN_TASK_NAME))
    {
        Post_Linein_StartInfo_to_rcsp();
    }
    else if(0 == strcmp(next_task_name,RDEF_UDISK_TASK_NAME))
    {
        Post_Udisk_StartInfo_to_rcsp();
    }
    else if(0 == strcmp(next_task_name,RDEF_LIGHT_TASK_NAME))
    {
        Post_Light_StartInfo_to_rcsp();
    }
    else
    {
        ;
    }

    rcsp_report_status(RCSP_REPORT_STATUS_CONTROL);

}


u32 Post_OneFrameStatus_to_rcsp(char *id_str,u8 code_type,u8 *data,u16 len)
{
    u16 cnt;
    u32 tmp32;
    task_frame_parm_t *frame_p;
    
    P_FUNCTION

    frame_p = (void*)rcsp_send_tmp_buf;
    frame_p->value_buf = rcsp_send_tmp_buf + sizeof(task_frame_parm_t);

    frame_p->code_type = code_type;
    memcpy(frame_p->id_str,id_str,4);
    frame_p->value_len = len;

    if(len > 0)
    {
        memcpy(frame_p->value_buf,data,len);
    }
    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t));
    return 0;
}



void Post_Common_FrameStatus_to_rcsp(void)
{
    u16 cnt = 0;
    task_frame_parm_t *frame_p;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*2);

    P_FUNCTION
    frame_p = (void*)rcsp_send_tmp_buf;
    CHECK_MALLOC(frame_p,1);

    if(0 == strcmp(curr_task->name,RDEF_UDISK_TASK_NAME))
    {
        value_buf_addr[0] = MAX_SYS_VOL_L;
        packet_frame_data(frame_p+cnt,NULL,"MVOL",FRAME_TYPE_ANSI,(void*)value_buf_addr,1);
        cnt++;
        packet_frame_data(frame_p+cnt,NULL,"CVOL",FRAME_TYPE_ANSI,(void*)&usb_spk_vol_value,1);
        cnt++;
    }
    else
    {
        value_buf_addr[0] = MAX_SYS_VOL_L; 
		packet_frame_data(frame_p+cnt,NULL,"MVOL",FRAME_TYPE_ANSI,(void*)value_buf_addr,1);
        cnt++;
        packet_frame_data(frame_p+cnt,NULL,"CVOL",FRAME_TYPE_ANSI,(void*)&dac_ctl.sys_vol_l,1);
        cnt++;
    }

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}

static const char music_dev_name[][4] =
{
    {"SD0"},
    {"SD1"},
    {"USB"},
};

void Post_MusicDev_FrameStatus_to_rcsp(void)
{
    u8 value[4];
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*6);

    P_FUNCTION

    CHECK_MALLOC(frame_p,1);

//    value[0] = (u8)file_operate_get_total_phydev();
//    packet_frame_data(frame_p+cnt,value_buf_addr++,"MDEV",FRAME_TYPE_ANSI,value,1);
//    cnt++;

    packet_frame_data(frame_p+cnt,NULL,"CDEV",FRAME_TYPE_ANSI,&curr_device,1);
    cnt++;
//    packet_frame_data(frame_p+cnt,NULL,"DEVN",FRAME_TYPE_ANSI,(void*)music_dev_name[curr_device],3);
//    cnt++;

    value[0] = play_mode - REPEAT_ALL;
    value[0] = 1 << value[0];
    packet_frame_data(frame_p+cnt,value_buf_addr++,"PLCU",FRAME_TYPE_ANSI,value,1);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}


void Post_Mode_EQ_config_to_rcsp(void)
{

#if RCSP_EQ_EN
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*3);

    P_FUNCTION
    CHECK_MALLOC(frame_p,1);

    value_buf_addr[0] = RCSP_SYS_EQ_SL;
    packet_frame_data(frame_p+cnt,NULL,"EQSL",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

    get_eq_table_value((void*)value_buf_addr);

    packet_frame_data(frame_p+cnt,NULL,"EQVL",FRAME_TYPE_ANSI,(void*)value_buf_addr,6*10);
    cnt++;
    value_buf_addr += 6*10;

    if(0 == strcmp(curr_task->name,RDEF_BTSTACK_TASK_NAME))
    {
        value_buf_addr[0] = 1 << get_bt_eq_mode();
    }
    else
    {
        value_buf_addr[0] = 1 << eq_mode;
    }

    if(rcsp_eq_sw_flag == 0)
    {
        value_buf_addr[0] = 0;
    }

    packet_frame_data(frame_p+cnt,NULL,"EQCU",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

#endif

}

void Post_Mode_EQ_select_to_rcsp(void)
{

#if RCSP_EQ_EN

    u8 value[4];
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*3);

    P_FUNCTION
    CHECK_MALLOC(frame_p,1);

    if(0 == strcmp(curr_task->name,RDEF_BTSTACK_TASK_NAME))
    {
        value[0] = get_bt_eq_mode();
    }
    else if(0 == strcmp(curr_task->name,RDEF_MUSIC_TASK_NAME))
    {
        value[0] = eq_mode;
    }
    else if(0 == strcmp(curr_task->name,RDEF_RADIO_TASK_NAME))
    {
        value[0] = eq_mode;
    }
    else
    {
        return;
    }

    value[0] = 1 << value[0];

    if(rcsp_eq_sw_flag == 0)
    {
        value[0] = 0;
    }

    packet_frame_data(frame_p+cnt,value_buf_addr++,"EQCU",FRAME_TYPE_ANSI,value,1);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

#endif

}


void Post_Music_PlayMode_config_to_rcsp(void)
{
    u8 value[4];
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*2);

    P_FUNCTION
    CHECK_MALLOC(frame_p,1);

//    value[0] = RCSP_SYS_PLAYMODE_SL;
//    packet_frame_data(frame_p+cnt,value_buf_addr++,"PLMD",FRAME_TYPE_ANSI,value,1);
//    cnt++;

    value[0] = play_mode - REPEAT_ALL;
    value[0] = 1 << value[0];
    packet_frame_data(frame_p+cnt,value_buf_addr++,"PLCU",FRAME_TYPE_ANSI,value,1);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}


void Post_Music_FileInfo_to_rcsp(void)
{
    u32 tmp32;
    u8 tmp8;
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*9);

    P_FUNCTION

    if(music_name_buf.tpath == NULL)
    {
        return;
    }

    rcsp_update_cur_device(curr_device);

    packet_frame_data(frame_p+cnt,NULL,"CDEV",FRAME_TYPE_ANSI,&curr_device,1);
    cnt++;

//    printf("music_file,cur_num= %d,total_num= %d \n",music_cur_filenum,music_cur_filetotal);

    WRITE_BIG_U16(value_buf_addr,music_cur_filenum);
    packet_frame_data(frame_p+cnt,NULL,"TOTF",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    WRITE_BIG_U16(value_buf_addr,music_cur_filetotal);
    packet_frame_data(frame_p+cnt,NULL,"CURF",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    WRITE_BIG_U16(value_buf_addr,music_playfile_total_time);
    packet_frame_data(frame_p+cnt,NULL,"TOTT",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    WRITE_BIG_U16(value_buf_addr,0);
    packet_frame_data(frame_p+cnt,NULL,"SATT",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    tmp32 = get_decode_time();
    WRITE_BIG_U16(value_buf_addr,tmp32);
    packet_frame_data(frame_p+cnt,NULL,"CURT",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    rcsp_db_printf("music_time: start= %d,curr= %d, total= %d\n",0,tmp32,music_playfile_total_time);

    if(music_name_buf.file_name->lfn_cnt == 0)
    {
        tmp32 = strlen((void*)music_name_buf.file_name->lfn);
        tmp8 = FRAME_TYPE_ANSI;
    }
    else
    {
        tmp32 = music_name_buf.file_name->lfn_cnt;
        tmp8 = FRAME_TYPE_UTF16;
    }

    rcsp_db_printf("play_file's name: %d\n",tmp32);

    rcsp_put_buf((void*)music_name_buf.file_name->lfn,tmp32);
    packet_frame_data(frame_p+cnt,NULL,"FILN",tmp8,(void*)music_name_buf.file_name->lfn,tmp32);
    cnt++;

    packet_frame_data(frame_p+cnt,NULL,"CLUS",FRAME_TYPE_ANSI,(void*)&play_file_clust,4);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_LONGFRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}

void Post_Music_PlayTime_to_rcsp(void)
{
    u32 tmp32;
    u8 tmp8;
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*3);

    P_FUNCTION

//    WRITE_BIG_U16(value_buf_addr,music_playfile_total_time);
//    packet_frame_data(frame_p+cnt,NULL,"TOTT",FRAME_TYPE_ANSI,value_buf_addr,2);
//    cnt++;
//    value_buf_addr += 2;
//
//    WRITE_BIG_U16(value_buf_addr,0);
//    packet_frame_data(frame_p+cnt,NULL,"SATT",FRAME_TYPE_ANSI,value_buf_addr,2);
//    cnt++;
//    value_buf_addr += 2;

    tmp32 = get_decode_time();
    WRITE_BIG_U16(value_buf_addr,tmp32);
    packet_frame_data(frame_p+cnt,NULL,"CURT",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    rcsp_db_printf("music_time: curr= %d \n",tmp32);
    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);
}

void Post_Radio_Info_to_rcsp(void)
{
    u8 i=0;
    u8 j=0;
    u8 byte=0;
    u16 cnt = 0;
    u8 *fm_data;
    u8 channel = 0;
    u16 len = 1;
    u16 freq = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*6);

    P_FUNCTION
    CHECK_MALLOC(frame_p,1);

    BigData_buf_malloc(1024);
    fm_data = rcsp_send_BigData_buf;
    CHECK_MALLOC(fm_data,1);

    packet_frame_data(frame_p+cnt,value_buf_addr,"FMCF",FRAME_TYPE_ANSI,(void*)&fm_mode_var->wFreChannel,1);
    cnt++;

    value_buf_addr++;
    value_buf_addr[0] = fm_mode_var->wFreq >> 8;
    value_buf_addr[1] = fm_mode_var->wFreq & 0xff;
    packet_frame_data(frame_p+cnt,NULL,"FMCP",FRAME_TYPE_ANSI,(void*)value_buf_addr,2);
    value_buf_addr += 2;
    cnt++;

    if(fm_mode_var->wTotalChannel > 0)
    {
        for(i = 0; i < MEM_FM_LEN; i++)
        {
            byte = fm_info->dat[FM_CHANNL + i];
            for(j=0; j < 8; j++)
            {
                if(byte & BIT(0))
                {
                    channel++;
                    freq = MIN_FRE + (i*8+j);
                    fm_data[len] = channel;
                    fm_data[len+1] = (freq>>8);
                    fm_data[len+2] = (freq&0xff);
                    len+=3;
                }
                byte >>= 1; //
            }
        }
    }
    else
    {
        channel = 0;
    }
    fm_data[0] = channel;

    packet_frame_data(frame_p+cnt,fm_data,"FMTF",FRAME_TYPE_ANSI,fm_data,len);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_LONGFRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}

void Post_Radio_CurChannel_to_rcsp(void)
{
    u8 value[4];
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*3);

    P_FUNCTION
    CHECK_MALLOC(frame_p,1);

    packet_frame_data(frame_p+cnt,value_buf_addr++,"FMCF",FRAME_TYPE_ANSI,(void*)&fm_mode_var->wFreChannel,1);
    cnt++;

    value_buf_addr[0] = fm_mode_var->wFreq >> 8;
    value_buf_addr[1] = fm_mode_var->wFreq & 0xff;
    packet_frame_data(frame_p+cnt,NULL,"FMCP",FRAME_TYPE_ANSI,(void*)value_buf_addr,2);
    value_buf_addr += 2;
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}

void Post_Light_Info_to_rcsp(void)
{
    u32 tmp32;
    u8 tmp8;
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*8);

    P_FUNCTION

    if(light_info_tab.button)
    {
        if(light_info_tab.mode == LGT_MD_SCENE)
        {
            value_buf_addr[0] = 0xff;
            value_buf_addr[1] = light_info_tab.m_value[0];
        }
        else
        {
            value_buf_addr[0] = 0;
            value_buf_addr[1] = light_info_tab.mode;
        }
    }
    else
    {
        value_buf_addr[0] = 0;
        value_buf_addr[1] = 0;
    }

    packet_frame_data(frame_p+cnt,NULL,"LMOD",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    if(light_info_tab.mode == LGT_MD_COLOR_FLASH)
    {
        packet_frame_data(frame_p+cnt,NULL,"LEFF",FRAME_TYPE_ANSI,(void*)&light_info_tab.m_value[0],1);
        cnt++;
    }

    if(light_info_tab.mode == LGT_MD_WHITE)
    {
        packet_frame_data(frame_p+cnt,NULL,"LWHI",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_WHITE],1);
        cnt++;
    }

    if(light_info_tab.mode == LGT_MD_COLOR)
    {
        packet_frame_data(frame_p+cnt,NULL,"LRED",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_RED],1);
        cnt++;

        packet_frame_data(frame_p+cnt,NULL,"LGRR",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_GREEN],1);
        cnt++;

        packet_frame_data(frame_p+cnt,NULL,"LBLU",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_BLUE],1);
        cnt++;
    }

    if(light_info_tab.button != 0)
    {
        packet_frame_data(frame_p+cnt,NULL,"LBRI",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_BRIGHT],1);
        cnt++;
    }

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}



void Post_Udisk_StartInfo_to_rcsp(void)
{
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*3);

    packet_frame_data(frame_p+cnt,NULL,"MVOL",FRAME_TYPE_ANSI,(void*)&rcsp_vol_max_value,1);
    cnt++;
    packet_frame_data(frame_p+cnt,NULL,"CVOL",FRAME_TYPE_ANSI,(void*)&usb_spk_vol_value,1);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}

void Post_Light_StartInfo_to_rcsp(void)
{
    u32 tmp32;
    u8 tmp8;
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*8);

    P_FUNCTION

    if(light_info_tab.button)
    {
        if(light_info_tab.mode == LGT_MD_SCENE)
        {
            value_buf_addr[0] = 0xff;
            value_buf_addr[1] = light_info_tab.m_value[0];
        }
        else
        {
            value_buf_addr[0] = 0;
            value_buf_addr[1] = light_info_tab.mode;
        }
    }
    else
    {
        value_buf_addr[0] = 0;
        value_buf_addr[1] = 0;
    }

    packet_frame_data(frame_p+cnt,NULL,"LMOD",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    if(light_info_tab.mode == LGT_MD_COLOR_FLASH)
    {
        packet_frame_data(frame_p+cnt,NULL,"LEFF",FRAME_TYPE_ANSI,(void*)&light_info_tab.m_value[0],1);
        cnt++;
    }

    if(light_info_tab.mode == LGT_MD_WHITE)
    {
        packet_frame_data(frame_p+cnt,NULL,"LWHI",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_WHITE],1);
        cnt++;
    }

    if(light_info_tab.mode == LGT_MD_COLOR)
    {
        packet_frame_data(frame_p+cnt,NULL,"LRED",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_RED],1);
        cnt++;

        packet_frame_data(frame_p+cnt,NULL,"LGRR",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_GREEN],1);
        cnt++;

        packet_frame_data(frame_p+cnt,NULL,"LBLU",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_BLUE],1);
        cnt++;
    }

    if(light_info_tab.button != 0)
    {
        packet_frame_data(frame_p+cnt,NULL,"LBRI",FRAME_TYPE_ANSI,(void*)&light_info_tab.color_value[LGT_COLOR_BRIGHT],1);
        cnt++;
    }

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}


void Post_Linein_StartInfo_to_rcsp(void)
{
    u16 cnt = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*3);

    packet_frame_data(frame_p+cnt,NULL,"MVOL",FRAME_TYPE_ANSI,(void*)&rcsp_vol_max_value,1);
    cnt++;
    packet_frame_data(frame_p+cnt,NULL,"CVOL",FRAME_TYPE_ANSI,(void*)&dac_ctl.sys_vol_l,1);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_FRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}

void Post_Bt_StartInfo_to_rcsp(void)
{
    u16 cnt = 0;
    task_frame_parm_t *frame_p;/// = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf;/// + (sizeof(task_frame_parm_t)*3);

    BigData_buf_malloc(512);
    CHECK_MALLOC(rcsp_send_BigData_buf,1);

    frame_p = (void*)&rcsp_send_BigData_buf[64];

    packet_frame_data(frame_p+cnt,NULL,"MVOL",FRAME_TYPE_ANSI,(void*)&rcsp_vol_max_value,1);
    cnt++;
    packet_frame_data(frame_p+cnt,NULL,"CVOL",FRAME_TYPE_ANSI,(void*)&dac_ctl.sys_vol_l,1);
    cnt++;

#if RCSP_EQ_EN
    value_buf_addr[0] = RCSP_SYS_EQ_SL;
    packet_frame_data(frame_p+cnt,NULL,"EQSL",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

    get_eq_table_value((void*)rcsp_send_BigData_buf);
    packet_frame_data(frame_p+cnt,(void*)NULL,"EQVL",FRAME_TYPE_ANSI,(void*)rcsp_send_BigData_buf,6*10);
    cnt++;

    value_buf_addr[0] = 1 << get_bt_eq_mode();
    if(rcsp_eq_sw_flag == 0)
    {
        value_buf_addr[0] = 0;
    }
    packet_frame_data(frame_p+cnt,NULL,"EQCU",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;
#endif

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_LONGFRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

post_end:
    ;

}

void Post_Music_StartInfo_to_rcsp(void)
{
    u16 cnt = 0;
    u32 tmp32;
    u8 tmp8;
    task_frame_parm_t *frame_p;/// = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf;/// + (sizeof(task_frame_parm_t)*3);

    BigData_buf_malloc(512);
    CHECK_MALLOC(rcsp_send_BigData_buf,1);

    frame_p = (void*)&rcsp_send_BigData_buf[64];

    packet_frame_data(frame_p+cnt,NULL,"MVOL",FRAME_TYPE_ANSI,(void*)&rcsp_vol_max_value,1);
    cnt++;
    packet_frame_data(frame_p+cnt,NULL,"CVOL",FRAME_TYPE_ANSI,(void*)&dac_ctl.sys_vol_l,1);
    cnt++;

#if RCSP_EQ_EN
    value_buf_addr[0] = RCSP_SYS_EQ_SL;
    packet_frame_data(frame_p+cnt,NULL,"EQSL",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

    get_eq_table_value((void*)rcsp_send_BigData_buf);
    packet_frame_data(frame_p+cnt,(void*)NULL,"EQVL",FRAME_TYPE_ANSI,(void*)rcsp_send_BigData_buf,6*10);
    cnt++;

    value_buf_addr[0] = 1 << eq_mode;

	if(rcsp_eq_sw_flag == 0)
	{
		value_buf_addr[0] = 0; 
	}

	packet_frame_data(frame_p+cnt,NULL,"EQCU",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;
#endif

    value_buf_addr[0] = RCSP_SYS_PLAYMODE_SL;
    packet_frame_data(frame_p+cnt,NULL,"PLMD",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

    value_buf_addr[0] = play_mode - REPEAT_ALL;
    value_buf_addr[0] = 1 << value_buf_addr[0];
    packet_frame_data(frame_p+cnt,NULL,"PLCU",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

    value_buf_addr[0] = (u8)file_operate_get_total_phydev();
    if(value_buf_addr[0] == 0)
    {
        rcsp_err_puts("no device\n");
        goto post_end;
    }

    rcsp_update_cur_device(curr_device);

    packet_frame_data(frame_p+cnt,NULL,"CDEV",FRAME_TYPE_ANSI,&curr_device,1);
    cnt++;

    WRITE_BIG_U16(value_buf_addr,music_cur_filenum);
    packet_frame_data(frame_p+cnt,NULL,"TOTF",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    WRITE_BIG_U16(value_buf_addr,music_cur_filetotal);
    packet_frame_data(frame_p+cnt,NULL,"CURF",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    WRITE_BIG_U16(value_buf_addr,music_playfile_total_time);
    packet_frame_data(frame_p+cnt,NULL,"TOTT",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    WRITE_BIG_U16(value_buf_addr,0);
    packet_frame_data(frame_p+cnt,NULL,"SATT",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    tmp32 = get_decode_time();
    WRITE_BIG_U16(value_buf_addr,tmp32);
    packet_frame_data(frame_p+cnt,NULL,"CURT",FRAME_TYPE_ANSI,value_buf_addr,2);
    cnt++;
    value_buf_addr += 2;

    rcsp_db_printf("music_time: start= %d,curr= %d, total= %d\n",0,tmp32,music_playfile_total_time);

    if(music_name_buf.file_name->lfn_cnt == 0)
    {
        tmp32 = strlen((void*)music_name_buf.file_name->lfn);
        tmp8 = FRAME_TYPE_ANSI;
    }
    else
    {
        tmp32 = music_name_buf.file_name->lfn_cnt;
        tmp8 = FRAME_TYPE_UTF16;
    }

    rcsp_db_printf("play_file's name: %d\n",tmp32);

    rcsp_put_buf((void*)music_name_buf.file_name->lfn,tmp32);
    packet_frame_data(frame_p+cnt,NULL,"FILN",tmp8,(void*)music_name_buf.file_name->lfn,tmp32);
    cnt++;

    packet_frame_data(frame_p+cnt,NULL,"CLUS",FRAME_TYPE_ANSI,(void*)&play_file_clust,4);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_LONGFRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

post_end:
    ;
}

void Post_Radio_StartInfo_to_rcsp(void)
{
    u8 i=0;
    u8 j=0;
    u8 byte=0;
    u16 cnt = 0;
    u8 *fm_data;
    u8 channel = 0;
    u16 len = 1;
    u16 freq = 0;
    task_frame_parm_t *frame_p = (void*)rcsp_send_tmp_buf;
    u8 *value_buf_addr = rcsp_send_tmp_buf + (sizeof(task_frame_parm_t)*9);

    P_FUNCTION
    CHECK_MALLOC(frame_p,1);

    BigData_buf_malloc(1024);
    fm_data = rcsp_send_BigData_buf;
    CHECK_MALLOC(fm_data,1);

    packet_frame_data(frame_p+cnt,NULL,"MVOL",FRAME_TYPE_ANSI,(void*)&rcsp_vol_max_value,1);
    cnt++;
    packet_frame_data(frame_p+cnt,NULL,"CVOL",FRAME_TYPE_ANSI,(void*)&dac_ctl.sys_vol_l,1);
    cnt++;

#if RCSP_EQ_EN

    value_buf_addr[0] = RCSP_SYS_EQ_SL;
    packet_frame_data(frame_p+cnt,NULL,"EQSL",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

    get_eq_table_value((void*)&fm_data[1024-64]);

    packet_frame_data(frame_p+cnt,(void*)NULL,"EQVL",FRAME_TYPE_ANSI,(void*)&fm_data[1024-64],6*10);
    cnt++;

    value_buf_addr[0] = 1 << eq_mode;

    if(rcsp_eq_sw_flag == 0)
    {
        value_buf_addr[0] = 0;
    }

    packet_frame_data(frame_p+cnt,NULL,"EQCU",FRAME_TYPE_ANSI,value_buf_addr,1);
    cnt++;
    value_buf_addr++;

#endif

    packet_frame_data(frame_p+cnt,value_buf_addr,"FMCF",FRAME_TYPE_ANSI,(void*)&fm_mode_var->wFreChannel,1);
    cnt++;
    value_buf_addr++;

    value_buf_addr[0] = fm_mode_var->wFreq >> 8;
    value_buf_addr[1] = fm_mode_var->wFreq & 0xff;
    packet_frame_data(frame_p+cnt,NULL,"FMCP",FRAME_TYPE_ANSI,(void*)value_buf_addr,2);
    value_buf_addr += 2;
    cnt++;

    if(fm_mode_var->wTotalChannel > 0)
    {
        for(i = 0; i < MEM_FM_LEN; i++)
        {
            byte = fm_info->dat[FM_CHANNL + i];
            for(j=0; j < 8; j++)
            {
                if(byte & BIT(0))
                {
                    channel++;
                    freq = MIN_FRE + (i*8+j);
                    fm_data[len] = channel;
                    fm_data[len+1] = (freq>>8);
                    fm_data[len+2] = (freq&0xff);
                    len+=3;
                }
                byte >>= 1; //
            }
        }
    }
    else
    {
        channel = 0;
    }
    fm_data[0] = channel;

    packet_frame_data(frame_p+cnt,fm_data,"FMTF",FRAME_TYPE_ANSI,fm_data,len);
    cnt++;

    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_LONGFRAME,(int)frame_p,sizeof(task_frame_parm_t)*cnt);

}


u8 Post_msg_to_rcsp_Task_do(u32 msg, u32 param1, u32 param2)
{
    u8 err = 0;

    if(0 == rcsp_com_run_flag)
    {
        rcsp_db_puts("rcsp com not run \n");
        rcsp_com_report_flag = 0;
        return 0;
    }

    rcsp_msg_deal_setflag(msg,1);

    err = os_taskq_post(RCSP_TASK_NAME, 3, msg, (u32) param1, param2);

    if (0 != err)
    {
        rcsp_err_puts("rcsp:*task post msg fail\n");
        rcsp_msg_deal_setflag(msg,0);
    }
    else
    {
        ;
    }
    return err;
}


//------------------------------------------------------------------------------------------------
extern void test_fw_led_mode(void);

u8 Update_rcsp_Task_State_do(int msg,u8 update_tag)
{
    u8 err = 0;
    static u8 update_count = 0;
    static u8 debug_count = 0;
    static u8 wait_app_conn_timeout = 0;
    //P_FUNCTION

    debug_count++;
    if((debug_count %16) == 0)
    {
///        GET_SYS_MEM_STAT();
    }

//    test_fw_led_mode();

    if(0 == rcsp_com_run_flag)
    {
        ///rcsp_db_puts("rcsp com not run \n");
        rcsp_com_report_flag = 0;
        wait_app_conn_timeout = 0;
		return 0;
    }

    if(rcsp_check_app_connection() == FALSE)
    {
        rcsp_db_puts("wait app connect\n");
		if(wait_app_conn_timeout > 14)
		{
        	rcsp_fw_disconnect();
			wait_app_conn_timeout = 0;
		}
		else
		{
			wait_app_conn_timeout++;
		}
        return 0;
    }

    if ((msg != MSG_HALF_SECOND)&&(0 != update_tag))
    {
        ///rcsp_com_report_flag |= (RCSP_REPORT_STATUS_CONTROL);
    }

    rcsp_send_data_state = 0x55;
    while((rcsp_com_report_flag != 0) && (rcsp_deal_msg_flag == 0))
    {
        rcsp_db_printf("\n---rcsp_com_report_flag= 0x%04x\n",rcsp_com_report_flag);
        update_count = 0;

        if(rcsp_com_report_flag & RCSP_REPORT_STATUS_CONTROL)
        {
            rcsp_db_puts("--st_ctl\n");
            rcsp_com_report_flag &= (~RCSP_REPORT_STATUS_CONTROL);
            Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_CTL_STATE,(u32)rcsp_cur_task_status,1);
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_START_STATE)
        {
            rcsp_db_puts("--st_start_state\n");
            rcsp_com_report_flag  &= (~RCSP_REPORT_START_STATE);
            Post_appstart_state_to_rcsp();
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_START_CONFIG)
        {
            rcsp_db_puts("--st_start_config\n");
            rcsp_com_report_flag  &= (~RCSP_REPORT_START_CONFIG);
            Post_appstart_config_to_rcsp();
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_VOL_VALUE)
        {
            rcsp_db_puts("--st_vol\n");
            rcsp_com_report_flag &= (~RCSP_REPORT_VOL_VALUE);
            if(0 == strcmp(curr_task->name,RDEF_UDISK_TASK_NAME))
            {
                Post_OneFrameStatus_to_rcsp("CVOL",0,(void*)&usb_spk_vol_value,1);
            }
            else
            {
			   	Post_OneFrameStatus_to_rcsp("CVOL",0,(void*)&dac_ctl.sys_vol_l,1);
            }
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_EQ_CONFIG)
        {
            rcsp_db_puts("--st_eq\n");
            rcsp_com_report_flag &=(~RCSP_REPORT_EQ_CONFIG);
            Post_Mode_EQ_config_to_rcsp();
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_EQ_VALUE)
        {
            rcsp_db_puts("--sel_eq\n");
            rcsp_com_report_flag &=(~RCSP_REPORT_EQ_VALUE);
            Post_Mode_EQ_select_to_rcsp();
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_PLAYMODE_VALUE)
        {
            rcsp_db_puts("--st_playmode\n");
            rcsp_com_report_flag &= (~RCSP_REPORT_PLAYMODE_VALUE);
            Post_Music_PlayMode_config_to_rcsp();
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_MUSIC_FILEINFO)
        {
            rcsp_db_puts("--st_fileinfo\n");
            if(getting_music_info == 0x50)
            {
                rcsp_com_report_flag &= (~RCSP_REPORT_MUSIC_FILEINFO);
                Post_Music_FileInfo_to_rcsp();
                goto end_break;
            }
            else
            {
                rcsp_db_puts("getting\n");
                break;
            }

        }

        if(rcsp_com_report_flag & RCSP_REPORT_RADIO_TABLE)
        {
            rcsp_db_puts("--st_radioinfo\n");
            rcsp_com_report_flag &= (~RCSP_REPORT_RADIO_TABLE);
            Post_Radio_Info_to_rcsp();
            goto end_break;
        }

        if(rcsp_com_report_flag & RCSP_REPORT_RADIO_CHANNEL)
        {
            rcsp_db_puts("--st_radio channel\n");
            rcsp_com_report_flag &= (~RCSP_REPORT_RADIO_CHANNEL);
            Post_Radio_CurChannel_to_rcsp();
            goto end_break;
        }


        if(rcsp_com_report_flag & RCSP_REPORT_MUSIC_PLAYTIME)
        {
            rcsp_db_puts("--st_music playtime\n");
            rcsp_com_report_flag &= (~RCSP_REPORT_MUSIC_PLAYTIME);
            if(rcsp_cur_task_status == STAUS_PLAY)
            {
                Post_Music_PlayTime_to_rcsp();
            }
            goto end_break;
            
        }

        if(rcsp_com_report_flag & RCSP_REPORT_LIGHT_INFO)
        {
            rcsp_db_puts("--st_light info\n");
            rcsp_com_report_flag &= (~RCSP_REPORT_LIGHT_INFO);
            Post_Light_Info_to_rcsp();
            goto end_break;
        }

end_break:
        ;
    }

    rcsp_send_data_state = 0;
    if((rcsp_com_report_flag != 0) || (rcsp_deal_msg_flag != 0))
    {
        rcsp_db_printf("\n***wait_flag: 0x%x, 0x%x\n",rcsp_com_report_flag , rcsp_deal_msg_flag);
        
	if(rcsp_deal_msg_flag != 0)
	{
            if(update_count > 12)
            {
               rcsp_deal_msg_flag = 0;
	       update_count = 0;
               rcsp_err_puts("**reset--rcsp_deal_msg_flag\n");
            }
	    else
	    {
	       update_count++;
	    }
	}
    }
    else
    {
        update_count = 0;
    }
    return err;
}


void post_playfile_id3_info(void *hdl)
{
#if 0
    if(rcsp_com_run_flag == 0)
    {
        rcsp_db_puts("com not run\n");
        return;
    }
    Post_msg_to_rcsp_Task_do(MSG_RSCP_REPORT_ID3_INFO,(u32)hdl,1);
    rcsp_msg_deal_wait(MSG_RSCP_REPORT_ID3_INFO);
#endif
}

void rcsp_fw_disconnect(void)
{
    P_FUNCTION
    if(0 != rcsp_com_run_flag)
    {
        ///puts("send cmd\n");
        if(rcsp_conn_apk_type == 2)
        {
            ble_server_disconnect();
        }
        else
        {
            bt_spp_disconnect();
        }
    }
}

#endif //#if





