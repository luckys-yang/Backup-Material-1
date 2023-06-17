//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//�о�԰����
//���̵�ַ��http://shop73023976.taobao.com
//
//  �� �� ��   : main.c
//  �� �� ��   : v2.0
//  ��    ��   : HuangKai
//  ��������   : 2018-10-31
//  ����޸�   : 
//  ��������   : STM32F303RC������
//              ˵��: 
//              ----------------------------------------------------------------
//              GND    ��Դ��
//              VCC  ��5V��3.3v��Դ
//              D0   ��PA5��SCL��
//              D1   ��PA7��SDA��
//              DC    ��PD2
//              CS1   ��PB5
//              FS0   ��PA4
//              CS2   ��PB6
//              ----------------------------------------------------------------
// �޸���ʷ   :
// ��    ��   : 
// ��    ��   : HuangKai
// �޸�����   : �����ļ�
//��Ȩ���У�����ؾ���
//Copyright(C) �о�԰����2018-10-31
//All rights reserved
#include "main.h"
#include "stm32f3xx_hal.h"
#include "oled.h"
#include "bmp.h"


void SystemClock_Config(void);
int main()
{
	u8 t=' ';
	HAL_Init();
	SystemClock_Config();
	OLED_Init();
	OLED_ColorTurn(0);//0������ʾ��1 ��ɫ��ʾ
  OLED_DisplayTurn(0);//0������ʾ 1 ��Ļ��ת��ʾ
	while(1)
	{
		OLED_Display_128x64(bmp1);
		HAL_Delay(500);
		OLED_Display_GB2312_string(0,0,"12864,�������ֿ�");	/*�ڵ�1ҳ����1�У���ʾһ��16x16�����ֻ�8x16��ASCII��*/
		OLED_Display_GB2312_string(0,2,"16X16���庺�ֿ�,");  /*��ʾһ��16x16�����ֻ�8x16��ASCII��.������ͬ*/
		OLED_Display_GB2312_string(0,4,"��8X16����ASCII,");
		OLED_Display_GB2312_string(0,6,"��5X7����ASCII��");
		HAL_Delay(500);				
		OLED_Clear();
		OLED_Display_GB2312_string(24,0,"�о�԰����");	
		OLED_Display_GB2312_string(0,2,"��Ҫ����OLEDģ��");
		OLED_Display_GB2312_string(0,4,"�˿�������Ϸ���");
		OLED_Display_GB2312_string(0,6,"������������һ��");
		HAL_Delay(500);	
		OLED_Clear();		
		OLED_Display_GB2312_string(0,0,"GB2312�����ֿ⼰");	
		OLED_Display_GB2312_string(0,2,"��ͼ�͹��ܣ�����");	
		OLED_Display_GB2312_string(0,4,"����ֻ�ͼ�����");
		OLED_Display_GB2312_string(0,6,"Ƨ�֣����磺");
		OLED_Display_16x16(97,6,jiong1);					/*�ڵ�7ҳ����81����ʾ�����Ա���Ƨ���֡��塱*/
    OLED_Display_16x16(113,6,lei1);
		HAL_Delay(500);
		OLED_Clear();
		OLED_Display_GB2312_string(0,0,"<!@#$%^&*()_-+]/");	/*�ڵ�1ҳ����1�У���ʾһ��16x16�����ֻ�8*16��ASCII��*/
		OLED_Display_string_5x7(0,2,"<!@#$%^&*()_-+]/;.,?[");/*�ڵ�3ҳ����1�У���ʾһ��5x7�����ASCII��*/
		OLED_Display_string_5x7(0,3,"XY electronics Co., ");/*��ʾһ��5x7�����ASCII��*/
		OLED_Display_string_5x7(0,4,"Ltd. established at  ");/*��ʾһ��5x7�����ASCII��*/	
		OLED_Display_string_5x7(0,5,"year 2010.Focus OLED ");/*��ʾһ��5x7�����ASCII��*/
		OLED_Display_string_5x7(0,6,"Mobile:13265585975");/*��ʾһ��5x7�����ASCII��*/
		OLED_Display_string_5x7(0,7,"Tel:0755-32910715");/*��ʾһ��5x7�����ASCII��*/
		HAL_Delay(500);
		OLED_Clear();	
		OLED_Display_GB2312_string(0,0,"����������������");	/*�ڵ�1ҳ����1�У���ʾһ��16x16�����ֻ�8x16��ASCII��*/
		OLED_Display_GB2312_string(0,2,"����������������");  /*��ʾһ��16x16�����ֻ�8x16��ASCII��.������ͬ*/
		OLED_Display_GB2312_string(0,4,"����������������");
		OLED_Display_GB2312_string(0,6,"����������������");		        
		HAL_Delay(500);
		OLED_Clear();	
		OLED_Display_GB2312_string(0,0,"����������������");
		OLED_Display_GB2312_string(0,2,"����������������");
		OLED_Display_GB2312_string(0,4,"����������������");
		OLED_Display_GB2312_string(0,6,"����������������");
		HAL_Delay(500);
		OLED_Clear();
	}
}

void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
   
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
