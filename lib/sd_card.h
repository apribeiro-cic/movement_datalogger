#ifndef SD_CARD_H
#define SD_CARD_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hardware/adc.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"

#include "ff.h"
#include "diskio.h"
#include "f_util.h"
#include "hw_config.h"
#include "my_debug.h"
#include "rtc.h"
#include "sd_card.h"

#define ADC_PIN 26 // GPIO 26

static bool logger_enabled;
static const uint32_t period = 1000;
static absolute_time_t next_log_time;

static char filename[20] = "imu_data.csv";

sd_card_t *sd_get_by_name(const char *const name);
FATFS *sd_get_fs_by_name(const char *name);
void run_setrtc();
void run_format();
void run_mount();
void run_unmount();
void run_getfree();
void run_ls();
void run_cat();
void run_help();
void process_stdio(int cRxedChar);
void capture_adc_data_and_save();
void read_file(const char *filename);

#endif // SD_CARD_H