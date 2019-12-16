#include "judge.h"
#include "crc.h"
#include "string.h"

uint8_t RX_BUF[RX_BUF_LEN];		// 定义数据接收的缓冲区
frame tmp_frame;	// 拆包得到的frame
judge_data JData;   // 包含所有的裁判系统数据

//将输入的32位转换为float值
float Change_to_flt(uint32_t x)
{
	int i;
	int reminder = 1;
	float res = 0;
	for(i=0; i<32; i++)
	{
		res = res + ((x>>i)%2) * reminder;				//取x的第i位，并与2的i次方相乘
		reminder = reminder*2;										//将reminder的值保持在2的i次方
	}
	return res;
}


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
	{
		memset((void *)&(frame_read->header.data_length), 0, sizeof(frame_read->header.data_length));
		return -1;
	}
	
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

//1. 获取比赛状态
short int getGameStatus(frame frame_read, ext_game_status_t *GameState)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 3)
		return -1;
	GameState->game_type = data[0]%16;		//game_type是四位数，只会保留0~3位的值,取余即可。
	GameState->game_progress = data[0]>>4;		//game_progress是四位数，data[0]是八位，将4~7位右移四位即可
	GameState->stage_remain_time = data[1]<<8 | data[2];
	return 0;
}

//2. 获取比赛结果，比赛结束后发送 
short int getGameResult(frame frame_read, ext_game_result_t *GameResult)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 1)
		return -1;
	GameResult->winner = data[0];
	return 0;
}

//3. *获取机器人血量
short int getRobotHP(frame frame_read, ext_game_robot_HP_t *RobotHP)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 28)
		return -1;
	RobotHP->red_1_robot_HP = data[0]<<8 | data[1];
	RobotHP->red_2_robot_HP = data[2]<<8 | data[3];
	RobotHP->red_3_robot_HP = data[4]<<8 | data[5];
	RobotHP->red_4_robot_HP = data[6]<<8 | data[7];
	RobotHP->red_5_robot_HP = data[8]<<8 | data[9];
	RobotHP->red_7_robot_HP = data[10]<<8 | data[11];
	RobotHP->red_base_HP = data[12]<<8 | data[13];
	RobotHP->blue_1_robot_HP = data[14]<<8 | data[15];
	RobotHP->blue_2_robot_HP = data[16]<<8 | data[17];
	RobotHP->blue_3_robot_HP = data[18]<<8 | data[19];
	RobotHP->blue_4_robot_HP = data[20]<<8 | data[21];
	RobotHP->blue_5_robot_HP = data[22]<<8 | data[23];
	RobotHP->blue_7_robot_HP = data[24]<<8 | data[25];
	RobotHP->blue_base_HP = data[26]<<8 | data[27];
	return 0;
}

//4. *获取场地事件的数据，事件改变后发送 
short int getEventData(frame frame_read, ext_event_data_t *EventData)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 4)
		return -1;
	EventData->event_type = data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
	return 0;
}

// 5. 获得场地补给站动作标识数据，动作改变后发送
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

 //6. 获得补给子弹的信息 (RM 对抗赛尚未开放)
short int getSupplyBullet(frame frame_read, ext_supply_projectile_booking_t *SupplyBullet)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 3)
		return -1;
	SupplyBullet->supply_projectile_id = data[0];
	SupplyBullet->supply_robot_id = data[1];
	SupplyBullet->supply_num = data[2];
	return 0;
}

//7. *获取裁判警告信息
short int getRefereeWarning(frame frame_read, ext_referee_warning_t *RefereeWarning)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 2)
		return -1;
	RefereeWarning->level = data[0];
	RefereeWarning->foul_robot_id = data[1];
	return 0;
}

//8. 获取比赛机器人数据
short int getRobotStatus(frame frame_read, ext_game_robot_status_t *RobotStatus)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 15)
		return -1;
	RobotStatus->robot_id = data[0];
	RobotStatus->robot_level = data[1];
	RobotStatus->remain_HP = data[2]<<8 |data[3];
	RobotStatus->max_HP = data[4]<<8 | data[5];
	RobotStatus->shooter_heat0_cooling_rate = data[6]<<8 |data[7];
	RobotStatus->shooter_heat0_cooling_limit = data[8]<<8 | data[9];
	RobotStatus->shooter_heat1_cooling_rate = data[10]<<8 | data[11];
	RobotStatus->shooter_heat1_cooling_limit = data[12]<<8 | data[13];
	RobotStatus->mains_power_gimbal_output = data[14]%2;
	RobotStatus->mains_power_chassis_output = (data[14]>>1)%2;
	RobotStatus->mains_power_shooter_output = (data[14]>>2)%2;
	return 0;
}

// 9. 获得实时功率热量数据
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

//10. 获取机器人的位置
short int getRobotPot(frame frame_read, ext_game_robot_pos_t *RobotPot)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 16)
		return -1;
	RobotPot->x = Change_to_flt(data[0]<<24 | data[1]<<16 | data[2]<<8 |data[3]);
	RobotPot->y = Change_to_flt(data[4]<<24 | data[5]<<16 | data[6]<<8 |data[7]);
	RobotPot->z = Change_to_flt(data[8]<<24 | data[9]<<16 | data[10]<<8 |data[11]);
	RobotPot->yaw = Change_to_flt(data[12]<<24 | data[13]<<16 | data[14]<<8 |data[15]);
	return 0;
}

//11. 获取机器人增益状态
short int getBuff(frame frame_read, ext_buff_t *Buff)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 1)
		return -1;
	Buff->power_rune_buff = data[0];
	return 0;
}

//12. *获取空中机器人能量状态
short int getAerEnergy(frame frame_read,  aerial_robot_energy_t *RobotEnergy)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 3)
		return -1;
	RobotEnergy->energy_point = data[0];
	RobotEnergy->attack_time = data[1]<<8 | data[2];
	return 0;
}

//13. *获取伤害状态
short int getRobotHurt(frame frame_read,  ext_robot_hurt_t *RobotHurt)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 1)
		return -1;
	RobotHurt->armor_id = data[0]%16;
	RobotHurt->hurt_type = data[0]>>4;
	return 0;
}

//14. 获取实时射击信息
short int getShoot(frame frame_read,  ext_shoot_data_t *Shootdata)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 6)
		return -1;
	Shootdata->bullet_type = data[0];
	Shootdata->bullet_freq = data[1];
	Shootdata->bullet_speed = Change_to_flt(data[2]<<24 | data[3]<<16 | data[4]<<8 | data[5]);
	return 0;
}

//15. 获取子弹发射数量
short int getBulletrem(frame frame_read, ext_bullet_remaining_t *Bulletrem)
{
	uint8_t *data = frame_read.data.data;
	if(frame_read.data.data_length != 2)
		return -1;
	Bulletrem->bullet_remaining_num = data[0]<<8 | data[1];
	return 0;
}

short int getJData(frame frame_read, judge_data *JData)
{
	switch(frame_read.cmd_id)
	{
		case GAME_STAT_ID:
			getGameStatus(frame_read, &JData->GameStatus);
			return 0;
		case GAME_RESULT_ID:
			getGameResult(frame_read, &JData->GameResult);
			return 0;
		case ROBOT_HP_ID:
			getRobotHP(frame_read, &JData->RobotHP);
			return 0;
		case EVENT_ID:
			getEventData(frame_read, &JData->EventData);
			return 0;
		case SUPPLY_ACTION_ID:
			getSupplyAction(frame_read, &JData->SupplyAction);
			return 0;	
		case SUPPLY_BOOKING_ID:
			getSupplyBullet(frame_read, &JData->SupplyBooking);
			return 0;
		case REFER_WARN_ID:
			getRefereeWarning(frame_read, &JData->ReferWarn);
			return 0;
		case ROBOT_STAT_ID:
			getRobotStatus(frame_read, &JData->RobotStatus);
			return 0;
		case POWER_HEAT_ID:
			getPowerHeat(frame_read, &JData->PowerHeat);
			return 0;
		case ROBOT_POS_ID:
			getRobotPot(frame_read, &JData->RobotPos);
			return 0;
		case ROBOT_BUF_ID:
			getBuff(frame_read, &JData->RobotBuff);
			return 0;
		case AER_ENERGY_ID:
			getAerEnergy(frame_read, &JData->AerEnergy);
			return 0;
		case ROBOT_HURT_ID:
			getRobotHurt(frame_read, &JData->RobotHurt);
			return 0;
		case SHOOT_DATA_ID:
			getShoot(frame_read, &JData->ShootData);
			return 0;
		case BULLET_REMAIN_ID:
			getBulletrem(frame_read, &JData->BulletRemain);
			return 0;
		default:
			return -1;
	}
}
