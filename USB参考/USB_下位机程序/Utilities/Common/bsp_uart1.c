#include "bsp_uart1.h"
#include "wb7720.h"

/**
  * @name    uart1_init
  * @brief   初始化UART1
  * @param   apbclk: UART1所在的APB总线时钟频率
  * @param   baud: 要配置的波特率
  * @retval  无
  */
void uart1_init(uint32_t apbclk, uint32_t baud)
{
  uint32_t divider;

  RCC->APBENR |= RCC_APBENR_GPIOEN | RCC_APBENR_UART1EN;

  RCC->APBRSTR |= RCC_APBRSTR_UART1RST;
  RCC->APBRSTR &= ~RCC_APBRSTR_UART1RST;

  /*
    PA14 (UART1_TX)
    PA15 (UART1_RX)
  */
  GPIO_Init(GPIOA, GPIO_Pin_14 |GPIO_Pin_15, GPIO_MODE_AF |GPIO_OTYPE_PP |GPIO_PUPD_UP |GPIO_SPEED_HIGH |GPIO_AF2);
  divider = (apbclk + (baud >> 1)) / baud;    // 整型除法四舍五入
  UART1->BDR = divider;
  UART1->CR |= UART1_CR_TXE | UART1_CR_RXE;
}

/**
  * @name    uart1_send
  * @brief   通过UART1发送数据
  * @param   buffer: 要发送的数据缓冲区
  * @param   length: 要发送的数据长度
  * @retval  无
  */
void uart1_send(const uint8_t* buffer, uint32_t length)
{
  while(length)
  {
    while (UART1->SR & UART1_SR_TXF);
    UART1->DR = *buffer;
    buffer++;  length--;
  }
  while (UART1->SR & UART1_SR_TXF);
}

