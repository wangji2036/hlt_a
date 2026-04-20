#ifndef TYPDEF_H_
#define TYPDEF_H_

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint32_t
#define uint32_t unsigned long
#endif

#ifndef uint64_t
#define uint64_t unsigned long long
#endif

#ifndef int8_t
#define int8_t signed char
#endif

#ifndef int16_t
#define int16_t signed short
#endif

#ifndef int32_t
#define int32_t signed long
#endif

#ifndef int64_t
#define int64_t signed long long
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif

typedef enum {FALSE, TRUE} BOOL;

typedef enum {false = 0, true} bool;

#define __I  volatile const /*!< Defines 'RO' permissions */
#define __O  volatile       /*!< Defines 'WO' permissions */
#define __IO volatile       /*!< Defines 'RW' permissions */

#endif /* TYPDEF_H_ */
