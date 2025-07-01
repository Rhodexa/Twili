#ifndef LEDCHAN_H
#define LEDCHAN_H

#include <Arduino.h>
#include "driver/ledc.h"

unsigned int gamma_lut[] = {
  0x0000, 0x0002, 0x0008, 0x0013, 0x0021, 0x0034, 0x004A, 0x0065,
  0x0084, 0x00A7, 0x00CE, 0x00FA, 0x0129, 0x015D, 0x0194, 0x01D0,
  0x0210, 0x0254, 0x029D, 0x02E9, 0x0339, 0x038E, 0x03E7, 0x0444,
  0x04A5, 0x050A, 0x0573, 0x05E0, 0x0652, 0x06C8, 0x0741, 0x07BF,
  0x0841, 0x08C7, 0x0952, 0x09E0, 0x0A73, 0x0B09, 0x0BA4, 0x0C43,
  0x0CE6, 0x0D8D, 0x0E38, 0x0EE8, 0x0F9B, 0x1053, 0x110F, 0x11CF,
  0x1293, 0x135B, 0x1427, 0x14F8, 0x15CC, 0x16A5, 0x1782, 0x1863,
  0x1948, 0x1A31, 0x1B1E, 0x1C10, 0x1D05, 0x1DFF, 0x1EFD, 0x1FFF
};

int framebuffer[16] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

float values[16];
float targets[16];

const int PIN_LED[16] = {
  /*  0  */ 16,
  /*  1  */ 17,
  /*  2  */ 18,
  /*  3  */ 19,
  /*  4  */ 21,
  /*  5  */ 22,
  /*  6  */ 23,
  /*  7  */ 25,
  /*  8  */ 26,
  /*  9  */ 27,
  /* 10  */ 32,
  /* 11  */ 33,
  /* 12  */ 2,
  /* 13  */ 13,
  /* 14  */ 4,
  /* 15  */ 14
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
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)ch, 0x1FFF - (int)values[ch]);
    ledc_set_duty(LEDC_LOW_SPEED_MODE,  (ledc_channel_t)ch, 0x1FFF - (int)values[ch + 8]);
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