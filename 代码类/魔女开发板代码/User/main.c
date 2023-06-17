/**==================================================================================================================
 **【文件名称】  main.c
 **【功能测试】  显示屏_1.8寸STM7735
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
 **【更新记录】
 **
 **【备注说明】  版权归魔女科技所有，请勿商用，谢谢！
 **              https://demoboard.taobao.com
====================================================================================================================*/
#include <stm32f10x.h>            // 头文件引用(标准库); 内核、芯片外设....;(stm32f10x.conf.h, 对标准库头文件进行调用)     
#include "stm32f10x_conf.h"       // 头文件引用(标准库); 内核、芯片外设....;(stm32f10x.conf.h, 对标准库头文件进行调用) 
#include "system_f103.h"
#include "stdio.h"                // C标准库文件，用于调用printf, sprintf等常用函数
#include "bsp_led.h"              // LED指示灯
#include "bsp_key.h"              // 按键
#include "bsp_usart.h"            // USART1、2、3，UART4、5
#include "bsp_w25qxx.h"           // 外部Flash芯片驱动文件， 注意，不要做全芯片擦除操作，地址0x00A00000前存储空间，可自由用作数据存储，地址后空间保存有字库数据，
#include "bsp_lcd_st7735.h"



extern const unsigned char imageLoge[];                       // 声明外部变量：图片缓存数组，位于外部文件font.h中



static void delay_ms(uint32_t ms)                             // 定义一个ms延时函数，减少移植时对外部文件依赖;
{
    ms = ms * 10286;                                          // 注意：打勾 "Options --> C/C++ ---> One ELF Section per Function选项"
    for (uint32_t i = 0; i < ms; i++);                        // 72MHz系统时钟下，大约多少个空循环耗时1ms
}



// 主函数
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);           // 中断分组，组2:抢占级0~3,子优先级0~3 ; 全局只设置一次，尽量放在显眼的地方
    System_SwdMode();                                         // 设置芯片调试方式(SWD); 关闭JTAG只保留SWD; 目的:释放PB3、PB4、PA15，只需PA13、PA14

    USART1_Init(115200);                                      // 串口1初始化; 用于与串口软件通信，方便代码调试; USART1(115200-N-8-1), 且工程已把printf重定向至USART1输出

    Led_Init();                                               // LED 初始化
    LED_BLUE_ON ;                                             // 点亮蓝灯

    W25qx_Init();                                             // 设备W25Q128初始化：引脚配置，SPI配置，测试连接, 测试是否存在字库

    LCD_Init();                                               // 显示屏初始化: 引脚配置，SPI配置，LCD参数配置

    LCD_Fill(0, 0, 127, 160, BLACK);                          // 屏幕填充
    LCD_String(18, 10, "综合测试",      24, YELLOW, BLACK);   // 使用板载的FLASH芯片字库显示中文，优点，可显示任意常用汉字; 注意，需要调用W25qx_Init();
    LCD_String(8, 40, "STM32-F103C8T6", 16, WHITE, BLACK);    // 显示英文字符时，字库会从font.h中提取字模

    // 显示特定中文                                           // 使用font.h中预先取模的字库，以数组形式存放，优点：无须依赖外部Flash字库; 缺点：受限芯片内部Flash大小，只能存放小量字库
    LCD_Chinese16ForFile(35, 65, 2, BLACK, WHITE);            // "显", 字模数据存放于font.h的数组中
    LCD_Chinese16ForFile(51, 65, 3, WHITE, RED);              // "示"
    LCD_Chinese16ForFile(67, 65, 4, BLACK, GREEN);            // "方"
    LCD_Chinese16ForFile(83, 65, 5, WHITE, BLUE);             // "案"

    LCD_Image(40, 100, 60, 60, imageLoge);                    // 显示图片; 图片预先取模，以数组数据形式，存放于font.h文件中

    LCD_Fill(1,   1,   8,   8, WHITE);                        // 屏幕填充，
    LCD_Fill(121,   1, 129,   8, RED);                        // 通过放大镜观察左上角、右上角两个色块，发现物理像素的X坐标，左边被遮挡了第1列，右边多了1列129
    LCD_Fill(1, 152,   8, 160, GREEN);                        // 用于判断XY坐标与物理像素位置是否相符
    LCD_Fill(121, 152, 129, 160, BLUE);                       //


    while (1)                                                 // while函数死循环，不能让main函数运行结束，否则会产生硬件错误
    {
        delay_ms(500);                                        // 间隔延时
        LED_BLUE_TOGGLE ;                                     // 每隔一段时间，就闪烁一次LED，以方便观察代码是否正常运行中
    }

}


// 注意：每一个代码文件，末尾保留一空行，否则编译会产生警告
