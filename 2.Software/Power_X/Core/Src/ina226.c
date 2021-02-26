#include "ina226.h"
//	模拟IIC:

INA226 INA226_data;

static void delay_nns(uint16_t D) //30纳秒ns  根据手册要用到IIC的HS高速模式
{
    while (--D)
        ;
}

void delay_nms(uint16_t ms) //毫秒
{
    uint16_t i;
    uint32_t M = 0; //720W
    for (i = 0; i < ms; i++)
        for (M = 12000; M > 0; M--)
            ;
}

void delay_nus(uint16_t us) //微秒
{
    uint16_t i;
    uint16_t M = 0; //720W
    for (i = 0; i < us; i++)
        for (M = 72; M > 0; M--)
            ;
}

void INA226_IIC_Init(void)
{
    // GPIO_InitTypeDef GPIO_Initure;
    // __HAL_RCC_GPIOB_CLK_ENABLE(); //使能GPIOB时钟

    // GPIO_Initure.Pin = GPIO_PIN_6 | GPIO_PIN_7; //PB10SCL PB11SDA
    // GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;    //推挽输出
    // GPIO_Initure.Pull = GPIO_PULLUP;            //上拉
    // GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;  //高速
    // HAL_GPIO_Init(GPIOB, &GPIO_Initure);

    IIC_SDA_H;
    IIC_SCL_H;
    delay_nms(5);
}

void INA226_IIC_Start(void)
{
    IIC_SDA_H;
    IIC_SCL_H;
    delay_nns(5);
    IIC_SDA_L; //START:when CLK is high,DATA change form high to low
    delay_nns(5);
    IIC_SCL_L; //钳住I2C总线，准备发送或接收数据
    delay_nns(5);
}

void INA226_IIC_Stop(void)
{
    IIC_SDA_L; //STOP:when CLK is high DATA change form low to high
    delay_nns(5);
    IIC_SCL_H;
    delay_nns(5);
    IIC_SDA_H; //发送I2C总线结束信号
    delay_nns(5);
}

void INA226_IIC_Ack(void)
{
    IIC_SDA_L;
    delay_nns(5);
    IIC_SCL_H;
    delay_nns(5);
    IIC_SCL_L;
    delay_nns(5);
    IIC_SDA_H;
}

void INA226_IIC_NAck(void)
{
    IIC_SDA_H;
    delay_nns(5);
    IIC_SCL_H;
    delay_nns(5);
    IIC_SCL_L;
    delay_nns(5);
    IIC_SDA_L;
}

uint8_t INA226_IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    //SDA_IN();      //SDA设置为输入
    IIC_SDA_H;
    delay_nns(5);
    IIC_SCL_H;
    delay_nns(5);

    while (READ_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            INA226_IIC_Stop();
            return 1;
        }
    }
    IIC_SCL_L; //时钟输出0
    return 0;
}

void INA226_IIC_Send_Byte(uint8_t txd)
{
    IIC_SCL_L; //拉低时钟开始数据传输
    for (uint8_t i = 0; i < 8; i++)
    {
        if (txd & 0x80)
            IIC_SDA_H;
        else
            IIC_SDA_L;
        txd <<= 1;

        IIC_SCL_H;
        delay_nns(5);
        IIC_SCL_L;
        delay_nns(5);
    }
    //    IIC_SDA_H;
    //    IIC_SCL_H;            //当去掉wait_ack时的时候添加
    //    delay_nns(5);
    //IIC_SCL_L;
}

uint8_t INA226_IIC_Read_Byte(unsigned char ack)
{
    uint8_t TData = 0, i;
    for (i = 0; i < 8; i++)
    {
        IIC_SCL_H;
        delay_nns(5);
        TData = TData << 1;
        //if(GPIOB->IDR& GPIO_IDR_IDR7)        //判断SDA是否为高
        if (SDA_READ)
        {
            TData |= 0x01;
        }
        IIC_SCL_L;
        delay_nns(5);
    }
    if (!ack)
        INA226_IIC_NAck();
    else
        INA226_IIC_Ack();
    //    IIC_SCL_L;
    //    delay_nns(5);
    //    IIC_SDA_L;
    //    delay_nns(5);                //ACK
    //    IIC_SCL_H;
    //    delay_nns(5);
    //    IIC_SCL_L;
    //    delay_nns(5);
    //    IIC_SDA_H;
    return TData;
}

/**********************IIC_END*************************/

//初始化INA226
void mx_ina226_init(void)
{

    INA226_IIC_Init();
    INA226_SendData(INA226_ADDR1, CFG_REG, 0x8000); //重新启动

    INA226_SendData(INA226_ADDR1, CFG_REG, 0x484f); //设置转换时间204us,求平均值次数128，采样时间为204*128，设置模式为分流和总线连续模式
    INA226_SendData(INA226_ADDR1, CAL_REG, CAL);    //设置分辨率
    INA226_Get_ID(INA226_ADDR1);                    //获取ina226的id
}

//设置寄存器指针
void INA226_SetRegPointer(uint8_t addr, uint8_t reg)
{
    INA226_IIC_Start();

    INA226_IIC_Send_Byte(addr);
    INA226_IIC_Wait_Ack();

    INA226_IIC_Send_Byte(reg);
    INA226_IIC_Wait_Ack();

    INA226_IIC_Stop();
}

//发送,写入数据
void INA226_SendData(uint8_t addr, uint8_t reg, uint16_t data)
{
    uint8_t temp = 0;
    INA226_IIC_Start();

    INA226_IIC_Send_Byte(addr);
    INA226_IIC_Wait_Ack();

    INA226_IIC_Send_Byte(reg);
    INA226_IIC_Wait_Ack();

    temp = (uint8_t)(data >> 8);
    INA226_IIC_Send_Byte(temp);
    INA226_IIC_Wait_Ack();

    temp = (uint8_t)(data & 0x00FF);
    INA226_IIC_Send_Byte(temp);
    INA226_IIC_Wait_Ack();

    INA226_IIC_Stop();
}

//读取数据
uint16_t INA226_ReadData(uint8_t addr)
{
    uint16_t temp = 0;
    INA226_IIC_Start();

    INA226_IIC_Send_Byte(addr + 1);
    INA226_IIC_Wait_Ack();

    temp = INA226_IIC_Read_Byte(1);
    temp <<= 8;
    temp |= INA226_IIC_Read_Byte(0);

    INA226_IIC_Stop();
    return temp;
}
//1mA/bit
uint16_t INA226_GetShunt_Current(uint8_t addr)
{
    uint16_t temp = 0;
    INA226_SetRegPointer(addr, CUR_REG);
    temp = INA226_ReadData(addr);
    if (temp & 0x8000)
        temp = ~(temp - 1);
    return temp;
}

//获取id
void INA226_Get_ID(uint8_t addr)
{
    uint32_t temp = 0;
    INA226_SetRegPointer(addr, INA226_GET_ADDR);
    temp = INA226_ReadData(addr);
    INA226_data.ina226_id = temp;
}

//获取校准值
uint16_t INA226_GET_CAL_REG(uint8_t addr)
{
    uint32_t temp = 0;
    INA226_SetRegPointer(addr, CAL_REG);
    temp = INA226_ReadData(addr);
    return (uint16_t)temp;
}

//1.25mV/bit
uint16_t INA226_GetVoltage(uint8_t addr)
{
    uint32_t temp = 0;
    INA226_SetRegPointer(addr, BV_REG);
    temp = INA226_ReadData(addr);
    return (uint16_t)temp;
}

//2.5uV/bit
uint16_t INA226_GetShuntVoltage(uint8_t addr)
{
    int16_t temp = 0;
    INA226_SetRegPointer(addr, SV_REG);
    temp = INA226_ReadData(addr);
    if (temp & 0x8000)
        temp = ~(temp - 1);
    return (uint16_t)temp;
}

//获取电压
void GetVoltage(float *Voltage) //mV
{
    *Voltage = INA226_GetVoltage(INA226_ADDR1) * Voltage_LSB;
}

//获取分流电压
void Get_Shunt_voltage(float *Voltage) //uV
{
    *Voltage = (INA226_GetShuntVoltage(INA226_ADDR1) * INA226_VAL_LSB); //如需矫正电流分流参数请将这里改为2.5
}

//获取电流
void Get_Shunt_Current(float *Current) //mA
{
    *Current = (INA226_GetShunt_Current(INA226_ADDR1) * CURRENT_LSB);
}

//获取功率= 总线电压 * 电流
void get_power() //W
{
    GetVoltage(&INA226_data.voltageVal);           //mV
    Get_Shunt_voltage(&INA226_data.Shunt_voltage); //uV
    Get_Shunt_Current(&INA226_data.Shunt_Current); //mA
    Get_Power(&INA226_data.Power);
    INA226_data.Power_Val = INA226_data.voltageVal * 0.001f * INA226_data.Shunt_Current * 0.001f; //mV*mA
}

//获取功率装载值,ina226内部计算的的功率，由于未经校准，故不采用

uint16_t INA226_Get_Power(uint8_t addr)
{
    int16_t temp = 0;
    INA226_SetRegPointer(addr, PWR_REG);
    temp = INA226_ReadData(addr);
    return (uint16_t)temp;
}

//获取功率,ina226内部计算，不准确，不采用
void Get_Power(float *Power) //W
{
    *Power = (INA226_Get_Power(INA226_ADDR1) * POWER_LSB);
}

//不设置报警，舍弃
/*
uint8_t INA226_AlertAddr()
{
	uint8_t temp;
	IIC_Start();

	IIC_Send_Byte(INA226_GETALADDR);
	IIC_Wait_Ack();
	
	temp = IIC_Read_Byte(1);
	
	IIC_Stop();
	return temp;
}
*/
