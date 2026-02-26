#include "bsp_uart0.h"
#include "wb7720.h"

/**
  * @name    uart0_init
  * @brief   初始化UART0
  * @param   apbclk: UART0所在的APB总线时钟频率
  * @param   baud: 要配置的波特率
  * @retval  无
  */
void uart0_init(uint32_t apbclk, uint32_t baud)
{
  uint32_t divider;

  RCC->APBENR |= RCC_APBENR_GPIOEN | RCC_APBENR_UART0EN;

  RCC->APBRSTR |= RCC_APBRSTR_UART0RST;
  RCC->APBRSTR &= ~RCC_APBRSTR_UART0RST;

  /*
    PA2 (UART0_TX) - Debug print only, no RX needed
  */
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_MODE_AF |GPIO_OTYPE_PP |GPIO_PUPD_UP |GPIO_SPEED_HIGH |GPIO_AF2);
  divider = (apbclk + (baud >> 1)) / baud;    // 整型除法四舍五入
  UART0->BRR = divider;
  UART0->CR1 = UART0_CR1_TE | UART0_CR1_RE | UART0_CR1_UE;
}

/**
  * @name    uart0_send
  * @brief   通过UART0发送数据
  * @param   buffer: 要发送的数据缓冲区
  * @param   length: 要发送的数据长度
  * @retval  无
  */
void uart0_send(const uint8_t* buffer, uint32_t length)
{
  while(length)
  {
    while((UART0->ISR & UART0_ISR_TXE) == 0);
    UART0->TDR = *buffer;
    buffer++;  length--;
  }
  while((UART0->ISR & UART0_ISR_TC) == 0);
}

