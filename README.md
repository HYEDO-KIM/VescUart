# VescUart-stm32-C-

stm32CubeIDE C library for interfacing with a VESC over UART. This library is based upon the library written by RollingGecko (https://github.com/RollingGecko/VescUartControl) and SolidGeek (https://github.com/SolidGeek/VescUart).

**Important:** You will have to make some changes to your software, see below.

## Implementation

To use the library you will have initiate the VescUart class and set the Serial port for UART communcation.

```c
#include "../../VescUart/src/crc.h"
#include "../../VescUart/src/crc.cpp"
#include "../../VescUart/src/buffer.cpp"
#include "../../VescUart/src/buffer.h"
#include "../../VescUart/src/datatypes.h"
#include "../../VescUart/src/VescUart.c"
#include "../../VescUart/src/VescUart.h"

int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	initVesc(&vesc, 2, (uint8_t*)&fc, sizeof(FlipperController), UART4);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_TIM6_Init();
    MX_UART4_Init();
  /* USER CODE BEGIN 2 */

	HAL_TIM_Base_Start_IT(&htim6);
  
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_2, rxBuffer);
	LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_2, &UART4->DR);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_2, RX_BUFFER_SIZE);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);
	LL_USART_EnableDMAReq_RX(UART4);
	LL_USART_EnableIT_IDLE(UART4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}
```

You can now safely use the functions and change the values. 

Getting VESC telemetry is easy:

```c
/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6) { 
		getVescValues(&vesc);
		//setCurrent(set_current);
		//setDuty(set_current);
		//setRPM(&vesc, current);
	}
}

/* USER CODE END 4 */
```
