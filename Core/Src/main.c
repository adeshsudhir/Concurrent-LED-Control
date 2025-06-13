
#include "main.h"
#include "cmsis_os.h"
#include "gpio.h"

#include <stdbool.h>

/*
 ------ Variables ------
*/

/*
 ------ Task Handles ------
*/
TaskHandle_t task_a_handle;
TaskHandle_t task_b_handle;
TaskHandle_t task_c_handle;
TaskHandle_t task_d_handle;
TaskHandle_t task_e_handle;

/*
 ------ Semaphore Handles ------
*/
SemaphoreHandle_t led_mutex;


/*
 ------ Tasks ------
*/

void task_a(void *args)
{
  bool laststate = false;

  while(1)
  {

    bool isButtonPressed = HAL_GPIO_ReadPin(user_button_GPIO_Port, user_button_Pin);

    if(isButtonPressed != laststate)
    {
      laststate = isButtonPressed;
      xTaskNotify(task_b_handle, (uint32_t)laststate, eSetValueWithOverwrite);
      xTaskNotify(task_d_handle, (uint32_t)laststate, eSetValueWithOverwrite);
      xTaskNotify(task_e_handle, (uint32_t)laststate, eSetValueWithOverwrite);
      xTaskNotifyGive(task_c_handle);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void task_b(void *args)
{
  uint32_t button_pressed = 0;
  while (1)
  {

    xTaskNotifyWait(0, 0, &button_pressed, portMAX_DELAY);
    if (button_pressed == true)
    {
      HAL_GPIO_WritePin(green_led_GPIO_Port, green_led_Pin, GPIO_PIN_SET);
    }else
    {
      HAL_GPIO_WritePin(green_led_GPIO_Port, green_led_Pin, GPIO_PIN_RESET);
    }
  }
}

void task_c(void *args)
{
  while (1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (uint8_t i = 0; i < 10; i++)
    {
      HAL_GPIO_TogglePin(orange_led_GPIO_Port, orange_led_Pin);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  
}

void task_d(void *args)
{
  uint32_t flag_set = 0;
  while (1)
  {
    xTaskNotifyWait(0, 0, &flag_set, portMAX_DELAY);

    while (flag_set)
    {
      if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(10)))
      {
        HAL_GPIO_TogglePin(red_led_GPIO_Port, red_led_Pin);
        xSemaphoreGive(led_mutex);
      }
      vTaskDelay(pdMS_TO_TICKS(100));
      
      xTaskNotifyWait(0, 0, &flag_set, 0); 
    }

    if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(10)))
    {
      HAL_GPIO_WritePin(red_led_GPIO_Port, red_led_Pin, GPIO_PIN_RESET);
      xSemaphoreGive(led_mutex);
    }
  }
}

void task_e(void *args)
{
  uint32_t button_pressed;
  bool cantoggle = true;

  while (1)
  {
    if (xTaskNotifyWait(0, 0, &button_pressed, 0) == pdPASS)
    {
      cantoggle = (button_pressed == 0);
    }

    if (cantoggle)
    {
      if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(10)))
      {
        HAL_GPIO_TogglePin(blue_led_GPIO_Port, blue_led_Pin);
        xSemaphoreGive(led_mutex);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /*
   ------ Mutex Initialization ------
  */
  led_mutex = xSemaphoreCreateMutex();

  /*
  ----- initializing tasks -----
  */
  xTaskCreate(task_a, "task a", 128, NULL, 5, &task_a_handle);
  xTaskCreate(task_b, "task b", 128, NULL, 5, &task_b_handle);
  xTaskCreate(task_c, "task c", 128, NULL, 5, &task_c_handle);
  xTaskCreate(task_d, "task d", 128, NULL, 10, &task_d_handle);
  xTaskCreate(task_e, "task e", 128, NULL, 5, &task_e_handle);

  // Start Scheduler
  vTaskStartScheduler();

  while (1)
  {

  }

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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
