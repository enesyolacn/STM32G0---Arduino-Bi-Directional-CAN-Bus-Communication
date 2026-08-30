/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MCP_WRITE       0x02
#define MCP_READ        0x03
#define MCP_RESET       0xC0
#define MCP_CANSTAT     0x0E
#define MCP_CANCTRL     0x0F
#define MCP_CNF1        0x2A
#define MCP_CNF2        0x29
#define MCP_CNF3        0x28
#define MCP_RXB0CTRL    0x60
#define MCP_RXB0D0      0x66
#define MCP_CANINTF     0x2C
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
uint8_t rxData[8] = {0};
uint8_t msgReceived = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

/*USER CODE BEGIN PFP*/
void MCP2515_CS_Low(void);
void MCP2515_CS_High(void);
void MCP2515_WriteRegister(uint8_t address, uint8_t value);
uint8_t MCP2515_ReadRegister(uint8_t address);
void MCP2515_Reset(void);
uint8_t MCP2515_Init(void);
uint8_t MCP2515_ReadMessage(uint8_t *data);
void MCP2515_SendMessage(uint16_t id, uint8_t *data, uint8_t len);

// 2. Fonksiyon Gövdeleri
void MCP2515_CS_Low(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}

void MCP2515_CS_High(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
}

void MCP2515_WriteRegister(uint8_t address, uint8_t value) {
    uint8_t txData[3] = {MCP_WRITE, address, value};
    MCP2515_CS_Low();
    HAL_SPI_Transmit(&hspi1, txData, 3, HAL_MAX_DELAY);
    MCP2515_CS_High();
}

uint8_t MCP2515_ReadRegister(uint8_t address) {
    uint8_t txData[2] = {MCP_READ, address};
    uint8_t rxValue = 0;
    MCP2515_CS_Low();
    HAL_SPI_Transmit(&hspi1, txData, 2, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &rxValue, 1, HAL_MAX_DELAY);
    MCP2515_CS_High();
    return rxValue;
}

void MCP2515_Reset(void) {
    uint8_t txData = MCP_RESET;
    MCP2515_CS_Low();
    HAL_SPI_Transmit(&hspi1, &txData, 1, HAL_MAX_DELAY);
    MCP2515_CS_High();
    HAL_Delay(10);
}

uint8_t MCP2515_Init(void) {
    MCP2515_Reset();

    uint8_t status = MCP2515_ReadRegister(MCP_CANSTAT);
    if ((status & 0xE0) != 0x80) return 0;

    // 8MHz Osilatör, 500kbps Baud Rate
    MCP2515_WriteRegister(MCP_CNF1, 0x00);
    MCP2515_WriteRegister(MCP_CNF2, 0x90);
    MCP2515_WriteRegister(MCP_CNF3, 0x02);

    MCP2515_WriteRegister(MCP_RXB0CTRL, 0x60); // Filtresiz tüm mesajları al
    MCP2515_WriteRegister(MCP_CANCTRL, 0x00);  // Normal Moda geç

    status = MCP2515_ReadRegister(MCP_CANSTAT);
    if ((status & 0xE0) == 0x00) return 1;
    return 0;
}

uint8_t MCP2515_ReadMessage(uint8_t *data) {
    uint8_t interruptStatus = MCP2515_ReadRegister(MCP_CANINTF);
    if (interruptStatus & 0x01) {
        for (int i = 0; i < 8; i++) {
            data[i] = MCP2515_ReadRegister(MCP_RXB0D0 + i);
        }
        MCP2515_WriteRegister(MCP_CANINTF, interruptStatus & ~0x01);
        return 1;
    }
    return 0;
}

void MCP2515_SendMessage(uint16_t id, uint8_t *data, uint8_t len) {
    MCP2515_WriteRegister(0x31, (uint8_t)(id >> 3));         // ID High
    MCP2515_WriteRegister(0x32, (uint8_t)((id & 0x07) << 5)); // ID Low
    MCP2515_WriteRegister(0x35, len & 0x0F);                  // Data Length

    for (uint8_t i = 0; i < len; i++) {
        MCP2515_WriteRegister(0x36 + i, data[i]);             // Data Payload
    }

    MCP2515_CS_Low();
    uint8_t rtsCmd = 0x81; // RTS TXB0
    HAL_SPI_Transmit(&hspi1, &rtsCmd, 1, HAL_MAX_DELAY);
    MCP2515_CS_High();
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  MCP2515_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (MCP2515_ReadMessage(rxData)) {
	          msgReceived = 1;

	          // Yanıt paketimiz
	          uint8_t responseData[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x01, 0x02, 0x03, 0x04};

	          HAL_Delay(50);
	          MCP2515_SendMessage(0x321, responseData, 8); // Arduino'ya 0x321 ID'si ile yanıt at
	      }

	      HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  **/
void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
