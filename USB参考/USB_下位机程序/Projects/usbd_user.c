#include "wb7720.h"
#include "usbd_user.h"
#include "usbd_hid.h"
#include "usbd_user_hid.h"

extern volatile uint32_t ep_out_evt;
extern volatile uint32_t ep_in_busy;

/**
 * @brief  Initializes the USB Device.
 * @return None
 */
void USBD_User_Init(void)
{
  /* Enable USB peripheral clock */
  RCC->APBENR |= RCC_APBENR_USBEN;

  /* Reset USB peripheral */
  RCC->APBRSTR |= RCC_APBRSTR_USBRST;
  RCC->APBRSTR &= ~RCC_APBRSTR_USBRST;

  /* Configure and enable USBCLK */
  RCC->USBCLKSRC = RCC_USBCLKSRC_HSI48;

  USB->INTRUSBE = 0x00;
  USB->INTRINE = 0x00;
  USB->INTROUTE = 0x00;

  NVIC_SetPriority(USB_IRQn, 0);
  NVIC_EnableIRQ(USB_IRQn);
}

/**
 * @brief  Deinitializes the USB Device.
 * @return None
 */
void USBD_User_DeInit(void)
{
  /* Disable USB interrupt channel */
  NVIC_DisableIRQ(USB_IRQn);

  /* Disable USBCLK */
  RCC->USBCLKSRC = RCC_USBCLKSRC_NONE;

  /* Disable USB peripheral clock */
  RCC->APBENR &= ~RCC_APBENR_USBEN;
}

/**
 * @brief  Connects the device to the USB host.
 * @return None
 */
void USBD_User_Connect(void)
{
  RCC->APBENR |= RCC_APBENR_GPIOEN;

  GPIO_Init(GPIOD, GPIO_Pin_1 | GPIO_Pin_2, GPIO_MODE_AF | GPIO_AF1 | GPIO_SPEED_HIGH);
  GPIOD->DRR &= ~(GPIO_Pin_1 | GPIO_Pin_2);
  GPIO_Init(GPIOD, GPIO_Pin_0 | GPIO_Pin_3, GPIO_MODE_AF | GPIO_AF3 | GPIO_SPEED_HIGH);
  GPIOD->DRR &= ~(GPIO_Pin_0 | GPIO_Pin_3);

  SYSCFG->CFGR2 |= (0x89A00000 | SYSCFG_CFGR2_DPPUEN);

  USB->POWER = USB_POWER_SUSEN;
  USB->INTRUSBE = USB_INTRUSBE_RSTIE | USB_INTRUSBE_RSUIE | USB_INTRUSBE_SUSIE;
}

/**
 * @brief  Disconnects the device from the USB host.
 * @return None
 */
void USBD_User_Disconnect(void)
{
  uint32_t cfgr2 = SYSCFG->CFGR2;

  cfgr2 &= ~SYSCFG_CFGR2_DPPUEN;
  SYSCFG->CFGR2 = (0x89A00000 | cfgr2);
}

/**
 * @brief  USB Reset Event Service Routine.
 * @return None
 */
void USBD_User_Reset(void)
{
  USB->POWER = USB_POWER_SUSEN;
  USB->INTRINE = USB_INTRINE_EP0E;
  USB->INTROUTE = 0x00;

  ep_out_evt = 0;
  ep_in_busy = 0;
}

/**
 * @brief  USB Resume Event Service Routine.
 * @return None
 */
void USBD_User_Resume(void)
{
}

/**
 * @brief  USB Suspend Event Service Routine.
 * @return None
 */
void USBD_User_Suspend(void)
{
}

/**
 * @brief  USB SOF Event Service Routine.
 * @return None
 */
void USBD_User_SOF(void)
{
}


/**
 * @brief  Configures device.
 * @param  cfgidx: the configuration index.
 * @return true - Success, false - Error
 */
bool USBD_User_SetConfig(uint8_t cfgidx)
{
  if (cfgidx == 1)
  {
    // Configure IN Endpoint 1 (Interrupt)
    USB->INDEX = 0x01;
    USB->INCSR2 = 0x00;
    USB->INMAXP = (64 >> 3);
    USB->INCSR1 = USB_INCSR1_CLRDATATOG;
    USB->INCSR1 = USB_INCSR1_FLUSHFIFO;
    USB->INCSR1 = USB_INCSR1_FLUSHFIFO;
    USB->INTRINE |= (0x01 << 1);

    // Configure OUT Endpoint 1 (Interrupt)
    USB->INDEX = 0x01;
    USB->OUTCSR2 = 0x00;
    USB->OUTMAXP = (64 >> 3);
    USB->OUTCSR1 = USB_OUTCSR1_CLRDATATOG;
    USB->OUTCSR1 = USB_OUTCSR1_FLUSHFIFO;
    USB->OUTCSR1 = USB_OUTCSR1_FLUSHFIFO;
    USB->INTROUTE |= (0x01 << 1);

    return true;
  }

  return false;
}

/**
 * @brief  Clear current configuration.
 * @param  cfgidx: the configuration index.
 * @note   If cfgidx is 0, this function should clear all configuration.
 * @return None
 */
void USBD_User_ClearConfig(uint8_t cfgidx)
{
  USB->INTRINE &= ~(0x01 << 1);
  USB->INTROUTE &= ~(0x01 << 1);
}


/**
 * @brief  Handle the setup device requests (Except the recipient is device).
 * @return The next control stage.
 */
UsbdControlStage USBD_User_EndPoint0_Setup(void)
{
  UsbdControlStage next_stage = USBD_CONTROL_STAGE_STALL;

  if ((UsbdCoreInfo.SetupPacket.bmRequestType & USB_REQUEST_RECIPIENT_Msk) == USB_REQUEST_RECIPIENT_INTERFACE)
  {
    if (UsbdCoreInfo.SetupPacket.wIndexL == USBD_VENDOR_HID_IF_NUM) {
      next_stage = USBD_EndPoint0_Setup_HID_Req();
    }
  }

  return next_stage;
}

/**
 * @brief  Handle the out device requests.
 * @return The next control stage.
 */
UsbdControlStage USBD_User_EndPoint0_Out(void)
{
  UsbdControlStage next_stage = USBD_CONTROL_STAGE_STALL;

  if ((UsbdCoreInfo.SetupPacket.bmRequestType & (USB_REQUEST_TYPE_Msk | USB_REQUEST_RECIPIENT_Msk)) == 
      (USB_REQUEST_TYPE_CLASS | USB_REQUEST_RECIPIENT_INTERFACE))
  {
    if (UsbdCoreInfo.SetupPacket.wIndexL == USBD_VENDOR_HID_IF_NUM) {
      next_stage = USBD_EndPoint0_Out_HID_Req();
    }
  }

  return next_stage;
}



/**
 * @brief  IN Endpoint 1 Service Routine.
 * @return None
 */
void USBD_User_EP1_IN(void)
{
  ep_in_busy = 0;
}

/**
 * @brief  IN Endpoint 2 Service Routine.
 * @return None
 */
void USBD_User_EP2_IN(void)
{
}

/**
 * @brief  IN Endpoint 3 Service Routine.
 * @return None
 */
void USBD_User_EP3_IN(void)
{
}

/**
 * @brief  IN Endpoint 4 Service Routine.
 * @return None
 */
void USBD_User_EP4_IN(void)
{
}

/**
 * @brief  OUT Endpoint 1 Service Routine.
 * @return None
 */
void USBD_User_EP1_OUT(void)
{
  USB->INDEX = 0x01;
  if (USB->OUTCSR1 & USB_OUTCSR1_OUTPKTRDY)
  {
    ep_out_evt = 1;
  }
}

/**
 * @brief  OUT Endpoint 2 Service Routine.
 * @return None
 */
void USBD_User_EP2_OUT(void)
{
}

/**
 * @brief  OUT Endpoint 3 Service Routine.
 * @return None
 */
void USBD_User_EP3_OUT(void)
{
}

/**
 * @brief  OUT Endpoint 4 Service Routine.
 * @return None
 */
void USBD_User_EP4_OUT(void)
{
}
