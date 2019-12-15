# RM_Judge_System

## 工作进展
* 完成了2019年裁判系统的串口数据读取与信息获取
* 尚未完成CRC检验

## 文件分布
* 裁判系统读取的代码放在 [./Inc/judge.h](https://github.com/dearmrlv/RM_Judge_System/blob/master/Inc/judge.h) 及 [./Src/judge.c](https://github.com/dearmrlv/RM_Judge_System/blob/master/Src/judge.c) 中
* CRC的参考代码放在 [./Inc/crc.h](https://github.com/dearmrlv/RM_Judge_System/tree/master/Src) 和 [./Src/crc.c](https://github.com/dearmrlv/RM_Judge_System/blob/master/Src/crc.c) 中

## 代码使用指南
* 本项目由STM32CubeMX生成，使用TTL串口协议与裁判系统通讯，需要开启USART及其中断功能
* 需要更新裁判系统的2019年版的固件库至最新版本
* 已经定义的全局变量：
```C
judge_data JData;	// 裁判系统发送的所有数据
uint8_t RX_BUF[RX_BUF_LEN];		// 定义数据接收的缓冲区
frame tmp_frame;	// 拆包得到的frame
```
* 裁判系统的数据读取由
```short int getJData(frame frame_read, judge_data *JData)```函数实现，存储在变量```JData```中


## 尚未解决的问题
* 串口有时候会收到cmd_id为0x302的数据包，而此对应的数据包类型在[协议手册](https://github.com/RoboMaster/referee_serial_port_protocol/blob/master/doc/cn/裁判系统串口协议附录%20V2.0.pdf
)中没有提及
* CRC的原理与参考代码的修改与使用
* 把float的二进制表示变为float型

## TO DO
* 完成裁判系统其他类型的数据包的处理（目前只处理了实时功率热量以及场地补给动作信息）
* 在程序中搭载CRC功能
* 完成整套裁判系统的搭建，包括单片机的处理、Client和Server的搭建
