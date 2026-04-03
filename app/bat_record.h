#ifndef __BAT_RECORD_H__
#define __BAT_RECORD_H__

#include "g_data.h"

/********************* Flash Address Definitions *********************/
/* Multi-page layout: 2 pages for exception log rotation */
#define LOG_PAGE_COUNT          2
#define MAX_RECORDS_PER_PAGE    24
#define FLASH_LOG_PAGE1         0x1400      /* Primary log page */
#define FLASH_LOG_PAGE2         0x1200      /* Secondary log page */
#define FLASH_PAGE_SIZE         512

#define GET_ACTIVE_PAGE_ADDR(p) ((p) == 0 ? FLASH_LOG_PAGE1 : FLASH_LOG_PAGE2)

/********************* Configuration Parameters *********************/
/* Version management */
#define MAGIC_VALUE_V5          0x42415430  /* 'BAT0' (v5) - old version */
#define MAGIC_VALUE_V6          0x42415436  /* 'BAT6' (v6) - single page version */
#define MAGIC_VALUE_V7          0x42415437  /* 'BAT7' (v7) - multi-page version */
#define MAGIC_VALUE             MAGIC_VALUE_V7

/* Exception type definitions */
#define EXCEPTION_TYPE_OVERVOLTAGE  0x01
#define EXCEPTION_TYPE_OVERTEMP     0x02
#define EXCEPTION_TYPE_UNDERTEMP    0x03

#if CONFIG_NEW_CCC_LOG_ENABLE

/********************* Window Tracker Structures (GB31241) *********************/

/* Per-window overvoltage tracker (Cell1 and Cell2 independent) */
typedef struct {
    bool     triggered;        /* Exception occurred in this window */
    uint16_t max_voltage;      /* Max voltage in this window (mV) */
    uint16_t total_voltage;    /* Total voltage at max point (mV) */
    TimeStamp_t max_timestamp; /* RTC time when max was reached */
    uint8_t  cell_num;         /* Cell number (1 or 2) */
} OvWindowTracker_t;

/* Per-window temperature tracker (charging and discharging independent) */
typedef struct {
    bool     triggered;        /* Exception occurred in this window */
    int16_t  max_temperature;  /* Max temperature (0.1 degC) */
    TimeStamp_t max_timestamp; /* RTC time when max was reached */
    uint8_t  event_type;       /* 0x02=overtemp, 0x03=undertemp */
    uint8_t  charge_state;     /* BUCKBOOST_CHAGER_MODE or BUCKBOOST_DISCHG_MODE */
} TempWindowTracker_t;

/********************* Flash Page Layout (per-page 496B) *********************/

typedef struct __attribute__((packed)) {
    uint32_t magic;                                     /* 4B: Version magic */
    uint8_t  page_records_count;                        /* 1B: Records in this page (0-24) */
    uint8_t  page_number;                               /* 1B: Page index (0/1) */
    uint8_t  overflow_ptr;                              /* 1B: Reserved */
    uint8_t  page_seq;                                  /* 1B: Sequence number for newest page */
    uint32_t page_timestamp;                            /* 4B: Last write RTC seconds */
    BatteryExceptionRecord_t records[MAX_RECORDS_PER_PAGE]; /* 480B: 24 × 20B records */
    uint16_t checksum;                                  /* 2B: Page checksum */
    uint16_t padding;                                   /* 2B: Alignment */
} FlashPageLayout_t;  /* Total: 496B fits in 512B page */

/********************* Public API *********************/

void battery_record_init(void);
void battery_record_update_overvoltage(void);
void battery_record_update_temperature(void);
void battery_record_periodic_check(void);
uint8_t battery_record_read_exceptions(BatteryExceptionRecord_t *buf, uint8_t max_count);
void battery_record_print_next_log(void);
void battery_record_erase_all(void);
void battery_record_reset_tracking(void);

#endif /* CONFIG_NEW_CCC_LOG_ENABLE */
#endif /* __BAT_RECORD_H__ */
