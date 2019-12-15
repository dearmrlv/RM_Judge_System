#include "judge.h"
#include "crc.h"

uint8_t RX_BUF[RX_BUF_LEN];		// 定义数据接收的缓冲区
frame tmp_frame;	// 拆包得到的frame
judge_data JData;   // 包含所有的裁判系统数据

/*
  返回0xA5出现的位罿, 找不到则返回-1
*/
short int find_head(uint8_t *buffer)
{
  // 采用遍历的方法
  uint8_t i;
  for(i = 0; buffer[i] != 0xA5 && i < RX_BUF_LEN; i++);
  return i == RX_BUF_LEN ? -1 : i;
}

// buffer是缓冲区地址，frame_read是已经建立的帧存放地坿
short int unpack(uint8_t *buffer, frame *frame_read)
{
  /*
  1. 在buffer中找刿0xA5，也就是先找到SOF
  2. 依次读取各个参数的忼，赋给frame_read
  */
  int head;

  head = find_head(buffer);
  if(head < 0)
		return -1;
  
  // 帧头CRC检验
  if(!Verify_CRC8_Check_Sum((unsigned char *)&buffer[head], 5))
		return -1;
	
  // frame_read->header.data_length = (buffer[head + 1]<<8) + buffer[head + 2];
	frame_read->header.data_length = buffer[head + 1];	// buffer[head + 2]丿般是0x00
	
	if(!Verify_CRC16_Check_Sum(&buffer[head], 9 + frame_read->header.data_length))
		return -1;
	
	if(head + buffer[head + 1] >= RX_BUF_LEN){
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
		return -1;
	}
	
	frame_read->header.seq = buffer[head + 3];
	frame_read->header.CRC8 = buffer[head + 4];
	
	// frame_read->cmd_id = (buffer[head + 5]<<8) + buffer[head + 6];
	frame_read->cmd_id = buffer[head + 5]<<8 | buffer[head + 6];
	
	frame_read->data.data_length = frame_read->header.data_length;
	frame_read->data.data = &buffer[head + 7];
	
	// frame_read->frame_tail = (frame_read->data.data[frame_read->data.data_length] << 8)
	//														+ frame_read->data.data[frame_read->data.data_length + 1];
	frame_read->frame_tail = frame_read->data.data[frame_read->data.data_length] << 8
															| frame_read->data.data[frame_read->data.data_length + 1];
	
	return 0;
}

// 获得实时功率热量数据
short int getPowerHeat(frame frame_read, ext_power_heat_data_t *PowerHeat)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 14)
		return -1;
	PowerHeat->chassis_volt = data[0]<<8 | data[1];
	PowerHeat->chassis_current = (data[2]<<8) + data[3];
	PowerHeat->chassis_power = data[7]<<24 | data[6]<<16 | data[5]<<8 | data[4];
	PowerHeat->chassis_power_buffer = data[8]<<8 | data[9];
	PowerHeat->shooter_heat0 = data[10]<<8 | data[11];
	PowerHeat->shooter_heat1 = data[12]<<8 | data[13];
	return 0;
}

// 获得场地补给站动作标识数据，动作改变后发送
short int getSupplyAction(frame frame_read, ext_supply_projectile_action_t *SupplyAction)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 4)
		return -1;
	SupplyAction->supply_projectile_id = data[0];
	SupplyAction->supply_robot_id = data[1];
	SupplyAction->supply_projectile_step = data[2];
	SupplyAction->supply_projectile_num = data[3];
	return 0;
}

short int getJData(frame frame_read, judge_data *JData)
{
	switch(frame_read.cmd_id)
	{
		case POWER_HEAT_ID:
			getPowerHeat(frame_read, &JData->PowerHeat);
			return 0;
		case SUPPLY_ACTION_ID:
			getSupplyAction(frame_read, &JData->SupplyAction);
			return 0;
		default:
			return -1;
	}
}
