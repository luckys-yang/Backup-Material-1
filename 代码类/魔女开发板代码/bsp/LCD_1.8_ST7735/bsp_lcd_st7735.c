/***********************************************************************************************************************************
 ** ���ļ����ơ�  lcd_st7735.c
 ** ����д��Ա��  ħŮ�������Ŷ�
 ** �����·���  QȺ�ļ���       1126717453
 ** ����    ����  ħŮ������      https://demoboard.taobao.com
 ***********************************************************************************************************************************
 ** ���ļ����ܡ�  ��ʼ��GPIO��SPI, ��������ܺ���
 **
 ** ������ƽ̨��  STM32F103 + ��׼��v3.5 + keil5
 **
 ** �����غ�����
 **
 ** �����¼�¼��
 **               2021-11-27   �޸�RST���Ʋ��֣���������ʾ����RST���Ž�GPIO���š�Reset����
 **               2021-11-27   ����LCD_GUI()���������
 **               2021-05-01   �ֿ����ݶ�ȡ����getBGK(), �����ⲿ����W25qxx_ReadGBK()ʵ��, ��������ֲ�ķ���;
 **               2021-04-23   ����FLASH�ֿ��ж������궨��
 **               2021-04-23   ���ӱ�����ʱ������������ֲʱ���ⲿ�ļ�������
 **               2020-12-18   ���ƴ���ṹ������ע�͸�ʽ
 **
************************************************************************************************************************************/
#include "bsp_lcd_st7735.h"
#include "font.h"




/*****************************************************************************
 ** ���ر���
 *****************************************************************************/
struct _LCD              // �ṹ�壺���ڱ�����Ϣ��״̬
{
    u8  InitOK;
    u16 width;           // LCD ���
    u16 height;          // LCD �߶�

    u32 bColor;          // ����ɫ
} xLCD;



/*****************************************************************************
 ** ���غ���
 ****************************************************************************/
static void gpioInit(void);                                         // ���ų�ʼ��
static void spiInit(void);                                          // SPIͨ��Э���ʼ��
static void sendOrder(u16 order);                                   // ��Һ�������ͣ��Ĵ�����ֵַ(ָ��), 8λ
static void sendByte(u8  Data);                                     // ��Һ�������ͣ����ݣ�8λ
static void sendShort(u16 Data);                                    // ��Һ�������ͣ����ݣ�16λ
static void setCursor(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd);  // ������ʾ����
static void delay_ms(u32);                                           // �򵥵���ʱ������������ֲʱ���ⲿ�ļ�������




/*****************************************************************************
 ** ��ʾ�����ƺ궨��, ��ҪΪ�˼��ļ��ڲ���
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


// ����US������ʱ������������ֲʱ���ⲿ�ļ�������
//static void delayUS( u32 times)
//{
//    times=times*7;      //  10us����7;
//    while(--times)
//        __nop();
//}

// ����MS������ʱ������������ֲʱ���ⲿ�ļ�������
static void delay_ms(u32 ms)
{
    ms = ms * 6500;
    for (u32 i = 0; i < ms; i++); // 72MHzϵͳʱ���£����ٸ���ѭ��Լ��ʱ1ms
}




// ���ų�ʼ��
static void gpioInit(void)
{
    // ����SWD����ģʽ�����ͷ�PB3, PB4, PA15
    RCC->APB2ENR |= 1<<0;           // ��������ʱ��       
    AFIO->MAPR   &= 0XF8FFFFFF;     // ��0MAPR��[26:24]
    AFIO->MAPR   |= 0x2<<24;        // ����ģʽ  000:ȫ��   010��ֻ��SWD   100:ȫ�� 
    
    // ʹ�ñ�׼��
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE); // PORTʱ��ʹ��
    // SCK
    GPIO_InitStructure.GPIO_Pin   = LCD_SCK_PIN  ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;           // �����������
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

    LCD_CS_HIGH ;   // ����������ֹ��ƽ����
    LCD_BL_HIGH ;   // ����Ļ��ʾ
}

// SPIͨ��Э���ʼ��
// ��Ҫ����ΪSPI���߿��ܹ��ض��Ҫ��ͬͨ�Ų������豸�������ÿ��ͨ��ǰ������һ�����ú���
static void spiInit(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB2PeriphClockCmd(LCD_SPI_SCK,  ENABLE);                       // SPI1ʱ��ʹ��

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��  SPI_Direction_1Line_Tx
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       // ����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                   // ����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                         // ѡ���˴���ʱ�ӵ���̬:ʱ�����ո�
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                        // ���ݲ����ڵڶ���ʱ����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                           // NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ2
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  // ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7;                            // CRCֵ����Ķ���ʽ
    SPI_Init(SPI1, &SPI_InitStructure);                                 // ����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI_Cmd(LCD_SPI_PORT, ENABLE);                                      // ʹ��SPI����
    //SPI2_ReadWriteByte(0xff);//��������
}

// SPI���ʹ���1�ֽ�
static u8 spiSendByte(u8 data)
{
    static u16 retry;

    retry = 0;
    while ((LCD_SPI_PORT -> SR & 2) == 0)
    {
        // �ȴ�������Ϊ��
        retry++;
        if (retry > 500) return 0;
    }
    LCD_SPI_PORT ->DR = data;

    retry = 0;
    while ((LCD_SPI_PORT -> SR & 1) == 0)
    {
        // �ȴ�����������
        retry++;
        if (retry > 250) return 0;
    }
    return LCD_SPI_PORT -> DR ;
}



/*****************************************************************************
*��  ����LCD_Init
*��  �ܣ���ʼ��lcd
*��  ����Ϊ�˼���⡢������ֲ������(��Ļ���ء���ʾ����)��h�ļ�������޸�
*����ֵ����
*��  ע��
*****************************************************************************/
void LCD_Init(void)
{
    xLCD.InitOK = 0;
    xLCD.bColor  = BLACK;

    // ��ʾ��������ƥ��
    if ((LCD_DIR == 1) || (LCD_DIR == 3))
    {
        xLCD.width  = LCD_WIDTH ;       // ��������أ�������ֵ����оƬ���Զ����У�ע�⣺�����Ļ�ұ��л������ͼӴ����ֵ
        xLCD.height = LCD_HIGH ;        // ���߶����أ� ע�⣺�����Ļ�����л������ͼӴ����ֵ
    }
    else
    {
        xLCD.width  = LCD_HIGH;
        xLCD.height = LCD_WIDTH;
    }

    // ��ʼ��GPIO
    gpioInit();
    // ��ʼ��SPI���衢Э��
    spiInit();

    // ��Ļ��λ
#ifdef LCD_RST_GPIO
    LCD_RST_LOW ;          // LCD_RST=0      //SPI�ӿڸ�λ
    delay_ms(10);           // delay 20 ms
    LCD_RST_HIGH  ;        // LCD_RST=1
#endif
    delay_ms(200);          // delay 20 ms

    sendOrder(0x11);       // �˳�˯��ģʽ
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
    if (LCD_DIR == 1)  sendByte(0xC0); // ��Ҫ����ʾ������ƣ�C0/00/A0/60,  C8/08/A8/68
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

    // ���ڸ������ӵģ�������
    sendOrder(0x2a);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x00);
    sendByte(0x7f);
    // ���ڸ������ӵģ�������
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
// ��LCD���ͼĴ�����ַ(ָ��)
static void sendOrder(u16 order)
{
    LCD_CS_LOW;            // SPI�豸Ƭѡ���ͣ���ʼͨ��
    LCD_RS_LOW;            // RS��: ����ֵ�� RS��: �Ĵ�����ֵַ
    spiSendByte(order);    // ���� �Ĵ�����ֵַ
    LCD_CS_HIGH;           // SPI�豸Ƭѡ���ߣ�����ͨ��
}
// ��LCD������ֵ
static void sendByte(u8 data)
{
    LCD_CS_LOW;            // SPI�豸Ƭѡ���ͣ���ʼͨ��
    LCD_RS_HIGH;           // RS��: ����ֵ�� RS��: �Ĵ�����ֵַ
    spiSendByte(data);     // ����1�ֽ�
    LCD_CS_HIGH;           // SPI�豸Ƭѡ���ߣ�����ͨ��
}
// ��LCD����2���ֽ�
static void sendShort(u16 data)
{
    LCD_CS_LOW;            // SPI�豸Ƭѡ���ͣ���ʼͨ��
    LCD_RS_HIGH;           // RS��: ���ݣ� RS��: ָ��
    spiSendByte(data >> 8); // ����1�ֽ�
    spiSendByte(data);     // ����1�ֽ�
    LCD_CS_HIGH;           // SPI�豸Ƭѡ���ߣ�����ͨ��
}

//д�Ĵ���: �Ĵ�����ַ������
//static void sendRegisterValue(u8 reg, u16 val)
//{
//    sendOrder(reg);
//    sendShort(val);
//}

/*****************************************************************
 * ��  ����LCD_Display
 * ��  �ܣ���ʾ������
 * ��  ����0:Ϩ����0����
 * ����ֵ����
 *
 * ��  ע�� ħŮ�������Ŷӱ�д   �Ա� https://demoboard.taobao.com
******************************************************************/
void LCD_Display(uint8_t sw)
{
    if (sw == 0)
        LCD_BL_LOW ;
    else
        LCD_BL_HIGH ;
}

/*****************************************************************
 * ��  ����setCursor
 * ��  �ܣ�������ʾ�����ڴ�����д�������Զ�����
 * ��  ����xStart���������, yStart���������
 *         xEnd�����������㣬yEnd������������
 * ����ֵ����
 *
 * ��  ע�� ħŮ�������Ŷӱ�д   �Ա� https://demoboard.taobao.com
******************************************************************/
static void setCursor(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd)
{
    sendOrder(0x2A);
    sendByte(0x00);    // ��ʼλ��x��λ����Ϊ1.8������128*160, ������255, ֱ��д0ʡ��
    sendByte(xStart);  // ��ʼλ��x��λ��ֵ����ʱ�Զ������˸�8λ��Ҳʡ��������
    sendByte(0x00);    // ��ʼλ��y��λ
    sendByte(xEnd);    // ��ʼλ��xλλ

    sendOrder(0x2B);
    sendByte(0x00);
    sendByte(yStart);
    sendByte(0x00);
    sendByte(yEnd);

    sendOrder(0x2c);  // ����д��������
}

/*****************************************************************
 * ��  ����drawPoint
 * ��  �ܣ���һ����
 * ��  ����x���꣬y����, 16λ��ɫֵ
 * ����ֵ����
 *
 * ��  ע�� ħŮ�������Ŷӱ�д   �Ա� https://demoboard.taobao.com
 * ��  �� ���ϸ�����QȺ�ļ��У�1126717453  ����޸�_2020��09��01��
******************************************************************/
static void drawPoint(u16 x, u16 y, u16 color)
{
    setCursor(x, y, x, y);      //���ù��λ��
    sendShort(color);
}

/******************************************************************
 * �������� LCD_Fill
 * ��  �ܣ� ��ָ����������䵥����ɫ
 *          ��Ļ�����1��ʼ����Ļ���Һ��·�ʵ�����ж�������
 * ��  ���� u16 xStart, yStart  ���Ͻ���ʼ����
 *          u16 xEnd,   yEnd    ���½���ʼ����
 *          u32 Color           ������ɫ
 *
 * ��  ע�� ħŮ�������Ŷӱ�д   �Ա� https://demoboard.taobao.com
 * ��  �� ���ϸ�����QȺ�ļ��У�1126717453  ����޸�_2020��09��01��
 *****************************************************************/
void LCD_Fill(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd, u16 color)
{
    spiInit();                                      // ��ֹSPI�����������豸�޸���
    u32 pixel = (xEnd - xStart + 1) * (yEnd - yStart + 1); // ������������

    setCursor(xStart, yStart, xEnd, yEnd);          // �趨��䷶Χ
    while (pixel-- > 0)                             // ������ɫֵ
        sendShort(color);
}

/******************************************************************
 * �������� LCD_Line
 * ��  �ܣ� ����
 * ��  ���� u16 x1,y1  �������
 *          u16 x2,y2  �յ�����
 *          u32 Color  ��ɫ
 *****************************************************************/
void LCD_Line(u16 sX, u16 sY, u16 eX, u16 eY, u16 color)
{
    spiInit();                                      // ��ֹSPI�����������豸�޸���

    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = eX - sX; //������������
    delta_y = eY - sY;
    uRow = sX;
    uCol = sY;
    if (delta_x > 0)incx = 1; //���õ�������
    else if (delta_x == 0)incx = 0; //��ֱ��
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0; //ˮƽ��
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)distance = delta_x; //ѡȡ��������������
    else distance = delta_y;
    for (t = 0; t <= distance + 1; t++) //�������
    {
        drawPoint(uRow, uCol, color);//����
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

//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
// ԭ�����ṩ�Ļ�Բ�������о��е�ʧ�棬����������������
void LCD_Circle(u16 x0, u16 y0, u8 r, u16 color)
{
    spiInit();                                   // ��ֹSPI�����������豸�޸���
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);                           //�ж��¸���λ�õı�־
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
        //ʹ��Bresenham�㷨��Բ
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/******************************************************************
 * �������� LCD_Image
 * ��  �ܣ� ��ָ�����������ָ��ͼƬ����
 *          ͼƬ������font.h�ļ���.ֻ�ʺ�����ͼƬ����
 *          Image2Lcdת����ˮƽɨ�裬16λ���ɫ
 * ��  ���� u16 x,y     ���Ͻ���ʼ����
 *          u16 width   ͼƬ���
 *          u16 height  ͼƬ�߶�
 *          u8* image   ���ݻ����ַ
 * ��  ע�� ħŮ�������Ŷ�  ���ϴ��QȺ��1126717453        ����޸�_2020��09��2��
 *****************************************************************/
void LCD_Image(u16 x, u16 y, u16 width, u16 height, const u8 *image)
{
    spiInit();                                 // ��ֹSPI�����������豸�޸���

    u16 colorData = 0;
    u32 cnt = 0;

    for (u16 i = 0; i < height; i++)           // һ��һ�е���ʾ
    {
        setCursor(x, y + i, x + width, y + height); // �������ù��λ��
        for (u16 j = 0; j < width; j++)        // һ���У������£�������ش���
        {
            colorData = (image[cnt * 2 + 1] << 8) | image[cnt * 2];
            sendShort(colorData);              // д��16λ��ɫ����
            cnt++;
        }
    }
}

/******************************************************************
 * �������� LCD_Chinese16ForFile
 * ��  �ܣ� ��ʾ1��16�ź���,�ֿ�������font�ļ��У�ֻ�ʺ��������̶ֹ����
 *          PCtoLCD2002ȡģ������+����ʽ+˳��+C51��ʽ
 * ��  ���� uint16_t  x         ��ʼ����x
 *          uint16_t  y         ��ʼ����y
 *          uint8_t   index     �ֿ������ڻ����е�λ��
 *          uint16_t  fColor    ��ɫ
 *          uint16_t  bColor    ��ɫ
 * ��  ��:  ��
 *****************************************************************/
void LCD_Chinese16ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor)
{
    spiInit();                                      // ��ֹSPI�����������豸�޸���

    const u8 *temp = hanzi16 + index * 32;          // ��ȡ���������������еĵ���ʼλ��

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
 * �������� LCD_Chinese32ForFile
 * ��  �ܣ� ��ʾ1��32�ź���,�ֿ�������font�ļ��У�ֻ�ʺ��������̶ֹ����
 *          PCtoLCD2002ȡģ������+����ʽ+˳��+C51��ʽ
 * ��  ���� uint16_t  x         ��ʼ����x
 *          uint16_t  y         ��ʼ����y
 *          uint8_t   index     �ֿ������ڻ����е�λ��
 *          uint16_t  fColor    ��ɫ
 *          uint16_t  bColor    ��ɫ
 * ��  ��:  ��
 *****************************************************************/
void LCD_Chinese32ForFile(uint16_t x, uint16_t y, uint8_t index, uint16_t fColor, uint16_t bColor)
{
    spiInit();                                  // ��ֹSPI�����������豸�޸���

    const u8 *temp = hanzi32 + index * 128;     // ��ȡ���������������еĵ���ʼλ��

    for (u8 j = 0; j < 32; j++)                 // һ��һ�е�����д
    {
        setCursor(x, y + j, x + 32, y + j + 32); // ֻҪ��λ���׸����꣬���淢�͵����ݣ��ͻ�������У�ֱ���յ��µ�����
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
 * �������� drawAscii
 * ��  �ܣ� ��ָ��λ����ʾһ���ַ�
 * ��  ���� u16 x,y     ��ʼ����
 *          u8  num     Ҫ��ʾ���ַ�:" "--->"~"
 *          u8  size    �����С 12/16/24/32
 *          u32 fColor  ������ɫ
 *          u32 bColor  ������ɫ
 * ��  ע�� �ο�ԭ�Ӹ��Ұ�����Ĵ�����޸�  ���ϴ��QȺ��1126717453
 *****************************************************************/
void drawAscii(u16 x, u16 y, u8 num, u8 size, u32 fColor, u32 bColor)
{
    spiInit();                                        // ��ֹSPI�����������豸�޸���

    if (xLCD.InitOK == 0) return;

    u8 temp;
    u16 y0 = y;

    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // �õ�����һ���ַ���Ӧ������ռ���ֽ���
    num = num - ' ';                                   // �õ�ƫ�ƺ��ֵ��ASCII�ֿ��Ǵӿո�ʼȡģ������-' '���Ƕ�Ӧ�ַ����ֿ⣩
    for (u8 t = 0; t < csize; t++)
    {
        if (size == 12)         temp = asc2_1206[num][t]; // ����1206����
        else if (size == 16)    temp = asc2_1608[num][t]; // ����1608����
        else if (size == 24)    temp = asc2_2412[num][t]; // ����2412����
        else if (size == 32)    temp = asc2_3216[num][t]; // ����3216����
        else return;                                   // û�е��ֿ�

        for (u8 t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)   drawPoint(x, y, fColor); // ���� ����
            else            drawPoint(x, y, bColor);   // ���� ����
            temp <<= 1;
            y++;
            if (y >= xLCD.height)    return;            // ������Ļ�߶�(��)
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                if (x >= xLCD.width) return;           // ������Ļ���(��)
                break;
            }
        }
    }
}



// ��ʾ����
void drawGBK(uint16_t x, uint16_t y, uint8_t *font, uint8_t size, uint16_t fColor, uint16_t bColor)
{
    u8 temp;
    u16 y0 = y;
    u8 GBK[128];
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size); // �õ�����һ���ַ���Ӧ������ռ���ֽ���

#ifdef GBK_STORAGE_ADDR                          // �ж��Ƿ�ʹ��ħŮ�Ƽ���w25qxx.c�ļ�, ��ֹ�������; �������޸�
    W25qxx_ReadGBK(font, size, GBK);                 // �õ���Ӧ��С�ĵ�������
#endif

    spiInit();                                       // ��ֹSPI�����������豸�޸���
    for (u8 t = 0; t < csize; t++)
    {
        temp = GBK[t];                                 // �õ�GBK��������
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
 * ��  ���� LCD_String
 * ��  �ܣ� ����Ļ����ʾ�ַ���
 *          ֧��Ӣ�ġ�����
 *          ���š�Ӣ����ģ�����font.h�ļ��У�������ģ�����W25Q128��
 * ��  ���� uint16_t  lcd_x   ��ʼ����X
 *          uint16_t  lcd_y   ��ʼ����y
 *          char*     pFont
 *          uint8_t   size    �����С��12��16��24��32, ���������4�ֹ�񣬻�Ĭ��ʹ��24������
 *          uint32_t  fColor  ������ɫ
 * ����ֵ��
******************************************************************************/
void LCD_String(uint16_t lcd_x, uint16_t lcd_y, char *pFont, uint8_t size, uint16_t fColor, uint16_t bColor)
{
    if (xLCD .InitOK == 0) return;

    u16 xStart = lcd_x;

    if (size != 12 && size != 16 && size != 24 && size != 32) // �����С����
        size = 24;

    while (*pFont != 0)                   // ������ȡ�ַ������ݣ�ֱ��'\0'ʱֹͣ
    {
        if (lcd_x > (xLCD.width - size))  // ��λ���жϣ����������ĩ���Ͱѹ�껻��
        {
            lcd_x = xStart;
            lcd_y = lcd_y + size;
        }
        if (lcd_y > (xLCD.height - size)) // ��λ���жϣ����������ĩ���ͷ��أ��������
            return;

        if (*pFont < 127)                 // ASCII�ַ�
        {
            drawAscii(lcd_x, lcd_y, *pFont, size, fColor, bColor);
            pFont++;
            lcd_x += size / 2;
        }
        else                              // ������ʾ
        {
#ifdef GBK_STORAGE_ADDR
            drawGBK(lcd_x, lcd_y, (u8 *)pFont, size, fColor, bColor);  // �ж��Ƿ�ʹ��ħŮ�����Ͳ�Ӱ��ASCIIӢ���ַ������
#endif

            pFont = pFont + 2;            // ��һ��Ҫ��ʾ���������ڴ��е�λ��
            lcd_x = lcd_x + size;         // ��һ��Ҫ��ʾ����������Ļ�ϵ�Xλ��
        }
    }
}

/******************************************************************************
 * ��  ���� LCD_GUI
 * ��  �ܣ� �򵥵���ʾ����
 *          ����ʾ��LCD��������ʹ��
 * ��  ����
 * ����ֵ��
******************************************************************************/
void LCD_GUI(void)
{
    // ��ʾ������Ӣ��                                       // �ŵ㣺�������������ֿ⣬����ʾ���⺺��; ȱ�㣺��Ҫ�������������Ѵ����ֿ����ݵ�Flash
    LCD_String(18, 10, "�ۺϲ���",      24, YELLOW, BLACK); // ��ʾ�ַ���
    LCD_String(8, 40, "STM32-F103C8T6", 16, WHITE, BLACK);  // ��ʾ�ַ���

    // ��ʾ�ض�����                                         // �ŵ㣺���������ⲿFlash�ֿ⣬���ݴ��ڣ�font.h; ȱ�㣺����оƬ�ڲ�Flash��С��ֻ�ܴ��С���ֿ�
    LCD_Chinese16ForFile(35, 65, 2, BLACK, WHITE);          // "��", ��ģ���ݴ����font.h��������
    LCD_Chinese16ForFile(51, 65, 3, WHITE, RED);            // "ʾ"
    LCD_Chinese16ForFile(67, 65, 4, BLACK, GREEN);          // "��"
    LCD_Chinese16ForFile(83, 65, 5, WHITE, BLUE);           // "��"

    LCD_Image(40, 100, 60, 60, imageLoge);                  // ��ʾͼƬ

    LCD_Fill(1,   1,   8,   8, WHITE);                      // ��Ļ��䣬
    LCD_Fill(121,   1, 129,   8, RED);                      // ͨ���Ŵ󾵹۲����Ͻǡ����Ͻ�����ɫ�飬�����������ص�X���꣬��߱��ڵ��˵�1�У��ұ߶���1��129
    LCD_Fill(1, 152,   8, 160, GREEN);                      // �����ж�XY��������������λ���Ƿ����
    LCD_Fill(121, 152, 129, 160, BLUE);                     //

}






