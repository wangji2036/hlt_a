/**
 * @file    wb7720_uart0.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the UART0 firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_uart0.h"
#include "wb7720_rcc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup UART0
  * @brief UART0 driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/*!< UART0 CR1 register clear Mask ((~(uint32_t)0xFFFFE6F3)) */
#define CR1_CLEAR_MASK            ((uint32_t)(UART0_CR1_M | UART0_CR1_PCE | \
                                              UART0_CR1_PS | UART0_CR1_TE | \
                                              UART0_CR1_RE))

/*!< UART0 Interrupts mask */
#define IT_MASK                   ((uint32_t)0x000000FF)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup UART0_Private_Functions
  * @{
  */

/**
  * @brief  Deinitializes the UART0 peripheral registers to their default reset values.
  * @retval None
  */
void UART0_DeInit(void)
{
  RCC_APBPeriphResetCmd(RCC_APBPeriph_UART0, ENABLE);
  RCC_APBPeriphResetCmd(RCC_APBPeriph_UART0, DISABLE);
}

/**
  * @brief  Initializes the UART0 peripheral according to the specified
  *         parameters in the UART0_InitStruct .  
  * @param  UART0_InitStruct: pointer to a UART0_InitTypeDef structure that contains
  *         the configuration information for the specified UART0 peripheral.
  * @retval None
  */
void UART0_Init(UART0_InitTypeDef* UART0_InitStruct)
{
  uint32_t divider = 0, apbclock = 0, tmpreg = 0;
  RCC_ClocksTypeDef RCC_ClocksStatus;

  /* Check the parameters */
  assert_param(IS_UART0_BAUDRATE(UART0_InitStruct->UART0_BaudRate));  
  assert_param(IS_UART0_WORD_LENGTH(UART0_InitStruct->UART0_WordLength));
  assert_param(IS_UART0_STOPBITS(UART0_InitStruct->UART0_StopBits));
  assert_param(IS_UART0_PARITY(UART0_InitStruct->UART0_Parity));
  assert_param(IS_UART0_MODE(UART0_InitStruct->UART0_Mode));

  /* Disable UART0 */
  UART0->CR1 &= (uint32_t)~((uint32_t)UART0_CR1_UE);

  /*---------------------------- UART0 CR2 Configuration -----------------------*/
  tmpreg = UART0->CR2;
  /* Clear STOP[13:12] bits */
  tmpreg &= (uint32_t)~((uint32_t)UART0_CR2_STOP_Msk);

  /* Configure the UART0 Stop Bits, Clock, CPOL, CPHA and LastBit ------------*/
  /* Set STOP[13:12] bits according to UART0_StopBits value */
  tmpreg |= (uint32_t)UART0_InitStruct->UART0_StopBits;

  /* Write to UART0 CR2 */
  UART0->CR2 = tmpreg;

  /*---------------------------- UART0 CR1 Configuration -----------------------*/
  tmpreg = UART0->CR1;
  /* Clear M, PCE, PS, TE and RE bits */
  tmpreg &= (uint32_t)~((uint32_t)CR1_CLEAR_MASK);

  /* Configure the UART0 Word Length, Parity and mode ----------------------- */
  /* Set the M bits according to UART0_WordLength value */
  /* Set PCE and PS bits according to UART0_Parity value */
  /* Set TE and RE bits according to UART0_Mode value */
  tmpreg |= (uint32_t)UART0_InitStruct->UART0_WordLength | UART0_InitStruct->UART0_Parity |
    UART0_InitStruct->UART0_Mode;

  /* Write to UART0 CR1 */
  UART0->CR1 = tmpreg;

  /*---------------------------- UART0 BRR Configuration -----------------------*/
  /* Configure the UART0 Baud Rate -------------------------------------------*/
  RCC_GetClocksFreq(&RCC_ClocksStatus);
  apbclock = RCC_ClocksStatus.APBCLK_Frequency;

  /* Determine the integer part */
  if ((UART0->CR1 & UART0_CR1_OVER8) != 0)
  {
    /* (divider * 10) computing in case Oversampling mode is 8 Samples */
    divider = (uint32_t)((2 * apbclock) / (UART0_InitStruct->UART0_BaudRate));
    tmpreg  = (uint32_t)((2 * apbclock) % (UART0_InitStruct->UART0_BaudRate));
  }
  else /* if ((UART0->CR1 & CR1_OVER8_Set) == 0) */
  {
    /* (divider * 10) computing in case Oversampling mode is 16 Samples */
    divider = (uint32_t)((apbclock) / (UART0_InitStruct->UART0_BaudRate));
    tmpreg  = (uint32_t)((apbclock) % (UART0_InitStruct->UART0_BaudRate));
  }
  
  /* round the divider : if fractional part i greater than 0.5 increment divider */
  if (tmpreg >=  (UART0_InitStruct->UART0_BaudRate) / 2)
  {
    divider++;
  }
  
  /* Implement the divider in case Oversampling mode is 8 Samples */
  if ((UART0->CR1 & UART0_CR1_OVER8) != 0)
  {
    /* get the LSB of divider and shift it to the right by 1 bit */
    tmpreg = (divider & (uint16_t)0x000F) >> 1;
    
    /* update the divider value */
    divider = (divider & (uint16_t)0xFFF0) | tmpreg;
  }
  
  /* Write to UART0 BRR */
  UART0->BRR = (uint16_t)divider;
}

/**
  * @brief  Fills each UART0_InitStruct member with its default value.
  * @param  UART0_InitStruct: pointer to a UART0_InitTypeDef structure
  *         which will be initialized.
  * @retval None
  */
void UART0_StructInit(UART0_InitTypeDef* UART0_InitStruct)
{
  /* UART0_InitStruct members default value */
  UART0_InitStruct->UART0_BaudRate = 9600;
  UART0_InitStruct->UART0_WordLength = UART0_WordLength_8b;
  UART0_InitStruct->UART0_StopBits = UART0_StopBits_1;
  UART0_InitStruct->UART0_Parity = UART0_Parity_No ;
  UART0_InitStruct->UART0_Mode = UART0_Mode_Rx | UART0_Mode_Tx;
}

/**
  * @brief  Enables or disables the specified UART0 peripheral.
  * @param  NewState: new state of the UART0 peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void UART0_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the selected UART0 by setting the UE bit in the CR1 register */
    UART0->CR1 |= UART0_CR1_UE;
  }
  else
  {
    /* Disable the selected UART0 by clearing the UE bit in the CR1 register */
    UART0->CR1 &= (uint32_t)~((uint32_t)UART0_CR1_UE);
  }
}

/**
  * @brief  Enables or disables the UART0's transmitter or receiver.
  * @param  UART0_Direction: specifies the UART0 direction.
  *          This parameter can be any combination of the following values:
  *            @arg UART0_Mode_Tx: UART0 Transmitter
  *            @arg UART0_Mode_Rx: UART0 Receiver
  * @param  NewState: new state of the UART0 transfer direction.
  *         This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
void UART0_DirectionModeCmd(uint32_t UART0_DirectionMode, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_UART0_MODE(UART0_DirectionMode));
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 

  if (NewState != DISABLE)
  {
    /* Enable the UART0's transfer interface by setting the TE and/or RE bits 
       in the UART0 CR1 register */
    UART0->CR1 |= UART0_DirectionMode;
  }
  else
  {
    /* Disable the UART0's transfer interface by clearing the TE and/or RE bits
       in the UART0 CR3 register */
    UART0->CR1 &= (uint32_t)~UART0_DirectionMode;
  }
}

/**
  * @brief  Enables or disables the UART0's 8x oversampling mode.
  * @param  NewState: new state of the UART0 8x oversampling mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   This function has to be called before calling UART0_Init() function
  *         in order to have correct baudrate Divider value.
  * @retval None
  */
void UART0_OverSampling8Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the 8x Oversampling mode by setting the OVER8 bit in the CR1 register */
    UART0->CR1 |= UART0_CR1_OVER8;
  }
  else
  {
    /* Disable the 8x Oversampling mode by clearing the OVER8 bit in the CR1 register */
    UART0->CR1 &= (uint32_t)~((uint32_t)UART0_CR1_OVER8);
  }
}

/**
  * @brief  Enables or disables the UART0's most significant bit first 
  *         transmitted/received following the start bit.
  * @param  NewState: new state of the UART0 most significant bit first
  *         transmitted/received following the start bit.
  *          This parameter can be: ENABLE or DISABLE.
  * @note   This function has to be called before calling UART0_Cmd() function.  
  * @retval None
  */
void UART0_MSBFirstCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  if (NewState != DISABLE)
  {
    /* Enable the most significant bit first transmitted/received following the 
       start bit by setting the MSBFIRST bit in the CR2 register */
    UART0->CR2 |= UART0_CR2_MSBFIRST;
  }
  else
  {
    /* Disable the most significant bit first transmitted/received following the 
       start bit by clearing the MSBFIRST bit in the CR2 register */
    UART0->CR2 &= (uint32_t)~((uint32_t)UART0_CR2_MSBFIRST);
  }
}

/**
  * @brief  Enables or disables the binary data inversion.
  * @param  NewState: new defined levels for the UART0 data.
  *          This parameter can be:
  *            @arg ENABLE: Logical data from the data register are send/received in negative
  *                          logic (1=L, 0=H). The parity bit is also inverted.
  *            @arg DISABLE: Logical data from the data register are send/received in positive
  *                          logic (1=H, 0=L).
  * @note   This function has to be called before calling UART0_Cmd() function.  
  * @retval None
  */
void UART0_DataInvCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the binary data inversion feature by setting the DATAINV bit in 
       the CR2 register */
    UART0->CR2 |= UART0_CR2_DATAINV;
  }
  else
  {
    /* Disable the binary data inversion feature by clearing the DATAINV bit in 
       the CR2 register */
    UART0->CR2 &= (uint32_t)~((uint32_t)UART0_CR2_DATAINV);
  }
}

/**
  * @brief  Enables or disables the Pin(s) active level inversion.
  * @param  UART0_InvPin: specifies the UART0 pin(s) to invert.
  *          This parameter can be any combination of the following values:
  *            @arg UART0_InvPin_Tx: UART0 Tx pin active level inversion.
  *            @arg UART0_InvPin_Rx: UART0 Rx pin active level inversion.
  * @param  NewState: new active level status for the UART0 pin(s).
  *          This parameter can be:
  *            @arg ENABLE: pin(s) signal values are inverted (Vdd =0, Gnd =1).
  *            @arg DISABLE: pin(s) signal works using the standard logic levels (Vdd =1, Gnd =0).
  * @note   This function has to be called before calling UART0_Cmd() function.  
  * @retval None
  */
void UART0_InvPinCmd(uint32_t UART0_InvPin, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_UART0_INVERSTION_PIN(UART0_InvPin));  
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 

  if (NewState != DISABLE)
  {
    /* Enable the active level inversion for selected pins by setting the TXINV 
       and/or RXINV bits in the UART0 CR2 register */
    UART0->CR2 |= UART0_InvPin;
  }
  else
  {
    /* Disable the active level inversion for selected requests by clearing the 
       TXINV and/or RXINV bits in the UART0 CR2 register */
    UART0->CR2 &= (uint32_t)~UART0_InvPin;
  }
}

/**
  * @brief  Enables or disables the Auto Baud Rate.
  * @param  NewState: new state of the UART0 auto baud rate.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void UART0_AutoBaudRateCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the auto baud rate feature by setting the ABREN bit in the CR2 
       register */
    UART0->CR2 |= UART0_CR2_ABREN;
  }
  else
  {
    /* Disable the auto baud rate feature by clearing the ABREN bit in the CR2 
       register */
    UART0->CR2 &= (uint32_t)~((uint32_t)UART0_CR2_ABREN);
  }
}

/**
  * @brief  Selects the UART0 auto baud rate method.
  * @param  UART0_AutoBaudRate: specifies the selected UART0 auto baud rate method.
  *          This parameter can be one of the following values:
  *            @arg UART0_AutoBaudRate_StartBit: Start Bit duration measurement.
  *            @arg UART0_AutoBaudRate_FallingEdge: Falling edge to falling edge measurement.
  * @note   This function has to be called before calling UART0_Cmd() function.  
  * @retval None
  */
void UART0_AutoBaudRateConfig(uint32_t UART0_AutoBaudRate)
{
  /* Check the parameters */
  assert_param(IS_UART0_AUTOBAUDRATE_MODE(UART0_AutoBaudRate));

  UART0->CR2 &= (uint32_t)~((uint32_t)UART0_CR2_ABRMODE_Msk);
  UART0->CR2 |= UART0_AutoBaudRate;
}

/**
  * @brief  Transmits single data through the UART0 peripheral.
  * @param  Data: the data to transmit.
  * @retval None
  */
void UART0_SendData(uint16_t Data)
{
  /* Check the parameters */
  assert_param(IS_UART0_DATA(Data)); 
    
  /* Transmit Data */
  UART0->TDR = (Data & (uint16_t)0x01FF);
}

/**
  * @brief  Returns the most recent received data by the UART0 peripheral.
  * @retval The received data.
  */
uint16_t UART0_ReceiveData(void)
{
  /* Receive Data */
  return (uint16_t)(UART0->RDR & (uint16_t)0x01FF);
}

/**
  * @brief  Enables or disables the UART0's LIN mode.
  * @param  NewState: new state of the UART0 LIN mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void UART0_LINCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the LIN mode by setting the LINEN bit in the CR2 register */
    UART0->CR2 |= UART0_CR2_LINEN;
  }
  else
  {
    /* Disable the LIN mode by clearing the LINEN bit in the CR2 register */
    UART0->CR2 &= (uint32_t)~((uint32_t)UART0_CR2_LINEN);
  }
}

/**
  * @brief  Enables or disables the specified UART0 interrupts.
  * @param  UART0_IT: specifies the UART0 interrupt sources to be enabled or disabled.
  *          This parameter can be one of the following values:
  *            @arg UART0_IT_LBD:  LIN Break detection interrupt.
  *            @arg UART0_IT_TXE:  Tansmit Data Register empty interrupt.
  *            @arg UART0_IT_TC:  Transmission complete interrupt.
  *            @arg UART0_IT_RXNE:  Receive Data register not empty interrupt.
  *            @arg UART0_IT_IDLE:  Idle line detection interrupt.
  *            @arg UART0_IT_PE:  Parity Error interrupt.
  *            @arg UART0_IT_ERR:  Error interrupt(Frame error, noise error, overrun error)
  * @param  NewState: new state of the specified UART0 interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void UART0_ITConfig(uint32_t UART0_IT, FunctionalState NewState)
{
  uint32_t uart0reg = 0, itpos = 0, itmask = 0;
  uint32_t uart0base = 0;
  /* Check the parameters */
  assert_param(IS_UART0_CONFIG_IT(UART0_IT));
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  
  uart0base = (uint32_t)UART0_BASE;
  
  /* Get the UART0 register index */
  uart0reg = (((uint16_t)UART0_IT) >> 0x08);
  
  /* Get the interrupt position */
  itpos = UART0_IT & IT_MASK;
  itmask = (((uint32_t)0x01) << itpos);
  
  if (uart0reg == 0x02) /* The IT is in CR2 register */
  {
    uart0base += 0x04;
  }
  else if (uart0reg == 0x03) /* The IT is in CR3 register */
  {
    uart0base += 0x08;
  }
  else /* The IT is in CR1 register */
  {
  }
  if (NewState != DISABLE)
  {
    *(__IO uint32_t*)uart0base  |= itmask;
  }
  else
  {
    *(__IO uint32_t*)uart0base &= ~itmask;
  }
}

/**
  * @brief  Enables the specified UART0's Request.
  * @param  UART0_Request: specifies the UART0 request.
  *          This parameter can be any combination of the following values:
  *            @arg UART0_Request_RXFRQ: Receive data flush Request
  *            @arg UART0_Request_SBKRQ: Send Break Request
  *            @arg UART0_Request_ABRRQ: Auto Baud Rate Request
  * @param  NewState: new state of the request.
  *          This parameter can be: ENABLE or DISABLE.  
  * @retval None
  */
void UART0_RequestCmd(uint32_t UART0_Request, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_UART0_REQUEST(UART0_Request));
  assert_param(IS_FUNCTIONAL_STATE(NewState)); 

  if (NewState != DISABLE)
  {
    /* Enable the UART0 Request by setting the dedicated request bit in the RQR
       register.*/
    UART0->RQR |= UART0_Request;
  }
  else
  {
    /* Disable the UART0 Request by clearing the dedicated request bit in the RQR
       register.*/
    UART0->RQR &= (uint32_t)~UART0_Request;
  }
}

/**
  * @brief  Enables or disables the UART0's Overrun detection.
  * @param  UART0_OVRDetection: specifies the OVR detection status in case of OVR error.
  *          This parameter can be any combination of the following values:
  *            @arg UART0_OVRDetection_Enable: OVR error detection enabled when
  *                                            the UART0 OVR error is asserted.
  *            @arg UART0_OVRDetection_Disable: OVR error detection disabled when
  *                                             the UART0 OVR error is asserted.
  * @retval None
  */
void UART0_OverrunDetectionConfig(uint32_t UART0_OVRDetection)
{
  /* Check the parameters */
  assert_param(IS_UART0_OVRDETECTION(UART0_OVRDetection));
  
  /* Clear the OVR detection bit */
  UART0->CR3 &= (uint32_t)~((uint32_t)UART0_CR3_OVRDIS);
  /* Set the new value for the OVR detection bit */
  UART0->CR3 |= UART0_OVRDetection;
}

/**
  * @brief  Checks whether the specified UART0 flag is set or not.
  * @param  UART0_FLAG: specifies the flag to check.
  *          This parameter can be one of the following values:
  *            @arg UART0_FLAG_REACK:  Receive Enable acknowledge flag.
  *            @arg UART0_FLAG_TEACK:  Transmit Enable acknowledge flag.
  *            @arg UART0_FLAG_SBK:  Send Break flag.
  *            @arg UART0_FLAG_BUSY:  Busy flag.
  *            @arg UART0_FLAG_ABRF:  Auto baud rate flag.
  *            @arg UART0_FLAG_ABRE:  Auto baud rate error flag.
  *            @arg UART0_FLAG_LBD:  LIN Break detection flag.
  *            @arg UART0_FLAG_TXE:  Transmit data register empty flag.
  *            @arg UART0_FLAG_TC:  Transmission Complete flag.
  *            @arg UART0_FLAG_RXNE:  Receive data register not empty flag.
  *            @arg UART0_FLAG_IDLE:  Idle Line detection flag.
  *            @arg UART0_FLAG_ORE:  OverRun Error flag.
  *            @arg UART0_FLAG_NE:  Noise Error flag.
  *            @arg UART0_FLAG_FE:  Framing Error flag.
  *            @arg UART0_FLAG_PE:  Parity Error flag.
  * @retval The new state of UART0_FLAG (SET or RESET).
  */
FlagStatus UART0_GetFlagStatus(uint32_t UART0_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_UART0_FLAG(UART0_FLAG));
  
  if ((UART0->ISR & UART0_FLAG) != (uint16_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

/**
  * @brief  Clears the UART0's pending flags.
  * @param  UART0_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg UART0_FLAG_LBD:  LIN Break detection flag.
  *            @arg UART0_FLAG_TC:  Transmission Complete flag.
  *            @arg UART0_FLAG_IDLE:  IDLE line detected flag.
  *            @arg UART0_FLAG_ORE:  OverRun Error flag.
  *            @arg UART0_FLAG_NE: Noise Error flag.
  *            @arg UART0_FLAG_FE: Framing Error flag.
  *            @arg UART0_FLAG_PE:   Parity Errorflag.
  *   
  * @note     RXNE pending bit is cleared by a read to the UART0_RDR register 
  *           (UART0_ReceiveData()) or by writing 1 to the RXFRQ in the register
  *           UART0_RQR (UART0_RequestCmd()).
  * @note     TC flag can be also cleared by software sequence: a read operation
  *           to UART0_SR register (UART0_GetFlagStatus()) followed by a write 
  *           operation to UART0_TDR register (UART0_SendData()).
  * @note     TXE flag is cleared by a write to the UART0_TDR register (UART0_SendData())
  *           or by writing 1 to the TXFRQ in the register UART0_RQR (UART0_RequestCmd()).
  * @note     SBKF flag is cleared by 1 to the SBKRQ in the register UART0_RQR
  *           (UART0_RequestCmd()).
  * @retval None
  */
void UART0_ClearFlag(uint32_t UART0_FLAG)
{
  /* Check the parameters */
  assert_param(IS_UART0_CLEAR_FLAG(UART0_FLAG));
     
  UART0->ICR = UART0_FLAG;
}

/**
  * @brief  Checks whether the specified UART0 interrupt has occurred or not.
  * @param  UART0_IT: specifies the UART0 interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg UART0_IT_LBD:  LIN Break detection interrupt.
  *            @arg UART0_IT_TXE:  Tansmit Data Register empty interrupt.
  *            @arg UART0_IT_TC:  Transmission complete interrupt.
  *            @arg UART0_IT_RXNE:  Receive Data register not empty interrupt.
  *            @arg UART0_IT_IDLE:  Idle line detection interrupt.
  *            @arg UART0_IT_ORE:  OverRun Error interrupt.
  *            @arg UART0_IT_NE:  Noise Error interrupt.
  *            @arg UART0_IT_FE:  Framing Error interrupt.
  *            @arg UART0_IT_PE:  Parity Error interrupt.
  * @retval The new state of UART0_IT (SET or RESET).
  */
ITStatus UART0_GetITStatus(uint32_t UART0_IT)
{
  uint32_t bitpos = 0, itmask = 0, uart0reg = 0;
  ITStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_UART0_GET_IT(UART0_IT)); 
  
  /* Get the UART0 register index */
  uart0reg = (((uint16_t)UART0_IT) >> 0x08);
  /* Get the interrupt position */
  itmask = UART0_IT & IT_MASK;
  itmask = (uint32_t)0x01 << itmask;
  
  if (uart0reg == 0x01) /* The IT  is in CR1 register */
  {
    itmask &= UART0->CR1;
  }
  else if (uart0reg == 0x02) /* The IT  is in CR2 register */
  {
    itmask &= UART0->CR2;
  }
  else /* The IT  is in CR3 register */
  {
    itmask &= UART0->CR3;
  }
  
  bitpos = UART0_IT >> 0x10;
  bitpos = (uint32_t)0x01 << bitpos;
  bitpos &= UART0->ISR;
  if ((itmask != (uint16_t)RESET)&&(bitpos != (uint16_t)RESET))
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  
  return bitstatus;  
}

/**
  * @brief  Clears the UART0's interrupt pending bits.
  * @param  UART0_IT: specifies the interrupt pending bit to clear.
  *          This parameter can be one of the following values:
  *            @arg UART0_IT_LBD:  LIN Break detection interrupt.
  *            @arg UART0_IT_TC:  Transmission complete interrupt.
  *            @arg UART0_IT_IDLE:  IDLE line detected interrupt.
  *            @arg UART0_IT_ORE:  OverRun Error interrupt.
  *            @arg UART0_IT_NE:  Noise Error interrupt.
  *            @arg UART0_IT_FE:  Framing Error interrupt.
  *            @arg UART0_IT_PE:  Parity Error interrupt.
  *
  * @note     RXNE pending bit is cleared by a read to the UART0_RDR register 
  *           (UART0_ReceiveData()) or by writing 1 to the RXFRQ in the register 
  *           UART0_RQR (UART0_RequestCmd()).
  * @note     TC pending bit can be also cleared by software sequence: a read 
  *           operation to UART0_SR register (UART0_GetITStatus()) followed by  
  *           a write operation to UART0_TDR register (UART0_SendData()).
  * @note     TXE pending bit is cleared by a write to the UART0_TDR register 
  *           (UART0_SendData()) or by writing 1 to the TXFRQ in the register 
  *           UART0_RQR (UART0_RequestCmd()).
  * @retval None
  */
void UART0_ClearITPendingBit(uint32_t UART0_IT)
{
  uint32_t bitpos = 0, itmask = 0;
  /* Check the parameters */
  assert_param(IS_UART0_CLEAR_IT(UART0_IT)); 
  
  bitpos = UART0_IT >> 0x10;
  itmask = ((uint32_t)0x01 << (uint32_t)bitpos);
  UART0->ICR = (uint32_t)itmask;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
