#include <stdlib.h>

#include "config.h"
#include "usb_def.h"
#include "usb_hid_def.h"
#include "usbd_user_desc.h"
#include "usbd_user_hid.h"

#define WBVAL(x) (x & 0xFF), ((x >> 8) & 0xFF)
#define STRING(x) #x         ///< stringify without expand
#define XSTRING(x) STRING(x) ///< expand then stringify
#define STRCAT(a, b) a##b    ///< concat without expand
#define USBSTR(s) STRCAT(L, s)

const uint8_t USBD_VendorHIDReportDescriptor[] = 
{
  0x06, 0x00, 0xFF, /* USAGE_PAGE (Vendor-Defined 1) */
  0x09, 0x01,       /* USAGE (Vendor-Defined 1) */
  0xA1, 0x01,       /* COLLECTION (Application) */
  0x15, 0x00,       /* LOGICAL_MINIMUM (0) */
  0x26, 0xFF, 0x00, /* LOGICAL_MAXIMUM (255) */
  0x75, 0x08,       /* REPORT_SIZE (8) */
  0x95, 0x40,       /* REPORT_COUNT (64) */
  0x09, 0x01,       /* USAGE (Vendor-Defined 1) */
  0x81, 0x02,       /* INPUT (Data,Var,Abs) */
  0x95, 0x40,       /* REPORT_COUNT (64) */
  0x09, 0x01,       /* USAGE (Vendor-Defined 1) */
  0x91, 0x02,       /* OUTPUT (Data,Var,Abs) */
  0x95, 0x40,       /* REPORT_COUNT (64) */
  0x09, 0x01,       /* USAGE (Vendor-Defined 1) */
  0xB1, 0x02,       /* FEATRUE (Data,Var,Abs) */
  0xC0,             /* END_COLLECTION */
};

const uint8_t USBD_DeviceDescriptor[] = 
{
  0x12,                                 /* bLength */
  USB_DESC_TYPE_DEVICE,                 /* bDescriptorType */
  WBVAL(0x0110), /* 1.10 */             /* bcdUSB */

  /* This is an Interface Class Defined Device */
  0x00,                                 /* bDeviceClass */
  0x00,                                 /* bDeviceSubClass */
  0x00,                                 /* bDeviceProtocol */

  0x40,                                 /* bMaxPacketSize0 0x40 = 64 */
  WBVAL(VENDOR_ID),                     /* idVendor */
  WBVAL(PRODUCT_ID),                    /* idProduct */
  WBVAL(DEVICE_VER),                    /* bcdDevice */
  1,                                    /* iManufacturer */
  2,                                    /* iProduct */
  3,                                    /* iSerialNumber */
  0x01                                  /* bNumConfigurations: one possible configuration*/
};


const uint8_t USBD_ConfigDescriptor[41] =
{
  /* Configuration 1 */
  0x09,                                 /* bLength */
  USB_DESC_TYPE_CONFIGURATION,          /* bDescriptorType */
  WBVAL(41),                            /* wTotalLength */
  0x01,                                 /* bNumInterfaces: 1 interface */
  0x01,                                 /* bConfigurationValue: 0x01 is used to select this configuration */
  0,                                    /* iConfiguration: no string to describe this configuration */
  USB_CONFIG_BUS_POWERED,               /* bmAttributes */
  USB_CONFIG_POWER_MA(100),             /* bMaxPower, device power consumption 100mA */

  /* offset: 9 */
  /************* Interface Descriptor **********/
  0x09,                                 /* bLength */
  USB_DESC_TYPE_INTERFACE,              /* bDescriptorType */
  USBD_VENDOR_HID_IF_NUM,               /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x02,                                 /* bNumEndpoints */
  0x03,                                 /* bInterfaceClass: HID */
  0x00,                                 /* bInterfaceSubClass */
  0x00,                                 /* bInterfaceProtocol */
  0,                                    /* iInterface */

  /* offset: 18 */
  /************* HID Descriptor **********/
  0x09,                                 /* bLength */
  HID_HID_DESCRIPTOR_TYPE,              /* bDescriptorType */
  WBVAL(0x0110),                        /* bcdHID */
  0x00,                                 /* bCountryCode */
  0x01,                                 /* bNumDescriptors */
  HID_REPORT_DESCRIPTOR_TYPE,           /* bDescriptorType */
  WBVAL(sizeof(USBD_VendorHIDReportDescriptor)),   /* wDescriptorLength */

  /* offset: 27 */
  /************* Endpoint Descriptor ***********/
  0x07,                                 /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType */
  USB_ENDPOINT_IN(0x01),                /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,          /* bmAttributes */
  WBVAL(0x0040),                        /* wMaxPacketSize 0x40 = 64 */
  0x01,                                 /* bInterval: 1ms */

  /* offset: 34 */
  /************* Endpoint Descriptor ***********/
  0x07,                                 /* bLength */
  USB_DESC_TYPE_ENDPOINT,               /* bDescriptorType */
  USB_ENDPOINT_OUT(0x01),               /* bEndpointAddress */
  USB_ENDPOINT_TYPE_INTERRUPT,          /* bmAttributes */
  WBVAL(0x0040),                        /* wMaxPacketSize 0x40 = 64 */
  0x01,                                 /* bInterval: 1ms */
};

/*
 * String descriptors
 */
const USB_Descriptor_String_t LanguageString = {
  .Header = {
    .Size                   = 4,
    .Type                   = USB_DESC_TYPE_STRING
  },
  .UnicodeString              = {0x0409}
}; /* LangID = 0x0409: U.S. English */

const USB_Descriptor_String_t ManufacturerString = {
  .Header = {
    .Size        = sizeof(USBSTR(MANUFACTURER)),
    .Type        = USB_DESC_TYPE_STRING
  },
  .UnicodeString = USBSTR(MANUFACTURER)
};

const USB_Descriptor_String_t ProductString = {
  .Header = {
    .Size        = sizeof(USBSTR(PRODUCT)),
    .Type        = USB_DESC_TYPE_STRING
  },
  .UnicodeString = USBSTR(PRODUCT)
};

const USB_Descriptor_String_t SerialNumberString = {
  .Header = {
    .Size        = sizeof(USBSTR(SERIAL_NUMBER)),
    .Type        = USB_DESC_TYPE_STRING
  },
  .UnicodeString = USBSTR(SERIAL_NUMBER)
};

/**
 * @brief  Returns the device descriptor.
 * @param  length: Pointer to data length variable.
 * @return Pointer to the device descriptor buffer.
 */
uint8_t* USBD_User_GetDeviceDescriptor(uint16_t* length)
{
  *length = sizeof(USBD_DeviceDescriptor);
  return ((uint8_t*)USBD_DeviceDescriptor);
}

/**
 * @brief  Returns the specified configuration descriptor.
 * @param  index: specifies the index of configuration descriptor.
 * @param  length: Pointer to data length variable.
 * @return Pointer to the specified configuration descriptor buffer.
 */
uint8_t* USBD_User_GetConfigDescriptor(uint8_t index, uint16_t* length)
{
  if (index < 1)
  {
    *length = (USBD_ConfigDescriptor[3] << 8) | USBD_ConfigDescriptor[2];
    return ((uint8_t*)USBD_ConfigDescriptor);
  }

  return NULL;
}

/**
 * @brief  Returns the specified string descriptor.
 * @param  index: specifies the index of string descriptor.
 * @param  length: Pointer to data length variable.
 * @return Pointer to the specified string descriptor buffer.
 */
uint8_t* USBD_User_GetStringDescriptor(uint8_t index, uint16_t* length)
{
  switch (index)
  {
    case 0: {
        *length = *((uint8_t *)(&LanguageString.Header.Size));
        return (uint8_t *)&LanguageString;
    } // break;
    case 1: {
        *length = *((uint8_t *)(&ManufacturerString.Header.Size));
        return (uint8_t *)&ManufacturerString;
    } // break;
    case 2: {
        *length = *((uint8_t *)(&ProductString.Header.Size));
        return (uint8_t *)&ProductString;
    } // break;
    case 3: {
        *length = *((uint8_t *)(&SerialNumberString.Header.Size));
        return (uint8_t *)&SerialNumberString;
    } // break;
  }

  return NULL;
}

uint8_t* USBD_User_GetHIDDescriptor(uint8_t interface, uint8_t desc_type, uint16_t* length)
{
  if (interface == USBD_VENDOR_HID_IF_NUM)
  {
    switch (desc_type)
    {
      case HID_HID_DESCRIPTOR_TYPE:
        *length = 9;
        return (((uint8_t*)USBD_ConfigDescriptor) + 18);
        // break;
      case HID_REPORT_DESCRIPTOR_TYPE:
        *length = sizeof(USBD_VendorHIDReportDescriptor);
        return ((uint8_t*)USBD_VendorHIDReportDescriptor);
        // break;
    }
  }

  return NULL;
}
