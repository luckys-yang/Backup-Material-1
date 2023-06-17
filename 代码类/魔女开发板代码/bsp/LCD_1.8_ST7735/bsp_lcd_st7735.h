#ifndef __BSP_LCD_H
#define __BSP_LCD_H
/**==================================================================================================================
 **【文件名称】  bsp_lcd_St7735.h
 **【功能测试】  显示屏_1.8寸STM7735_驱动文件
 **==================================================================================================================
 **【实验平台】  STM32F103 + KEIL5.27 + 1.8寸ST7735显示屏
 **
 **【实验操作】  1_接线
 **                      GND   ---  GND
 **                      VCC   ---  3.3V; 注意, 切勿接5V, 可能产生莫名错误
 **                      SCK   ---  PA5 ; SPI时钟
 **                      MOSI  ---  PA7 ; SPI数据输出
 **                      RST   ---  RST ; 低电平复位线，接芯片的RESET, 跟随芯片一起复位即可
 **                      DC/RS ---  PB4 ; 命令、数据切换
 **                      CS    ---  PB3 ; SPI片选
 **                      BL    ---  PA15; 控制背光LED，高电平亮。(魔女科技1.8寸屏内部BL带下拉电阻，优化为默认熄屏状态，减少误动作亮屏)
 **              2_工程编译后，烧录代码到开发板
 **              3_打开串口上位机即可观察数据输出，及显示屏输出
 **
 **【划 重 点】  1-本示例代码，适配市面大部分1.8寸ST7735屏
 **              2_本示例代码，使用硬件SPI，与市面1.8屏常用模拟SPI代码有区别，移植使用时注意和主程序的配合。
 **              3_本示例代码，中文输出使用了两种字模存储方式，注意区别;
 **
 **
 **【更新记录】  2022-10-18   完善注释 
 **              2021-11-27   修改RST引脚宏定义，以方便适配接GPIO或Reset引脚
 **              2021-04-23   增加FLASH字库判断条件宏定义
 **              2020-08-17   重建代码
 **
 **
 **【备注说明】  版权归魔女科技所有，请勿商用，谢谢！
 **              https://demoboard.taobao.com
====================================================================================================================*/
#include <stm32f10x.h>
#include <stm32f10x_spi.h>
#include "bsp_w25qxx.h"




/*****************************************************************************
 ** 移植修改，只需修改下面的引脚、SPI端口、屏幕参数
*****************************************************************************/
// 模块引脚
// 3-SCL
#define LCD_SCK_GPIO        GPIOA                // 时钟，SCL/SCK
#define LCD_SCK_PIN         GPIO_Pin_5
// 4-SDA
#define LCD_MOSI_GPIO       GPIOA                // 输出，SDA/DIN
#define LCD_MOSI_PIN        GPIO_Pin_7
// 5-RES
#if 0                                            // 用于适配显示屏的RST引脚接GPIO引脚，或Reset引脚；若接GPIO就使用这部分，若接板子的Reset就注释这两行；但不要删除，方便移植
#define LCD_RST_GPIO        GPIOB                // 复位, RST,
#define LCD_RST_PIN         GPIO_Pin_12
#endif
#define LCD_RST_PIN         GPIO_Pin_12
// 6-DC
#define LCD_DC_GPIO         GPIOB                // 命令, RS/DC    【注意，使用PB3, PB4引脚，要修改调试模式为SW模式】
#define LCD_DC_PIN          GPIO_Pin_4
// 7-CS
#define LCD_CS_GPIO         GPIOB                // 片选, CS/CE    【注意，使用PB3, PB4引脚，要修改调试模式为SW模式】
#define LCD_CS_PIN          GPIO_Pin_3
// 8-BL
#define LCD_BL_GPIO         GPIOA                // 背光，BL/LED   【注意，使用PB3, PB4引脚，要修改调试模式为SW模式】
#define LCD_BL_PIN          GPIO_Pin_15
// SPI
#define LCD_SPI_PORT        SPI1
#define LCD_SPI_SCK         RCC_APB2Periph_SPI1
// 屏幕像素
#define LCD_WIDTH           129                  // 设置屏幕的像素大小，
#define LCD_HIGH            160                  // 注意：实际像素好像要比标示值大1~2像素，
// 显示方向
#define LCD_DIR             1                    // 显示的方向，可选参数：1、2、3、4
//　中文字库
#define GBK_STORAGE_HAVE    1                    // 魔女开发板上的FLASH中文字库，可选参数：0-无，1-有; 若非魔女开发板，请用参数0;
//  结束 移植修改 ************************************************************





/*****************************************************************************
 ** 颜色定义， 移植时不用修改
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
#define BROWN              0XBC40    // 棕色
#define BRRED              0XFC07    // 棕红色
#define GRAY               0X8430    // 灰色
#define DARKBLUE           0X01CF    // 深蓝色
#define LIGHTBLUE          0X7D7C    // 浅蓝色  
#define GRAYBLUE           0X5458    // 灰蓝色 
#define LIGHTGREEN         0X841F    // 浅绿色
#define LIGHTGRAY          0XEF5B    // 浅灰色(PANNEL)
#define LGRAY              0XC618    // 浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE          0XA651    // 浅灰蓝色(中间层颜色)
#define LBBLUE             0X2B12    // 浅棕蓝色(选择条目的反色)



/*****************************************************************************
 ** 声明  全局函数； 移植时不用修改
****************************************************************************/
void LCD_Init(void);                                                                                     // LCD驱动ST7735初始化，屏幕参数在本文件顶部修改(移植参数区)，主要是像素尺寸、显示方向
void LCD_Display(uint8_t  sw);                                                                           // 显示屏开关， 1：打开屏幕， 0：熄屏
void LCD_Fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);                       // 填充单色
void LCD_Line(uint16_t sX, uint16_t sY, uint16_t eX, uint16_t eY, uint16_t color);                       // 画线
void LCD_Circle(uint16_t x0, uint16_t y0, uint8_t r,   uint16_t color);                                  // 画圆
void LCD_Image(uint16_t x,  uint16_t y,  uint16_t width, uint16_t height, const u8 *image);              // 显示图片
void LCD_String(uint16_t x,  uint16_t y,  char *pFont, uint8_t size, uint16_t fColor, uint16_t bColor);  // 显示任意中英文字符串；左上角坐标x, 左上角坐标y, 字库缓存地址，字号大小，字色，底色
void LCD_Chinese16ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor);      // 显示1个16号汉字；左上角坐标x, 左上角坐标y, 字库数据在缓存中的位置，字色，底色
void LCD_Chinese32ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor);      // 显示1个32号汉字；左上角坐标x, 左上角坐标y, 字库数据在缓存中的位置，字色，底色
void LCD_GUI(void);                                                                                      // 简单界面绘制示例
#endif





