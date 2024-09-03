/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "debug.h"
#include <stdio.h>
#include <string.h>
#include "flash_ops.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define B1_GPIO_Port   GPIOC
#define B1_Pin         GPIO_PIN_13
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static const char *TAG = "GSM";
volatile uint32_t timer_cnt = 0;
char OK_STR[] = "OK\r\n";
char ERROR_STR[] = "ERROR\r\n";

const uint32_t OTA_FW_CHUNK_SIZE  = 1024;
uint8_t rx_byte;
uint16_t rx_idx;
volatile uint8_t rx_complete = 0;
char uart_rx_buff[2048];

const char root_ca[] = "-----BEGIN CERTIFICATE-----"
		"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF"
		"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6"
		"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL"
		"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv"
		"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj"
		"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM"
		"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw"
		"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6"
		"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L"
		"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm"
		"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC"
		"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA"
		"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI"
		"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs"
		"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv"
		"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU"
		"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy"
		"rqXRfboQnoZsG4q5WTP468SQvvG5"
		"-----END CERTIFICATE-----";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM7_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
void jump_to_app(void);
void Send_AT(char *cmd);
uint8_t check_response(uint32_t time_out);
uint8_t check_response_with_arg(uint32_t time_out, char *r1, char *r2);
uint8_t gsm_modem_check(void);
void gsm_modem_init(void);
void gsm_gprs_init(void);
void gsm_http_get(void);
void print_rx_buffer(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef enum {
	AT_OK, AT_ERROR, AT_TIMEOUT, AT_R1, AT_R2
} AT_ErrorCode_t;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_TIM7_Init();
	MX_TIM6_Init();
	/* USER CODE BEGIN 2 */
	DEBUG_LOG("BL", "Bootloader v0.4");
	DEBUG_LOG("BL", "Press user button for firmware upgrade");
	uint8_t btn_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
	uint32_t last_tick = HAL_GetTick();

	while ((HAL_GetTick() - last_tick) < 3000) {
		btn_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
		if (btn_state == 0) {
			break;
		}
		HAL_Delay(100);
	}

	HAL_Delay(100);
	btn_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

	if (btn_state == 0) {
		DEBUG_LOG("BL", "loading app");

		HAL_TIM_Base_MspDeInit(&htim6);
		HAL_TIM_Base_MspDeInit(&htim7);
		HAL_UART_MspDeInit(&huart2);
		HAL_UART_MspDeInit(&huart1);
		__HAL_RCC_GPIOC_CLK_DISABLE();
		__HAL_RCC_GPIOH_CLK_DISABLE();
		__HAL_RCC_GPIOA_CLK_DISABLE();
		HAL_RCC_DeInit();
		HAL_DeInit();
		jump_to_app();
	}
	Send_AT("ATE0\r\n");
	DEBUG_LOG(TAG, "ATE0\r\n");
	HAL_TIM_Base_Start_IT(&htim7);
	HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
	if (gsm_modem_check() != AT_OK) {
		DEBUG_LOG(TAG, "Check Modem Connection or Power");
		while (1)
			;
	}
	gsm_modem_init();
	gsm_gprs_init();
	gsm_http_get();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
	RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1
			| RCC_PERIPHCLK_USART2;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void) {

	/* USER CODE BEGIN TIM6_Init 0 */

	/* USER CODE END TIM6_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM6_Init 1 */

	/* USER CODE END TIM6_Init 1 */
	htim6.Instance = TIM6;
	htim6.Init.Prescaler = 31999;
	htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim6.Init.Period = 50;
	htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim6) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM6_Init 2 */

	/* USER CODE END TIM6_Init 2 */

}

/**
 * @brief TIM7 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM7_Init(void) {

	/* USER CODE BEGIN TIM7_Init 0 */

	/* USER CODE END TIM7_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM7_Init 1 */

	/* USER CODE END TIM7_Init 1 */
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 31999;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 65535;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM7_Init 2 */

	/* USER CODE END TIM7_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LD2_Pin */
	GPIO_InitStruct.Pin = LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void jump_to_app(void) {
	void (*app_reset_handler)(void);

	SysTick->LOAD = 0;
	SysTick->CTRL = 0;
	SysTick->VAL = 0;
	// get stack pointer of application
	// required if reset handelr in app is not setting sp
//	uint32_t sp_val = (*(volatile uint32_t *)(APP_START_ADDR));
//	__set_MSP(sp_val);
	uint32_t reset_handler_address =
			(*(volatile uint32_t*) (APP_AREA_START_ADDR + 4));
	app_reset_handler = (void*) reset_handler_address;
	app_reset_handler();
}

void Send_AT(char *cmd) {
	HAL_UART_Transmit(&huart1, (uint8_t*) cmd, strlen(cmd), 1000);
	DEBUG_LOG(TAG, cmd);
}

void print_rx_buffer(void) {
	HAL_UART_Transmit(&huart2, (uint8_t *)uart_rx_buff, rx_idx, 1000);
}

uint8_t check_response(uint32_t time_out) {
	rx_idx = 0;
	rx_complete = 0;
	memset(uart_rx_buff, 0, sizeof(uart_rx_buff));
	AT_ErrorCode_t ret = AT_TIMEOUT;
	while (!rx_complete)
		;
	uint32_t t1 = HAL_GetTick();
	do {
		if (strstr(uart_rx_buff, "\r\nOK\r\n") > 0) {
			ret = AT_OK;
			break;
		} else if (strstr(uart_rx_buff, "\r\nERROE\r\n") > 0) {
			ret = AT_ERROR;
			break;
		}
		HAL_Delay(10);
	} while ((HAL_GetTick() - t1) < time_out);
	//DEBUG_LOG(TAG, uart_rx_buff);
//	HAL_UART_Transmit(&huart2,uart_rx_buff,rx_idx, 1000);
	rx_complete = 0;
	return ret;
}

uint8_t check_response_with_arg(uint32_t time_out, char *r1, char *r2) {
	rx_idx = 0;
	rx_complete = 0;
	memset(uart_rx_buff, 0, sizeof(uart_rx_buff));

	AT_ErrorCode_t ret = AT_TIMEOUT;
	while (!rx_complete)
		;
	uint32_t t1 = HAL_GetTick();
	do {
		if (strstr(uart_rx_buff, r1) > 0) {
			ret = AT_R1;
			break;
		} else if (strstr(uart_rx_buff, r2) > 0) {
			ret = AT_R2;
			break;
		}
		HAL_Delay(100);
	} while ((HAL_GetTick() - t1) < time_out);
//	DEBUG_LOG(TAG, uart_rx_buff);
	return ret;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		uart_rx_buff[rx_idx++] = rx_byte;
		HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
		__HAL_TIM_SetCounter(&htim6, 0);
		HAL_TIM_Base_Start_IT(&htim6);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		HAL_TIM_Base_Stop_IT(&htim6);
		rx_complete = 1;
	} else if (htim->Instance == TIM7) {
		if (timer_cnt > 0)
			timer_cnt--;
	}
}

//AT+SAPBR=3,1,"Contype","GPRS"	Configure bearer profile 1	OK
//AT+SAPBR=3,1,"APN","CMNET"	Configure the bearer profile APN	OK
//AT+SAPBR=1,1	Open a GPRS context	OK
//AT+SAPBR=2,1	Query a GPRS context	OK
//AT+SAPBR=0,1	Close the GPRS context	OK
//AT+HTTPINIT	Check the HTTP connection status	OK
//AT+HTTPPARA="CID",1	Set parameters for HTTP session	OK
//AT+HTTPPARA="URL","www.sim.com"	Set parameters for HTTP session	OK
//AT+HTTPACTION=0	GET session start	OK
//AT+HTTPREAD	Read the data of the HTTP server	OK
//AT+HTTPTERM	End HTTP service	OK
//if need server authentication, please set AT+SSLOPT=0,0
//If do not need server authentication, please set AT+SSLOPT=0,1
//If need client authentication, please set AT+SSLOPT=1,1
//If do not need client authentication, please

void set_ssl() {
	AT_ErrorCode_t ret;
	Send_AT("AT+SSLOPT=0,1\r\n");
	ret = check_response(2000);

	Send_AT("AT+HTTPSSL=1\r\n");
	ret = check_response(1000);

	Send_AT("AT+SSLSETROOT=\"C:\\USER\\aws_root.CRT\",1206\r\n");
	ret = check_response(1000);
	(void)ret;
}

void gsm_gprs_init(void) {
	AT_ErrorCode_t ret = AT_ERROR;
	uint8_t retry_count = 3;

	Send_AT("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
	ret = check_response(20000);
	print_rx_buffer();

	Send_AT("AT+SAPBR=3,1,\"APN\",\"internet\"\r\n");
	ret = check_response(20000);
	print_rx_buffer();

	Send_AT("AT+SAPBR=0,1\r\n");
	ret = check_response(20000);
	print_rx_buffer();

	Send_AT("AT+SAPBR=1,1\r\n");
	ret = check_response(85000);
	print_rx_buffer();

	while (retry_count) {
		Send_AT("AT+SAPBR=2,1\r\n");
		ret = check_response_with_arg(30000, "\r\n+SAPBR: ", "\r\nOK");
		HAL_Delay(1000);
		if (ret == AT_R1 || ret == AT_R2) {
			char *ptr = strstr(uart_rx_buff, "+SAPBR: ");
			HAL_Delay(100);
			int n1, n2;
			char name[512];
			char ip[16] = { 0 };
			int result = sscanf(ptr, "+SAPBR: %d,%d,%s", &n1, &n2, ip);
			(void)result;
			sprintf(name, "n1 %d, n2 %d ip:%s", n1, n2, ip);
			DEBUG_LOG(TAG, name);
			if (strstr(ip, "0.0.0.0") > 0) {
				retry_count--;
			} else {
				retry_count = 3;
				break;
			}
		} else {
			retry_count--;
		}
	}
}

void gsm_http_get(void) {
	AT_ErrorCode_t ret = AT_TIMEOUT;
	Send_AT("AT+HTTPTERM\r\n");
	ret = check_response(1000);
	print_rx_buffer();

	Send_AT("AT+HTTPINIT\r\n");
	ret = check_response(20000);
	print_rx_buffer();

	Send_AT("AT+HTTPPARA=\"CID\",1\r\n");
	ret = check_response(10000);
	print_rx_buffer();
//	Send_AT("AT+HTTPPARA=\"REDIR\",1\r\n");
//	ret = check_response(1000);

	Send_AT("AT+HTTPPARA=\"CONTENT\",\"application/octet-stream\"\r\n");
	ret = check_response(10000);
	print_rx_buffer();
//	Send_AT("AT+HTTPPARA=\"URL\",\"https://stm-ota.s3.amazonaws.com/STM32L073_Application.bin\"\r\n");
	Send_AT(
			"AT+HTTPPARA=\"URL\",\"http://ec2-34-229-18-159.compute-1.amazonaws.com/ota/STM32L073_Application.bin\"\r\n");
	ret = check_response(10000);
	print_rx_buffer();

	Send_AT("AT+HTTPACTION=0\r\n");
	ret = check_response(60000);
	print_rx_buffer();

	uint32_t fw_len;
	uint32_t http_code;
	while (1) {
		HAL_Delay(10);
		if (strstr(uart_rx_buff, "\r\n+HTTPACTION: 0,") > 0) {
			char *ptr = strstr(uart_rx_buff, "+HTTPACTION: ");
			HAL_Delay(100);
			int n;
			char name[512] = { 0 };
			sscanf(ptr, "+HTTPACTION: %d,%ld,%ld", &n, &http_code,
					&fw_len);

			sprintf(name, "n %d, http_code: %ld, fw_len: %ld", n, http_code,
					fw_len);
			DEBUG_LOG(TAG, name);
			DEBUG_LOG(TAG, uart_rx_buff);
			break;
		}
	}

	if (http_code == 200) {
#if 1
		Erasse_Flash(ERASE_APP_AREA);
		uint32_t remain_byte = fw_len;
		uint32_t start_addr = 0;
		uint32_t read_size = OTA_FW_CHUNK_SIZE;
		uint32_t app_addr = APP_AREA_START_ADDR;
#else
		Erasse_Flash(ERASE_DL_AREA);
		uint32_t remain_byte = fw_len;
		uint32_t start_addr = 0;
		uint32_t read_size = OTA_FW_CHUNK_SIZE;
		uint32_t app_addr = DL_AREA_START_ADDR;
#endif

		while (remain_byte) {
			char buff[30];
			sprintf(buff, "AT+HTTPREAD=%ld,%ld\r\n", start_addr, read_size);
			Send_AT(buff);
			ret = check_response_with_arg(50000, "+HTTPREAD:", "\r\nERROR");
			//print_rx_buffer();
			if (ret != AT_R1)
				break;
			uint32_t timer_cnt1 = HAL_GetTick();
			uint32_t diff;
			while (HAL_GetTick() - timer_cnt1 < 5000) {
				char *ptr = strstr(uart_rx_buff, "+HTTPREAD: ");
				if (ptr > 0 && rx_idx >= read_size + 20) {
					int received_len = 0;
					sscanf(ptr, "+HTTPREAD: %d", &received_len);
					if (received_len == read_size) {
						char *ptr2 = strstr(ptr, "\r\n");
						DEBUG_LOG(TAG, "got +HTTPREAD: ");
						diff = ptr2 - ptr;
						if (diff > 12) {
							HAL_Delay(500);
							HAL_UART_Transmit(&huart2, (uint8_t *)&uart_rx_buff[diff + 4],
									read_size, 1000);
							write_to_flash(app_addr, (uint8_t *)&uart_rx_buff[diff + 4],
									read_size);
							break;
						}
					}
				}
			}
			start_addr += read_size;
			remain_byte -= read_size;
			app_addr += read_size;
			if (remain_byte < read_size)
				read_size = remain_byte;
			sprintf(buff, "Remain=%ld,addr:%ld\r\n", remain_byte, start_addr);
			DEBUG_LOG(TAG, buff);
			HAL_Delay(2000);
		}
	}

#if 0

	Send_AT("AT+HTTPREAD\r\n");
	ret = check_response(5000);
	timer_cnt = 5000;
	while (timer_cnt) {
		HAL_Delay(10);
		if (strstr(uart_rx_buff, "+HTTPREAD: ") > 0) {
			DEBUG_LOG(TAG, uart_rx_buff);
			break;
		}
	}
	timer_cnt = 5000;
	while (timer_cnt) {
		if (strstr(uart_rx_buff, "OK") > 0) {
			break;
		}
		DEBUG_LOG(TAG, uart_rx_buff);
		HAL_Delay(10);
	}
#endif

	Send_AT("AT+HTTPTERM\r\n");
	ret = check_response(10000);
	print_rx_buffer();
	(void) ret;
}

uint8_t gsm_modem_check(void) {
	AT_ErrorCode_t ret = AT_TIMEOUT;
	for (uint8_t cnt = 0; cnt < 3; cnt++) {
		Send_AT("ATE0\r\n");
		ret = check_response(10000);
		print_rx_buffer();
		if (ret == AT_OK) {
			break;
		}
	}
	return ret;
}

void gsm_modem_init(void) {
	AT_ErrorCode_t ret = AT_TIMEOUT;
	Send_AT("AT+CPIN?\r\n");
	ret = check_response_with_arg(5000, "+CPIN: READY", "\r\nERROR");
	print_rx_buffer();
	if (ret == AT_R1) {
		DEBUG_LOG(TAG, "SIM Reday");
	} else {
		return;
	}

	Send_AT("AT+CSQ\r\n");
	ret = check_response_with_arg(5000, "+CSQ: ", "\r\nOK");
	print_rx_buffer();
	if (ret != AT_R1)
		return;

	Send_AT("AT+CREG?\r\n");
	ret = check_response_with_arg(5000, "+CREG: 0,1", "+CREG: 0,5");
	print_rx_buffer();
	if (ret != AT_R1 || ret != AT_R2 || ret == AT_TIMEOUT)
		return;

	Send_AT("AT+CGATT?\r\n");
	ret = check_response_with_arg(5000, "+CGATT: 1", "\r\nERROR");
	print_rx_buffer();
	if (ret != AT_R1 || ret != AT_R2 || ret == AT_TIMEOUT)
		return;

	Send_AT("AT+CGATT=1\r\n");
	ret = check_response(10000);
	print_rx_buffer();
	(void) ret;
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
