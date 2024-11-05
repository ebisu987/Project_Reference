#include "main.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"

ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osThreadId myTask03Handle;
osThreadId myTask04Handle;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);

#define SLAVE_ADDRESS_PFC 0x27<<1

void LCD_I2C_Write_CMD(char cmd)
{
	//uint8_t buf[4]={(data&(0xF0))|0x04,(data&(0xF0))|0x00,(data<<4)|0x04,(data<<4)|0x00};
	char data_u, data_l;
	uint8_t data_t[4];
	
	data_u = (cmd&0xf0);
	data_l = (cmd<<4)&0xf0;
	data_t[0] = data_u|0x0C;	//en=1, rs = 0
	data_t[1] = data_u|0x08;	//en=0, rs = 0
	data_t[2] = data_l|0x0C;	
	data_t[3] = data_l|0x08;	
	HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_PFC,(uint8_t *) data_t, 4, 100);
}
void LCD_I2C_Write_DATA(char data)
{
	//uint8_t buf[4]={(data&(0xF0))|0x05,(data&(0xF0))|0x01,(data<<4)|0x05,(data<<4)|0x01};
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = (data<<4)&0xf0;
	data_t[0] = data_u|0x0D; //en=1, rs = 1
	data_t[1] = data_u|0x09; //en=0, rs = 1
	data_t[2] = data_l|0x0D;	
	data_t[3] = data_l|0x09;		
	HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_PFC,(uint8_t *) data_t, 4, 100);
}
void LCD_I2C_Init()
{
	LCD_I2C_Write_CMD (0x33); /* set 4-bits interface */
	LCD_I2C_Write_CMD (0x32);
	HAL_Delay(50);
	LCD_I2C_Write_CMD (0x28); /* start to set LCD function */
	HAL_Delay(50);
	LCD_I2C_Write_CMD (0x01); /* clear display */
	HAL_Delay(50);
	LCD_I2C_Write_CMD (0x06); /* set entry mode */
	HAL_Delay(50);
	LCD_I2C_Write_CMD (0x0c); /* set display to on */	
	HAL_Delay(50);
	LCD_I2C_Write_CMD(0x02); /* move cursor to home and set data address to 0 */
	HAL_Delay(50);
	LCD_I2C_Write_CMD(0x80);
}
void LCD_I2C_Clear()
{
	LCD_I2C_Write_CMD(0x01);		//xoa toan man hinh
	HAL_Delay(2);	
}
void LCD_I2C_Goto_XY(int row, int col)
{
	uint8_t pos_addr;
	if(row == 1) 
	{
		pos_addr = 0x80 + row - 1 + col;
	}
	else
	{
		pos_addr = 0x80 | (0x40 + col);
	}
	LCD_I2C_Write_CMD(pos_addr);	
}
void LCD_I2C_Write_String(char* string)
{
	while(*string) LCD_I2C_Write_DATA(*string++); 
}
void LCD_I2C_Write_Number(int number)
{
	char buffer[8];
	sprintf(buffer,"%d",number);
	LCD_I2C_Write_String(buffer);
}

int ADC_VAL;

int main(void)
{


  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
	
	LCD_I2C_Init();
	LCD_I2C_Clear();

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityHigh, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, StartTask02, osPriorityNormal, 0, 128);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* definition and creation of myTask03 */
  osThreadDef(myTask03, StartTask03, osPriorityNormal, 0, 128);
  myTask03Handle = osThreadCreate(osThread(myTask03), NULL);

  /* definition and creation of myTask04 */
  osThreadDef(myTask04, StartTask04, osPriorityNormal, 0, 128);
  myTask04Handle = osThreadCreate(osThread(myTask04), NULL);

	
  /* add threads, ... */

  osKernelStart();


  while (1)
  {
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA3 PA4 PA5 PA6
                           PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void DisplaySpeed(ADC_VAL)
{
	if(ADC_VAL > 0 && ADC_VAL < 2)
	{
			LCD_I2C_Goto_XY(0,0);
			LCD_I2C_Write_String("=");	
	}		
	for(int i=0;i<ADC_VAL/200;i++)
	{
			LCD_I2C_Goto_XY(0,i);
			LCD_I2C_Write_String("=");			
	}
}

void SparkSwitch(int delayMilisec)
{
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_3);	
		osDelay(delayMilisec/4);
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_4);	
		osDelay(delayMilisec/4);
		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);	
		osDelay(delayMilisec/4);
		HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);	
		osDelay(delayMilisec/4);
}

void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 1000);
    ADC_VAL = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);	
	
		LCD_I2C_Clear();
		LCD_I2C_Goto_XY(1,8);
		LCD_I2C_Write_String("RPM:");		
		LCD_I2C_Goto_XY(1,12);
		LCD_I2C_Write_Number(ADC_VAL);
	
		DisplaySpeed(ADC_VAL);
		
		if(ADC_VAL > 0 && ADC_VAL<8) 
		{	
			
			osThreadSuspend(myTask02Handle);
			osThreadSuspend(myTask03Handle);
			osThreadSuspend(myTask04Handle);
			
		}
		
		if(ADC_VAL>10 && ADC_VAL<1000)
		{
			osThreadSuspend(myTask03Handle);
			osThreadSuspend(myTask04Handle);
			
			osThreadResume(myTask02Handle);
		}
		
		else if(ADC_VAL>1100 && ADC_VAL <2000)
		{
			osThreadSuspend(myTask02Handle);
			osThreadSuspend(myTask04Handle);
			
			osThreadResume(myTask03Handle);
		}
		
		
		else if(ADC_VAL>2100 && ADC_VAL <5000)
		{
			osThreadSuspend(myTask02Handle);
			osThreadSuspend(myTask03Handle);
			
			osThreadResume(myTask04Handle);
		}		
		
		
		osDelay(1000);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
			if(ADC_VAL<5)
			{
				LCD_I2C_Goto_XY(0,0);
				LCD_I2C_Write_String("Car stop");
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);		
							
			}
			if(ADC_VAL>5 && ADC_VAL<1000)
			{
				LCD_I2C_Goto_XY(1,0);
				LCD_I2C_Write_String("Gear:1");
				SparkSwitch(400);
			}
	
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
		LCD_I2C_Goto_XY(1,0);
		LCD_I2C_Write_String("Gear:2");
		SparkSwitch(150);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the myTask04 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  for(;;)
  {
		 if(ADC_VAL>2100 && ADC_VAL <3000)
		 {
				LCD_I2C_Goto_XY(1,0);
				LCD_I2C_Write_String("Gear:3");
				SparkSwitch(80);	 
		 }
		 else	if(ADC_VAL>3010 && ADC_VAL <5000)
		 {
				LCD_I2C_Goto_XY(1,0);
				LCD_I2C_Write_String("Gear:4");
				SparkSwitch(40);	 
		 }

  }
  /* USER CODE END StartTask04 */
}


/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
