#ifndef __BT_KEY_H__
#define __BT_KEY_H__

#include "typedef.h"

//extern const u8 fm_ad_table[4][10];
//extern const u8 fm_io_table[4][10];
//extern const u8 fm_ir_table[4][21];
extern const u8 fm_touch_table[4][10];

#ifdef SUPPORT_TM1628_KEY_READ
  #if LED_KEY_MAX
 extern const u8 fm_led_table[4][LED_KEY_MAX];
  #else
 extern const u8 fm_led_table[4][10];
  #endif
#endif

#ifdef adkey_max_num
extern const u8 fm_ad_table[4][adkey_max_num];
#else
extern const u8 fm_ad_table[4][10];
#endif
extern const u8 fm_io_table[4][10];


#ifdef ir_max_num
extern const u8 fm_ir_table[4][ir_max_num];
#else
extern const u8 fm_ir_table[4][21];
#endif


#endif
