#ifndef LEDCHAN_H
#define LEDCHAN_H

#include <Arduino.h>
#include "driver/ledc.h"

/*unsigned int gamma_lut[] = {
  0x0000, 0x0002, 0x0008, 0x0013, 0x0021, 0x0034, 0x004A, 0x0065,
  0x0084, 0x00A7, 0x00CE, 0x00FA, 0x0129, 0x015D, 0x0194, 0x01D0,
  0x0210, 0x0254, 0x029D, 0x02E9, 0x0339, 0x038E, 0x03E7, 0x0444,
  0x04A5, 0x050A, 0x0573, 0x05E0, 0x0652, 0x06C8, 0x0741, 0x07BF,
  0x0841, 0x08C7, 0x0952, 0x09E0, 0x0A73, 0x0B09, 0x0BA4, 0x0C43,
  0x0CE6, 0x0D8D, 0x0E38, 0x0EE8, 0x0F9B, 0x1053, 0x110F, 0x11CF,
  0x1293, 0x135B, 0x1427, 0x14F8, 0x15CC, 0x16A5, 0x1782, 0x1863,
  0x1948, 0x1A31, 0x1B1E, 0x1C10, 0x1D05, 0x1DFF, 0x1EFD, 0x1FFF
};*/

unsigned int gamma_lut[] = {
  0x0000, 0x0001, 0x0002, 0x0005, 0x0008, 0x000D, 0x0012, 0x0019, 
  0x0021, 0x0029, 0x0033, 0x003D, 0x0049, 0x0056, 0x0064, 0x0072, 
  0x0082, 0x0093, 0x00A5, 0x00B7, 0x00CB, 0x00E0, 0x00F6, 0x010D, 
  0x0125, 0x013D, 0x0157, 0x0172, 0x018E, 0x01AB, 0x01C9, 0x01E8, 
  0x0208, 0x0229, 0x024B, 0x026E, 0x0292, 0x02B7, 0x02DD, 0x0304, 
  0x032D, 0x0356, 0x0380, 0x03AB, 0x03D7, 0x0404, 0x0433, 0x0462, 
  0x0492, 0x04C3, 0x04F6, 0x0529, 0x055D, 0x0593, 0x05C9, 0x0600, 
  0x0639, 0x0672, 0x06AC, 0x06E8, 0x0724, 0x0762, 0x07A0, 0x07E0, 
  0x0820, 0x0862, 0x08A4, 0x08E8, 0x092C, 0x0972, 0x09B8, 0x0A00, 
  0x0A49, 0x0A92, 0x0ADD, 0x0B29, 0x0B75, 0x0BC3, 0x0C12, 0x0C61, 
  0x0CB2, 0x0D04, 0x0D57, 0x0DAB, 0x0DFF, 0x0E55, 0x0EAC, 0x0F04, 
  0x0F5D, 0x0FB7, 0x1012, 0x106D, 0x10CA, 0x1128, 0x1187, 0x11E7, 
  0x1248, 0x12AA, 0x130D, 0x1371, 0x13D6, 0x143D, 0x14A4, 0x150C, 
  0x1575, 0x15DF, 0x164A, 0x16B6, 0x1723, 0x1792, 0x1801, 0x1871, 
  0x18E2, 0x1955, 0x19C8, 0x1A3C, 0x1AB2, 0x1B28, 0x1B9F, 0x1C18, 
  0x1C91, 0x1D0B, 0x1D87, 0x1E03, 0x1E81, 0x1EFF, 0x1F7F, 0x1FFF
};

int framebuffer[16] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

float values[16];
float targets[16];

const int PIN_LED[16] = {
  25, /* 0 C */ /*  0  */ 
  13, /* 0 W */ /*  1  */ 
   2, /* 0 R */ /*  2  */ 
   4, /* 0 G */ /*  3  */ 
  16, /* 0 B */ /*  4  */ 
  33, /* 1 C */ /*  5  */ 
  27, /* 1 W */ /*  6  */ 
  17, /* 1 R */ /*  7  */ 
  18, /* 1 G */ /*  8  */ 
  19, /* 1 B */ /*  9  */ 
  32, /* 2 C */ /* 10  */ 
  26, /* 2 W */ /* 11  */ 
  21, /* 2 R */ /* 12  */ 
  22, /* 2 G */ /* 13  */ 
  23, /* 2 B */ /* 14  */

  14  /* ??? */ /* 15  */ 
};

const int PWM_FRQ = 4883; //9765;
const int PWM_RESOLUTION = 13;
const ledc_timer_t PWM_TIMER = LEDC_TIMER_0;

void ledchan_begin(){
  ledc_timer_config_t group_a_ledc_timer = {
    .speed_mode       = LEDC_HIGH_SPEED_MODE,
    .duty_resolution  = (ledc_timer_bit_t)PWM_RESOLUTION,
    .timer_num        = PWM_TIMER,
    .freq_hz          = PWM_FRQ,
    .clk_cfg          = LEDC_AUTO_CLK
  };
  ledc_timer_config(&group_a_ledc_timer);

  ledc_timer_config_t group_b_ledc_timer = {
    .speed_mode       = LEDC_LOW_SPEED_MODE,
    .duty_resolution  = (ledc_timer_bit_t)PWM_RESOLUTION,
    .timer_num        = PWM_TIMER,
    .freq_hz          = PWM_FRQ,
    .clk_cfg          = LEDC_AUTO_CLK
  };
  ledc_timer_config(&group_b_ledc_timer);

  for (int ch = 0; ch < 8; ++ch) {
    ledc_channel_config_t group_a_ledc_channel = {
      .gpio_num       = PIN_LED[ch],
      .speed_mode     = LEDC_HIGH_SPEED_MODE,
      .channel        = (ledc_channel_t)ch,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = PWM_TIMER,
      .duty           = 0,
      .hpoint         = 0
    };
    ledc_channel_config(&group_a_ledc_channel);

    ledc_channel_config_t group_b_ledc_channel = {
      .gpio_num       = PIN_LED[ch + 8],
      .speed_mode     = LEDC_LOW_SPEED_MODE,
      .channel        = (ledc_channel_t)ch,
      .intr_type      = LEDC_INTR_DISABLE,
      .timer_sel      = PWM_TIMER,
      .duty           = 0,
      .hpoint         = 0
    };
    ledc_channel_config(&group_b_ledc_channel);
  }
  
  // Enable phase-shifting
  for (int ch = 0; ch < 8; ++ch) {
    ledc_set_duty_with_hpoint (LEDC_HIGH_SPEED_MODE, (ledc_channel_t)ch, 0, (8192 / 16) * ch);
    ledc_set_duty_with_hpoint (LEDC_LOW_SPEED_MODE,  (ledc_channel_t)ch, 0, (8192 / 16) * (ch + 8));
  }
}


void run(){
  for(int i = 0; i < 16; i++){
    values[i] += (targets[i] - values[i]) * 0.15;
  }

  for (int ch = 0; ch < 8; ++ch) {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)ch, (int)values[ch]);
    ledc_set_duty(LEDC_LOW_SPEED_MODE,  (ledc_channel_t)ch, (int)values[ch + 8]);
  }
  for (int ch = 0; ch < 8; ++ch) {
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)ch);
    ledc_update_duty(LEDC_LOW_SPEED_MODE,  (ledc_channel_t)ch);
  }
}

void ledchan_update()
{
  for(int i = 0; i < 16; i++){
    targets[i] = gamma_lut[framebuffer[i]];
  }
}

#endif