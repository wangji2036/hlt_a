#ifndef __BAT_RECORD_H__
#define __BAT_RECORD_H__

#include "g_data.h"
#include "config.h"

#define BAT_LOG_DEBUG 0
#if SUPPORT_BAT_RECORD_LOG
	#define br_printk 	printk
	#if BAT_LOG_DEBUG
		#define br_printk_debug 	printk
	#else
	#define br_printk_debug(...)
	#endif
#else
	#define br_printk(...)
#endif

/********************* Flash Address Definitions *********************/
// Multi-page flash addresses (support 1-3 pages)
#define FLASH_LOG_PAGE1             AP_CFG_ROM_ADDR_LOG1  // 0x1400 - Page 1
#define FLASH_LOG_PAGE2             AP_CFG_ROM_ADDR_LOG2  // 0x1200 - Page 2
#define FLASH_LOG_PAGE3             AP_CFG_ROM_ADDR_LOG3  // 0x1000 - Page 3
#define FLASH_PAGE_SIZE             512                   // Page size in bytes

// Get page address by page number (supports 3 pages)
#define GET_ACTIVE_PAGE_ADDR(page_num) \
    ((page_num) == FLASH_PAGE_LOG1 ? FLASH_LOG_PAGE1 : \
     (page_num) == FLASH_PAGE_LOG2 ? FLASH_LOG_PAGE2 : \
     FLASH_LOG_PAGE3)

// Legacy compatibility (defaults to LOG1 for old code)
#define FLASH_LOG_BASE              FLASH_LOG_PAGE1
#define ADDR_MAGIC                  (FLASH_LOG_BASE + 0x00)
#define ADDR_EXCEPTION_COUNTER      (FLASH_LOG_BASE + 0x04)
#define ADDR_EXCEPTION_WRITE_PTR    (FLASH_LOG_BASE + 0x08)
#define ADDR_RESERVED               (FLASH_LOG_BASE + 0x0C)
#define ADDR_EXCEPTION_RECORDS      (FLASH_LOG_BASE + 0x10)


/********************* Configuration Parameters *********************/
// Version Management
#define MAGIC_VALUE_V5          0x42415430  // 'BAT0' (v5) - old version
#define MAGIC_VALUE_V6          0x42415436  // 'BAT6' (v6) - single page optimized
#define MAGIC_VALUE_V7          0x42415437  // 'BAT7' (v7) - dual-page version
#define MAGIC_VALUE             MAGIC_VALUE_V7  // Current version

#define ONE_HOUR_MS             3600000UL   // One hour in milliseconds
#define SLEEP_EXCEPTION_CHECK_CYCLES  40    // Check every ~60 sleep cycles (~72 seconds)

// Exception type definitions
#define EXCEPTION_TYPE_OVERVOLTAGE  0x01    // Overvoltage
#define EXCEPTION_TYPE_OVERTEMP     0x02    // Over temperature
#define EXCEPTION_TYPE_UNDERTEMP    0x03    // Under temperature

// Note: Structure definitions and bitfield macros moved to g_data.h


/********************* Public API Function Declarations *********************/

/**
 * @brief Initialize battery record function (check Flash and initialize cache)
 */
void battery_record_init(void);

/**
 * @brief Update overvoltage record (internal call)
 */
void battery_record_update_overvoltage(void);

/**
 * @brief Update temperature abnormal record (internal call)
 */
void battery_record_update_temperature(void);

/**
 * @brief Periodic check function (called in main loop)
 */
void battery_record_periodic_check(void);

/**
 * @brief Print next abnormal record (overvoltage or temperature)
 */
void battery_record_print_next_log(void);

/**
 * @brief Lightweight exception check for sleep mode
 * @note Checks 1-hour window and saves to Flash if needed
 * @return 1 if Flash write occurred, 0 otherwise
 */
uint8_t battery_record_sleep_check(void);

/**
 * @brief Erase all exception records (engineering mode erase command)
 * @return true if erase succeeded, false otherwise
 */
uint8_t battery_record_erase_all(void);

/**
 * @brief Get count of overtemperature exception records
 * @return Number of overtemp records across all log pages
 */
uint16_t battery_record_get_overtemp_count(void);

/**
 * @brief Get count of overvoltage exception records
 * @return Number of overvoltage records across all log pages
 */
uint16_t battery_record_get_overvolt_count(void);

/**
 * @brief Read single record by page and index, with tracking overlay
 * @param page Page number (0=LOG1, 1=LOG2)
 * @param index Record index within the page (0-23)
 * @param record Output buffer for the record (20 bytes)
 * @return true if read succeeded
 */
uint8_t battery_record_read_by_page_index(uint8_t page, uint8_t index, BatteryExceptionRecord_t *record);

/**
 * @brief Get record count for a page (header-only read, 5 bytes)
 * @param page Page number (0=LOG1, 1=LOG2)
 * @return Number of valid records in the page (0-24)
 */
uint8_t battery_record_get_page_count(uint8_t page);

#endif /* __BAT_RECORD_H__ */
