#include "wb7720.h"
#include "config.h"

#ifndef USB_STRING_LEN_MAX
#    define USB_STRING_LEN_MAX 50
#endif

/** \brief Standard USB Descriptor Header (LUFA naming conventions).
 *
 *  Type define for all descriptors' standard header, indicating the descriptor's length and type. This structure
 *  uses LUFA-specific element names to make each element's purpose clearer.
 *
 *  \see \ref USB_StdDescriptor_Header_t for the version of this type with standard element names.
 *
 *  \note Regardless of CPU architecture, these values should be stored as little endian.
 */
typedef __PACKED_STRUCT {
  uint8_t Size; /**< Size of the descriptor, in bytes. */
  uint8_t Type; /**< Type of the descriptor, either a value in \ref USB_DescriptorTypes_t or a value
                 *   given by the specific class.
                 */
} USB_Descriptor_Header_t;

typedef __PACKED_STRUCT {
  USB_Descriptor_Header_t Header; /**< Descriptor header, including type and size. */

  uint16_t UnicodeString[USB_STRING_LEN_MAX]; /**< String data, as unicode characters (alternatively,
                                               *   string language IDs). If normal ASCII characters are
                                               *   to be used, they must be added as an array of characters
                                               *   rather than a normal C string so that they are widened to
                                               *   Unicode size.
                                               *
                                               *   Under GCC, strings prefixed with the "L" character (before
                                               *   the opening string quotation mark) are considered to be
                                               *   Unicode strings, and may be used instead of an explicit
                                               *   array of ASCII characters on little endian devices with
                                               *   UTF-16-LE \c wchar_t encoding.
                                               */
} USB_Descriptor_String_t;

