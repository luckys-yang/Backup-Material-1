#ifndef __BSP_LCD_H
#define __BSP_LCD_H
/**==================================================================================================================
 **���ļ����ơ�  bsp_lcd_St7735.h
 **�����ܲ��ԡ�  ��ʾ��_1.8��STM7735_�����ļ�
 **==================================================================================================================
 **��ʵ��ƽ̨��  STM32F103 + KEIL5.27 + 1.8��ST7735��ʾ��
 **
 **��ʵ�������  1_����
 **                      GND   ---  GND
 **                      VCC   ---  3.3V; ע��, �����5V, ���ܲ���Ī������
 **                      SCK   ---  PA5 ; SPIʱ��
 **                      MOSI  ---  PA7 ; SPI�������
 **                      RST   ---  RST ; �͵�ƽ��λ�ߣ���оƬ��RESET, ����оƬһ��λ����
 **                      DC/RS ---  PB4 ; ��������л�
 **                      CS    ---  PB3 ; SPIƬѡ
 **                      BL    ---  PA15; ���Ʊ���LED���ߵ�ƽ����(ħŮ�Ƽ�1.8�����ڲ�BL���������裬�Ż�ΪĬ��Ϩ��״̬��������������)
 **              2_���̱������¼���뵽������
 **              3_�򿪴�����λ�����ɹ۲��������������ʾ�����
 **
 **���� �� �㡿  1-��ʾ�����룬��������󲿷�1.8��ST7735��
 **              2_��ʾ�����룬ʹ��Ӳ��SPI��������1.8������ģ��SPI������������ֲʹ��ʱע������������ϡ�
 **              3_��ʾ�����룬�������ʹ����������ģ�洢��ʽ��ע������;
 **
 **
 **�����¼�¼��  2022-10-18   ����ע�� 
 **              2021-11-27   �޸�RST���ź궨�壬�Է��������GPIO��Reset����
 **              2021-04-23   ����FLASH�ֿ��ж������궨��
 **              2020-08-17   �ؽ�����
 **
 **
 **����ע˵����  ��Ȩ��ħŮ�Ƽ����У��������ã�лл��
 **              https://demoboard.taobao.com
====================================================================================================================*/
#include <stm32f10x.h>
#include <stm32f10x_spi.h>
#include "bsp_w25qxx.h"




/*****************************************************************************
 ** ��ֲ�޸ģ�ֻ���޸���������š�SPI�˿ڡ���Ļ����
*****************************************************************************/
// ģ������
// 3-SCL
#define LCD_SCK_GPIO        GPIOA                // ʱ�ӣ�SCL/SCK
#define LCD_SCK_PIN         GPIO_Pin_5
// 4-SDA
#define LCD_MOSI_GPIO       GPIOA                // �����SDA/DIN
#define LCD_MOSI_PIN        GPIO_Pin_7
// 5-RES
#if 0                                            // ����������ʾ����RST���Ž�GPIO���ţ���Reset���ţ�����GPIO��ʹ���ⲿ�֣����Ӱ��ӵ�Reset��ע�������У�����Ҫɾ����������ֲ
#define LCD_RST_GPIO        GPIOB                // ��λ, RST,
#define LCD_RST_PIN         GPIO_Pin_12
#endif
#define LCD_RST_PIN         GPIO_Pin_12
// 6-DC
#define LCD_DC_GPIO         GPIOB                // ����, RS/DC    ��ע�⣬ʹ��PB3, PB4���ţ�Ҫ�޸ĵ���ģʽΪSWģʽ��
#define LCD_DC_PIN          GPIO_Pin_4
// 7-CS
#define LCD_CS_GPIO         GPIOB                // Ƭѡ, CS/CE    ��ע�⣬ʹ��PB3, PB4���ţ�Ҫ�޸ĵ���ģʽΪSWģʽ��
#define LCD_CS_PIN          GPIO_Pin_3
// 8-BL
#define LCD_BL_GPIO         GPIOA                // ���⣬BL/LED   ��ע�⣬ʹ��PB3, PB4���ţ�Ҫ�޸ĵ���ģʽΪSWģʽ��
#define LCD_BL_PIN          GPIO_Pin_15
// SPI
#define LCD_SPI_PORT        SPI1
#define LCD_SPI_SCK         RCC_APB2Periph_SPI1
// ��Ļ����
#define LCD_WIDTH           129                  // ������Ļ�����ش�С��
#define LCD_HIGH            160                  // ע�⣺ʵ�����غ���Ҫ�ȱ�ʾֵ��1~2���أ�
// ��ʾ����
#define LCD_DIR             1                    // ��ʾ�ķ��򣬿�ѡ������1��2��3��4
//�������ֿ�
#define GBK_STORAGE_HAVE    1                    // ħŮ�������ϵ�FLASH�����ֿ⣬��ѡ������0-�ޣ�1-��; ����ħŮ�����壬���ò���0;
//  ���� ��ֲ�޸� ************************************************************





/*****************************************************************************
 ** ��ɫ���壬 ��ֲʱ�����޸�
****************************************************************************/
#define WHITE              0xFFFF
#define BLACK              0x0000
#define BLUE               0x001F
#define BRED               0XF81F
#define GRED               0XFFE0
#define GBLUE              0X07FF
#define RED                0xF800
#define MAGENTA            0xF81F
#define GREEN              0x07E0
#define CYAN               0x7FFF
#define YELLOW             0xFFE0
#define BROWN              0XBC40    // ��ɫ
#define BRRED              0XFC07    // �غ�ɫ
#define GRAY               0X8430    // ��ɫ
#define DARKBLUE           0X01CF    // ����ɫ
#define LIGHTBLUE          0X7D7C    // ǳ��ɫ  
#define GRAYBLUE           0X5458    // ����ɫ 
#define LIGHTGREEN         0X841F    // ǳ��ɫ
#define LIGHTGRAY          0XEF5B    // ǳ��ɫ(PANNEL)
#define LGRAY              0XC618    // ǳ��ɫ(PANNEL),���屳��ɫ
#define LGRAYBLUE          0XA651    // ǳ����ɫ(�м����ɫ)
#define LBBLUE             0X2B12    // ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)



/*****************************************************************************
 ** ����  ȫ�ֺ����� ��ֲʱ�����޸�
****************************************************************************/
void LCD_Init(void);                                                                                     // LCD����ST7735��ʼ������Ļ�����ڱ��ļ������޸�(��ֲ������)����Ҫ�����سߴ硢��ʾ����
void LCD_Display(uint8_t  sw);                                                                           // ��ʾ�����أ� 1������Ļ�� 0��Ϩ��
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);                       // ��䵥ɫ
void LCD_Line(uint16_t sX, uint16_t sY, uint16_t eX, uint16_t eY, uint16_t color);                       // ����
void LCD_Circle(uint16_t x0, uint16_t y0, uint8_t r,   uint16_t color);                                  // ��Բ
void LCD_Image(uint16_t x,  uint16_t y,  uint16_t width, uint16_t height, const u8 *image);              // ��ʾͼƬ
void LCD_String(uint16_t x,  uint16_t y,  char *pFont, uint8_t size, uint16_t fColor, uint16_t bColor);  // ��ʾ������Ӣ���ַ��������Ͻ�����x, ���Ͻ�����y, �ֿ⻺���ַ���ֺŴ�С����ɫ����ɫ
void LCD_Chinese16ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor);      // ��ʾ1��16�ź��֣����Ͻ�����x, ���Ͻ�����y, �ֿ������ڻ����е�λ�ã���ɫ����ɫ
void LCD_Chinese32ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor);      // ��ʾ1��32�ź��֣����Ͻ�����x, ���Ͻ�����y, �ֿ������ڻ����е�λ�ã���ɫ����ɫ
void LCD_GUI(void);                                                                                      // �򵥽������ʾ��
#endif





