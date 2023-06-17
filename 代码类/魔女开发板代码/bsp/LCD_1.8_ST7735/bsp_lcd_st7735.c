/***********************************************************************************************************************************
 ** 【文件名称】  lcd_st7735.c
 ** 【编写人员】  魔女开发板团队
 ** 【更新分享】  Q群文件夹       1126717453
 ** 【淘    宝】  魔女开发板      https://demoboard.taobao.com
 ***********************************************************************************************************************************
 ** 【文件功能】  初始化GPIO、SPI, 定义各功能函数
 **
 ** 【适用平台】  STM32F103 + 标准库v3.5 + keil5
 **
 ** 【本地函数】
 **
 ** 【更新记录】
 **               2021-11-27   修改RST控制部分，以适配显示屏的RST引脚接GPIO引脚、Reset引脚
 **               2021-11-27   增加LCD_GUI()，方便测试
 **               2021-05-01   字库数据读取函数getBGK(), 改由外部函数W25qxx_ReadGBK()实现, 以增加移植的方便;
 **               2021-04-23   增加FLASH字库判断条件宏定义
 **               2021-04-23   增加本地延时函数，减少移植时对外部文件依赖；
 **               2020-12-18   完善代码结构、完善注释格式
 **
************************************************************************************************************************************/
#include "bsp_lcd_st7735.h"
#include "font.h"




/*****************************************************************************
 ** 本地变量
 *****************************************************************************/
struct _LCD              // 结构体：用于保存信息、状态
{
    u8  InitOK;
    u16 width;           // LCD 宽度
    u16 height;          // LCD 高度

    u32 bColor;          // 背景色
} xLCD;



/*****************************************************************************
 ** 本地函数
 ****************************************************************************/
static void gpioInit(void);                                         // 引脚初始化
static void spiInit(void);                                          // SPI通信协议初始化
static void sendOrder(u16 order);                                   // 向液晶屏发送：寄存器地址值(指令), 8位
static void sendByte(u8  Data);                                     // 向液晶屏发送：数据，8位
static void sendShort(u16 Data);                                    // 向液晶屏发送：数据，16位
static void setCursor(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd);  // 设置显示区域
static void delay_ms(u32);                                           // 简单的延时函数，减少移植时对外部文件依赖；




/*****************************************************************************
 ** 显示屏控制宏定义, 主要为了简化文件内操作
 *****************************************************************************/
// LCD_CS
#define    LCD_CS_LOW      (LCD_CS_GPIO->BSRR=((uint32_t)LCD_CS_PIN) <<16)
#define    LCD_CS_HIGH     (LCD_CS_GPIO->BSRR=((uint32_t)LCD_CS_PIN))
//
#define    LCD_BL_LOW      (LCD_BL_GPIO->BSRR=((uint32_t)LCD_BL_PIN) <<16)
#define    LCD_BL_HIGH     (LCD_BL_GPIO->BSRR=((uint32_t)LCD_BL_PIN))

#define    LCD_RS_LOW      (LCD_DC_GPIO->BSRR=((uint32_t)LCD_DC_PIN) <<16)
#define    LCD_RS_HIGH     (LCD_DC_GPIO->BSRR=((uint32_t)LCD_DC_PIN))
//LCD_RST
#ifdef LCD_RST_GPIO
    //#define    LCD_RST_LOW      LCD_RST_GPIO->BSRR=LCD_RST_PIN <<16
    //#define    LCD_RST_HIGH      LCD_RST_GPIO->BSRR=LCD_RST_PIN
#endif


// 本地US粗略延时函数，减少移植时对外部文件依赖；
//static void delayUS( u32 times)
//{
//    times=times*7;      //  10us内用7;
//    while(--times)
//        __nop();
//}

// 本地MS粗略延时函数，减少移植时对外部文件依赖；
static void delay_ms(u32 ms)
{
    ms = ms * 6500;
    for (u32 i = 0; i < ms; i++); // 72MHz系统时钟下，多少个空循环约耗时1ms
}




// 引脚初始化
static void gpioInit(void)
{
    // 开启SWD调试模式，以释放PB3, PB4, PA15
    RCC->APB2ENR |= 1<<0;           // 开启辅助时钟       
    AFIO->MAPR   &= 0XF8FFFFFF;     // 清0MAPR的[26:24]
    AFIO->MAPR   |= 0x2<<24;        // 设置模式  000:全开   010：只开SWD   100:全关 
    
    // 使用标准库
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE); // PORT时钟使能
    // SCK
    GPIO_InitStructure.GPIO_Pin   = LCD_SCK_PIN  ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;           // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_SCK_GPIO, &GPIO_InitStructure);
    // MOSI
    GPIO_InitStructure.GPIO_Pin   = LCD_MOSI_PIN ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(LCD_MOSI_GPIO, &GPIO_InitStructure);
    // RES
#ifdef LCD_RST_GPIO
    GPIO_InitStructure.GPIO_Pin   = LCD_RST_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_RST_GPIO, &GPIO_InitStructure);
#endif
    // DC
    GPIO_InitStructure.GPIO_Pin   = LCD_DC_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_DC_GPIO, &GPIO_InitStructure);
    // CS
    GPIO_InitStructure.GPIO_Pin   = LCD_CS_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_CS_GPIO, &GPIO_InitStructure);
    // BL
    GPIO_InitStructure.GPIO_Pin   = LCD_BL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_BL_GPIO, &GPIO_InitStructure);

    LCD_CS_HIGH ;   // 先上拉，防止电平浮动
    LCD_BL_HIGH ;   // 打开屏幕显示
}

// SPI通信协议初始化
// 重要：因为SPI总线可能挂载多个要求不同通信参数的设备，最好在每次通信前，调用一次配置函数
static void spiInit(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB2PeriphClockCmd(LCD_SPI_SCK,  ENABLE);                       // SPI1时钟使能

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工  SPI_Direction_1Line_Tx
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       // 设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                   // 设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                         // 选择了串行时钟的稳态:时钟悬空高
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                        // 数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                           // NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  //定义波特率预分频的值:波特率预分频值为2
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  // 指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;                            // CRC值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure);                                 // 根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(LCD_SPI_PORT, ENABLE);                                      // 使能SPI外设
    //SPI2_ReadWriteByte(0xff);//启动传输
}

// SPI发送传送1字节
static u8 spiSendByte(u8 data)
{
    static u16 retry;

    retry = 0;
    while ((LCD_SPI_PORT -> SR & 2) == 0)
    {
        // 等待发送区为空
        retry++;
        if (retry > 500) return 0;
    }
    LCD_SPI_PORT ->DR = data;

    retry = 0;
    while ((LCD_SPI_PORT -> SR & 1) == 0)
    {
        // 等待接收完数据
        retry++;
        if (retry > 250) return 0;
    }
    return LCD_SPI_PORT -> DR ;
}



/*****************************************************************************
*函  数：LCD_Init
*功  能：初始化lcd
*参  数：为了简化理解、便于移植，参数(屏幕像素、显示方向)在h文件里进行修改
*返回值：无
*备  注：
*****************************************************************************/
void LCD_Init(void)
{
    xLCD.InitOK = 0;
    xLCD.bColor  = BLACK;

    // 显示方向像素匹配
    if ((LCD_DIR == 1) || (LCD_DIR == 3))
    {
        xLCD.width  = LCD_WIDTH ;       // 屏宽度像素，超过此值驱动芯片会自动换行，注意：如果屏幕右边有花屏，就加大这个值
        xLCD.height = LCD_HIGH ;        // 屏高度像素， 注意：如果屏幕下面有花屏，就加大这个值
    }
    else
    {
        xLCD.width  = LCD_HIGH;
        xLCD.height = LCD_WIDTH;
    }

    // 初始化GPIO
    gpioInit();
    // 初始化SPI外设、协议
    spiInit();

    // 屏幕复位
#ifdef LCD_RST_GPIO
    LCD_RST_LOW ;          // LCD_RST=0      //SPI接口复位
    delay_ms(10);           // delay 20 ms
    LCD_RST_HIGH  ;        // LCD_RST=1
#endif
    delay_ms(200);          // delay 20 ms

    sendOrder(0x11);       // 退出睡眠模式
    delay_ms(120);          // Delay 120ms
    //------------------------------------ST7735S Frame Rate-----------------------------------------//
    sendOrder(0xB1);
    sendByte(0x05);
    sendByte(0x3C);
    sendByte(0x3C);

    sendOrder(0xB2);
    sendByte(0x05);
    sendByte(0x3C);
    sendByte(0x3C);

    sendOrder(0xB3);
    sendByte(0x05);
    sendByte(0x3C);
    sendByte(0x3C);
    sendByte(0x05);
    sendByte(0x3C);
    sendByte(0x3C);
    //------------------------------------End ST7735S Frame Rate-----------------------------------------//
    sendOrder(0xB4); // Dot inversion
    sendByte(0x03);

    sendOrder(0xC0); // ST7735R Power Sequence
    sendByte(0x28);
    sendByte(0x08);
    sendByte(0x04);

    sendOrder(0xC1);
    sendByte(0XC0);

    sendOrder(0xC2);
    sendByte(0x0D);
    sendByte(0x00);
    sendOrder(0xC3);

    sendByte(0x8D);
    sendByte(0x2A);

    sendOrder(0xC4);
    sendByte(0x8D);
    sendByte(0xEE);
    //---------------------------------End ST7735S Power Sequence-------------------------------------//
    sendOrder(0xC5); //VCOM
    sendByte(0x1A);
    sendOrder(0x36); //MX, MY, RGB mode
    if (LCD_DIR == 1)  sendByte(0xC0); // 重要：显示方向控制，C0/00/A0/60,  C8/08/A8/68
    if (LCD_DIR == 2)  sendByte(0x00);
    if (LCD_DIR == 3)  sendByte(0xA0);
    if (LCD_DIR == 4)  sendByte(0x60);
    //------------------------------------ST7735S Gamma Sequence-----------------------------------------//
    sendOrder(0xE0);
    sendByte(0x04);
    sendByte(0x22);
    sendByte(0x07);
    sendByte(0x0A);
    sendByte(0x2E);
    sendByte(0x30);
    sendByte(0x25);
    sendByte(0x2A);
    sendByte(0x28);
    sendByte(0x26);
    sendByte(0x2E);
    sendByte(0x3A);
    sendByte(0x00);
    sendByte(0x01);
    sendByte(0x03);
    sendByte(0x13);

    sendOrder(0xE1);
    sendByte(0x04);
    sendByte(0x16);
    sendByte(0x06);
    sendByte(0x0D);
    sendByte(0x2D);
    sendByte(0x26);
    sendByte(0x23);
    sendByte(0x27);
    sendByte(0x27);
    sendByte(0x25);
    sendByte(0x2D);
    sendByte(0x3B);
    sendByte(0x00);
    sendByte(0x01);
    sendByte(0x04);
    sendByte(0x13);

    // 后期复制增加的，不明白
    sendOrder(0x2a);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x7f);
    // 后期复制增加的，不明白
    sendOrder(0x2b);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x9f);
//------------------------------------End ST7735S Gamma Sequence-----------------------------------------//
    sendOrder(0x3A); // 65k mode
    sendByte(0x05);

    sendOrder(0x29); // Display on

    LCD_Fill(1, 1, xLCD.width, xLCD.height, BLACK);
    xLCD.InitOK = 1;
}
// 向LCD发送寄存器地址(指令)
static void sendOrder(u16 order)
{
    LCD_CS_LOW;            // SPI设备片选拉低，开始通信
    LCD_RS_LOW;            // RS高: 数据值， RS低: 寄存器地址值
    spiSendByte(order);    // 发送 寄存器地址值
    LCD_CS_HIGH;           // SPI设备片选拉高，结束通信
}
// 向LCD发送数值
static void sendByte(u8 data)
{
    LCD_CS_LOW;            // SPI设备片选拉低，开始通信
    LCD_RS_HIGH;           // RS高: 数据值， RS低: 寄存器地址值
    spiSendByte(data);     // 发送1字节
    LCD_CS_HIGH;           // SPI设备片选拉高，结束通信
}
// 向LCD发送2个字节
static void sendShort(u16 data)
{
    LCD_CS_LOW;            // SPI设备片选拉低，开始通信
    LCD_RS_HIGH;           // RS高: 数据， RS低: 指令
    spiSendByte(data >> 8); // 发送1字节
    spiSendByte(data);     // 发送1字节
    LCD_CS_HIGH;           // SPI设备片选拉高，结束通信
}

//写寄存器: 寄存器地址，数据
//static void sendRegisterValue(u8 reg, u16 val)
//{
//    sendOrder(reg);
//    sendShort(val);
//}

/*****************************************************************
 * 函  数：LCD_Display
 * 功  能：显示屏开关
 * 参  数：0:熄，非0：开
 * 返回值：无
 *
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
******************************************************************/
void LCD_Display(uint8_t sw)
{
    if (sw == 0)
        LCD_BL_LOW ;
    else
        LCD_BL_HIGH ;
}

/*****************************************************************
 * 函  数：setCursor
 * 功  能：设置显示区域，在此区域写点数据自动换行
 * 参  数：xStart横坐标起点, yStart纵坐标起点
 *         xEnd横坐标线束点，yEnd纵坐标线束点
 * 返回值：无
 *
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
******************************************************************/
static void setCursor(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd)
{
    sendOrder(0x2A);
    sendByte(0x00);    // 起始位置x高位，因为1.8寸屏是128*160, 不大于255, 直接写0省事
    sendByte(xStart);  // 起始位置x低位，值传递时自动舍弃了高8位，也省得运算了
    sendByte(0x00);    // 起始位置y高位
    sendByte(xEnd);    // 起始位置x位位

    sendOrder(0x2B);
    sendByte(0x00);
    sendByte(yStart);
    sendByte(0x00);
    sendByte(yEnd);

    sendOrder(0x2c);  // 发送写数据命令
}

/*****************************************************************
 * 函  数：drawPoint
 * 功  能：画一个点
 * 参  数：x坐标，y坐标, 16位颜色值
 * 返回值：无
 *
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：1126717453  最后修改_2020年09月01日
******************************************************************/
static void drawPoint(u16 x, u16 y, u16 color)
{
    setCursor(x, y, x, y);      //设置光标位置
    sendShort(color);
}

/******************************************************************
 * 函数名： LCD_Fill
 * 功  能： 在指定区域内填充单个颜色
 *          屏幕坐标从1开始；屏幕左右和下方实际上有多余行列
 * 参  数： u16 xStart, yStart  左上角起始坐标
 *          u16 xEnd,   yEnd    右下角起始坐标
 *          u32 Color           填充的颜色
 *
 * 备  注： 魔女开发板团队编写   淘宝 https://demoboard.taobao.com
 * 分  享： 不断更新于Q群文件夹：1126717453  最后修改_2020年09月01日
 *****************************************************************/
void LCD_Fill(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd, u16 color)
{
    spiInit();                                      // 防止SPI参数被其它设备修改了
    u32 pixel = (xEnd - xStart + 1) * (yEnd - yStart + 1); // 填充的像素数量

    setCursor(xStart, yStart, xEnd, yEnd);          // 设定填充范围
    while (pixel-- > 0)                             // 发送颜色值
        sendShort(color);
}

/******************************************************************
 * 函数名： LCD_Line
 * 功  能： 画线
 * 参  数： u16 x1,y1  起点坐标
 *          u16 x2,y2  终点坐标
 *          u32 Color  颜色
 *****************************************************************/
void LCD_Line(u16 sX, u16 sY, u16 eX, u16 eY, u16 color)
{
    spiInit();                                      // 防止SPI参数被其它设备修改了

    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = eX - sX; //计算坐标增量
    delta_y = eY - sY;
    uRow = sX;
    uCol = sY;
    if (delta_x > 0)incx = 1; //设置单步方向
    else if (delta_x == 0)incx = 0; //垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0; //水平线
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)distance = delta_x; //选取基本增量坐标轴
    else distance = delta_y;
    for (t = 0; t <= distance + 1; t++) //画线输出
    {
        drawPoint(uRow, uCol, color);//画点
        xerr += delta_x ;
        yerr += delta_y ;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
// 原厂商提供的画圆函数，感觉有点失真，后面再慢慢搞它吧
void LCD_Circle(u16 x0, u16 y0, u8 r, u16 color)
{
    spiInit();                                   // 防止SPI参数被其它设备修改了
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);                           //判断下个点位置的标志
    while (a <= b)
    {
        drawPoint(x0 + a, y0 - b, color);        //5
        drawPoint(x0 + b, y0 - a, color);        //0
        drawPoint(x0 + b, y0 + a, color);        //4
        drawPoint(x0 + a, y0 + b, color);        //6
        drawPoint(x0 - a, y0 + b, color);        //1
        drawPoint(x0 - b, y0 + a, color);
        drawPoint(x0 - a, y0 - b, color);        //2
        drawPoint(x0 - b, y0 - a, color);        //7
        a++;
        //使用Bresenham算法画圆
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/******************************************************************
 * 函数名： LCD_Image
 * 功  能： 在指定区域内填充指定图片数据
 *          图片数据在font.h文件中.只适合少量图片保存
 *          Image2Lcd转换：水平扫描，16位真彩色
 * 参  数： u16 x,y     左上角起始坐标
 *          u16 width   图片宽度
 *          u16 height  图片高度
 *          u8* image   数据缓存地址
 * 备  注： 魔女开发板团队  资料存放Q群：1126717453        最后修改_2020年09月2日
 *****************************************************************/
void LCD_Image(u16 x, u16 y, u16 width, u16 height, const u8 *image)
{
    spiInit();                                 // 防止SPI参数被其它设备修改了

    u16 colorData = 0;
    u32 cnt = 0;

    for (u16 i = 0; i < height; i++)           // 一行一行地显示
    {
        setCursor(x, y + i, x + width, y + height); // 重新设置光标位置
        for (u16 j = 0; j < width; j++)        // 一行中，从左到事，逐个像素处理
        {
            colorData = (image[cnt * 2 + 1] << 8) | image[cnt * 2];
            sendShort(colorData);              // 写入16位颜色数据
            cnt++;
        }
    }
}

/******************************************************************
 * 函数名： LCD_Chinese16ForFile
 * 功  能： 显示1个16号汉字,字库数据在font文件中，只适合少量汉字固定输出
 *          PCtoLCD2002取模：阴码+逐列式+顺向+C51格式
 * 参  数： uint16_t  x         起始坐标x
 *          uint16_t  y         起始坐标y
 *          uint8_t   index     字库数据在缓存中的位置
 *          uint16_t  fColor    字色
 *          uint16_t  bColor    底色
 * 返  回:  无
 *****************************************************************/
void LCD_Chinese16ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor)
{
    spiInit();                                      // 防止SPI参数被其它设备修改了

    const u8 *temp = hanzi16 + index * 32;          // 获取字体数据在数组中的的起始位置

    for (u8 j = 0; j < 16; j++)
    {
        setCursor(x, y + j, x + 16, y + j + 16);
        for (u8 k = 0; k < 2; k++)
        {
            for (u8 i = 0; i < 8; i++)
            {
                if ((*temp & (1 << i)) != 0)
                    sendShort(fColor);
                else
                    sendShort(bColor);
            }
            temp++;
        }
    }
}

/******************************************************************
 * 函数名： LCD_Chinese32ForFile
 * 功  能： 显示1个32号汉字,字库数据在font文件中，只适合少量汉字固定输出
 *          PCtoLCD2002取模：阴码+逐列式+顺向+C51格式
 * 参  数： uint16_t  x         起始坐标x
 *          uint16_t  y         起始坐标y
 *          uint8_t   index     字库数据在缓存中的位置
 *          uint16_t  fColor    字色
 *          uint16_t  bColor    底色
 * 返  回:  无
 *****************************************************************/
void LCD_Chinese32ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor)
{
    spiInit();                                  // 防止SPI参数被其它设备修改了

    const u8 *temp = hanzi32 + index * 128;     // 获取字体数据在数组中的的起始位置

    for (u8 j = 0; j < 32; j++)                 // 一行一行地向下写
    {
        setCursor(x, y + j, x + 32, y + j + 32); // 只要定位了首个坐标，后面发送的数据，就会向后排列，直至收到新的命令
        for (u8 k = 0; k < 4; k++)
        {
            for (u8 i = 0; i < 8; i++)
            {
                if ((*temp & (1 << i)) != 0)
                    sendShort(fColor);
                else
                    sendShort(bColor);
            }
            temp++;
        }
    }
}

/******************************************************************
 * 函数名： drawAscii
 * 功  能： 在指定位置显示一个字符
 * 参  数： u16 x,y     起始坐标
 *          u8  num     要显示的字符:" "--->"~"
 *          u8  size    字体大小 12/16/24/32
 *          u32 fColor  字体颜色
 *          u32 bColor  背景颜色
 * 备  注： 参考原子哥和野火大神的代码后修改  资料存放Q群：1126717453
 *****************************************************************/
void drawAscii(u16 x, u16 y, u8 num, u8 size, u32 fColor, u32 bColor)
{
    spiInit();                                        // 防止SPI参数被其它设备修改了

    if (xLCD.InitOK == 0) return;

    u8 temp;
    u16 y0 = y;

    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // 得到字体一个字符对应点阵集所占的字节数
    num = num - ' ';                                   // 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）
    for (u8 t = 0; t < csize; t++)
    {
        if (size == 12)         temp = asc2_1206[num][t]; // 调用1206字体
        else if (size == 16)    temp = asc2_1608[num][t]; // 调用1608字体
        else if (size == 24)    temp = asc2_2412[num][t]; // 调用2412字体
        else if (size == 32)    temp = asc2_3216[num][t]; // 调用3216字体
        else return;                                   // 没有的字库

        for (u8 t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)   drawPoint(x, y, fColor); // 字体 画点
            else            drawPoint(x, y, bColor);   // 背景 画点
            temp <<= 1;
            y++;
            if (y >= xLCD.height)    return;            // 超出屏幕高度(底)
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                if (x >= xLCD.width) return;           // 超出屏幕宽度(宽)
                break;
            }
        }
    }
}



// 显示汉字
void drawGBK(uint16_t x, uint16_t y, uint8_t *font, uint8_t size, uint16_t fColor, uint16_t bColor)
{
    u8 temp;
    u16 y0 = y;
    u8 GBK[128];
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size); // 得到字体一个字符对应点阵集所占的字节数

#ifdef GBK_STORAGE_ADDR                          // 判断是否使用魔女科技的w25qxx.c文件, 防止编译错误; 可自行修改
    W25qxx_ReadGBK(font, size, GBK);                 // 得到相应大小的点阵数据
#endif

    spiInit();                                       // 防止SPI参数被其它设备修改了
    for (u8 t = 0; t < csize; t++)
    {
        temp = GBK[t];                                 // 得到GBK点阵数据
        for (u8 t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)   drawPoint(x, y, fColor);
            else            drawPoint(x, y, bColor);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}



/******************************************************************************
 * 函  数： LCD_String
 * 功  能： 在屏幕上显示字符串
 *          支持英文、中文
 *          符号、英文字模存放在font.h文件中，汉字字模存放在W25Q128中
 * 参  数： uint16_t  lcd_x   起始坐标X
 *          uint16_t  lcd_y   起始坐标y
 *          char*     pFont
 *          uint8_t   size    字体大小，12、16、24、32, 如果不是这4种规格，会默认使用24号字体
 *          uint32_t  fColor  字体颜色
 * 返回值：
******************************************************************************/
void LCD_String(uint16_t lcd_x, uint16_t lcd_y, char *pFont, uint8_t size, uint16_t fColor, uint16_t bColor)
{
    if (xLCD .InitOK == 0) return;

    u16 xStart = lcd_x;

    if (size != 12 && size != 16 && size != 24 && size != 32) // 字体大小控制
        size = 24;

    while (*pFont != 0)                   // 连续读取字符串数据，直到'\0'时停止
    {
        if (lcd_x > (xLCD.width - size))  // 行位置判断，如果到了行末，就把光标换行
        {
            lcd_x = xStart;
            lcd_y = lcd_y + size;
        }
        if (lcd_y > (xLCD.height - size)) // 列位置判断，如果到了列末，就返回，不再输出
            return;

        if (*pFont < 127)                 // ASCII字符
        {
            drawAscii(lcd_x, lcd_y, *pFont, size, fColor, bColor);
            pFont++;
            lcd_x += size / 2;
        }
        else                              // 汉字显示
        {
#ifdef GBK_STORAGE_ADDR
            drawGBK(lcd_x, lcd_y, (u8 *)pFont, size, fColor, bColor);  // 判断是否使用魔女这样就不影响ASCII英文字符的输出
#endif

            pFont = pFont + 2;            // 下一个要显示的数据在内存中的位置
            lcd_x = lcd_x + size;         // 下一个要显示的数据在屏幕上的X位置
        }
    }
}

/******************************************************************************
 * 函  数： LCD_GUI
 * 功  能： 简单的显示界面
 *          用于示例LCD各函数的使用
 * 参  数：
 * 返回值：
******************************************************************************/
void LCD_GUI(void)
{
    // 显示任意中英文                                       // 优点：完整宋体中文字库，可显示任意汉字; 缺点：需要依赖开发板上已存有字库数据的Flash
    LCD_String(18, 10, "综合测试",      24, YELLOW, BLACK); // 显示字符串
    LCD_String(8, 40, "STM32-F103C8T6", 16, WHITE, BLACK);  // 显示字符串

    // 显示特定中文                                         // 优点：无须依赖外部Flash字库，数据存于：font.h; 缺点：受限芯片内部Flash大小，只能存放小量字库
    LCD_Chinese16ForFile(35, 65, 2, BLACK, WHITE);          // "显", 字模数据存放于font.h的数组中
    LCD_Chinese16ForFile(51, 65, 3, WHITE, RED);            // "示"
    LCD_Chinese16ForFile(67, 65, 4, BLACK, GREEN);          // "方"
    LCD_Chinese16ForFile(83, 65, 5, WHITE, BLUE);           // "案"

    LCD_Image(40, 100, 60, 60, imageLoge);                  // 显示图片

    LCD_Fill(1,   1,   8,   8, WHITE);                      // 屏幕填充，
    LCD_Fill(121,   1, 129,   8, RED);                      // 通过放大镜观察左上角、右上角两个色块，发现物理像素的X坐标，左边被遮挡了第1列，右边多了1列129
    LCD_Fill(1, 152,   8, 160, GREEN);                      // 用于判断XY坐标与物理像素位置是否相符
    LCD_Fill(121, 152, 129, 160, BLUE);                     //

}






