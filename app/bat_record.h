#ifndef __BAT_RECORD_H__
#define __BAT_RECORD_H__

#include "g_data.h"

/********************* Flash Address Definitions *********************/
#define FLASH_LOG_BASE              AP_CFG_ROM_ADDR_LOG
#define FLASH_PAGE_SIZE             512     // MTP page size in bytes

// Single page layout - all data in one 512-byte page
#define ADDR_MAGIC                  (FLASH_LOG_BASE + 0x00)
#define ADDR_EXCEPTION_COUNTER      (FLASH_LOG_BASE + 0x04)
#define ADDR_EXCEPTION_WRITE_PTR    (FLASH_LOG_BASE + 0x08)
#define ADDR_RESERVED               (FLASH_LOG_BASE + 0x0C)
#define ADDR_EXCEPTION_RECORDS      (FLASH_LOG_BASE + 0x10)


/********************* Configuration Parameters *********************/
// Version Management
#define MAGIC_VALUE_V5          0x42415430  // 'BAT0' (v5) - old version
#define MAGIC_VALUE_V6          0x42415436  // 'BAT6' (v6) - optimized version
#define MAGIC_VALUE             MAGIC_VALUE_V6

#define ONE_HOUR_MS             3600000UL   // One hour in milliseconds

// Exception type definitions
#define EXCEPTION_TYPE_OVERVOLTAGE  0x01    // Overvoltage
#define EXCEPTION_TYPE_OVERTEMP     0x02    // Over temperature
#define EXCEPTION_TYPE_UNDERTEMP    0x03    // Under temperature
#if CONFIG_NEW_CCC_LOG_ENABLE
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
 * @brief Read all exception records (unified format)
 * @param buf Receive buffer
 * @param max_count Maximum number to read
 * @return Actual number of records read
 */
uint8_t battery_record_read_exceptions(BatteryExceptionRecord_t *buf, uint8_t max_count);


/**
 * @brief Print next abnormal record (overvoltage or temperature)
 */
void battery_record_print_next_log(void);

/**
 * @brief Erase all exception records and reset storage to initial state.
 * Called from engineering mode erase command.
 */
void battery_record_erase_all(void);

/**
 * @brief Reset exception tracking state without erasing records.
 * Clears g_exception_cache so new parameters can trigger fresh OV/OT detection.
 */
void battery_record_reset_tracking(void);
#endif
#endif /* __BAT_RECORD_H__ */
