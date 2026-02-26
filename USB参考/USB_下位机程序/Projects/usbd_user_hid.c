#include "wb7720.h"
#include "usbd_core.h"
#include "usbd_user_hid.h"

static uint8_t hid_in_report_buf[64] = {0};

bool USBD_User_HID_GetReport(void)
{
  if ((UsbdCoreInfo.SetupPacket.wIndexL == USBD_VENDOR_HID_IF_NUM) &&
      (UsbdCoreInfo.SetupPacket.wValueH == HID_REPORT_INPUT) &&
      (UsbdCoreInfo.SetupPacket.wLength == 64)) {
    UsbdCoreInfo.DataPtr = hid_in_report_buf;
    return true;
  }

  return false;
}

bool USBD_User_HID_SetReport(bool data_received)
{
  return false;
}



bool USBD_User_HID_GetIdle(void)
{
  return false;
}

bool USBD_User_HID_SetIdle(void)
{
  return false;
}



bool USBD_User_HID_GetProtocol(void)
{
  return false;
}

bool USBD_User_HID_SetProtocol(void)
{
  return false;
}
