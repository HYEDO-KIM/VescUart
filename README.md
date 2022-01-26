# VescUart for stm32CubeIDE -C-

stm32CubeIDE C library for interfacing with a VESC over UART. This library is based upon the library written by \
RollingGecko (https://github.com/RollingGecko/VescUartControl) and \
SolidGeek (https://github.com/SolidGeek/VescUart).

**Important:** You will have to make some changes to your software, see below.

## Implementation

To use the library you will have use initVesc(); and set the UART communcation.

```c
#include "../../VescUart/src/crc.h"
#include "../../VescUart/src/crc.cpp"
#include "../../VescUart/src/buffer.cpp"
#include "../../VescUart/src/buffer.h"
#include "../../VescUart/src/datatypes.h"
#include "../../VescUart/src/VescUart.c"
#include "../../VescUart/src/VescUart.h"

/* USER CODE BEGIN PD */
#define RX_BUFFER_SIZE 300
/* USER CODE END PD */

/* USER CODE BEGIN PV */
uint8_t rxBuffer[RX_BUFFER_SIZE];
VescUart vesc;
float set_Current = 0;
float set_Duty = 0;
float set_RPM = 0;



/* USER CODE END PV */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	initVesc(&vesc, 1, UART4);
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
Then you have to edit 'void UART4_IRQHandler(void){}', declare extern variable and vesc structure in 'stm32f4xx_it.c file'\
Data received from the vesc should be placed in the packet[] of the vesc structure.

```c
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RX_BUFFER_SIZE 256
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern VescUart vesc;
extern uint8_t rxBuffer[RX_BUFFER_SIZE];
/* USER CODE END PV */

void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */
	if(LL_USART_IsActiveFlag_IDLE(UART4)) {

		int len = RX_BUFFER_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_2);
		vesc.length=len;
		pushVescPacket(&vesc, rxBuffer, len);
		//pushPacket(&serialLine, rxBuffer, len);

		LL_USART_ClearFlag_IDLE(UART4);
		LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_2);
		LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_2, RX_BUFFER_SIZE);
		LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_2);

		LL_USART_EnableDMAReq_RX(UART4);
		cnt++;
	}

	if(LL_USART_IsActiveFlag_ORE(UART4)) LL_USART_ClearFlag_ORE(UART4);

  /* USER CODE END UART4_IRQn 0 */
  /* USER CODE BEGIN UART4_IRQn 1 */

  /* USER CODE END UART4_IRQn 1 */
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
		//setCurrent(&vesc, set_Current);
		//setDuty(&vesc, set_Duty);
		//setRPM(&vesc, set_RPM);
	}
}

/* USER CODE END 4 */
```
