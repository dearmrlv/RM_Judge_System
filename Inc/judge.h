#ifndef __JUDGE_H
#define __JUDGE_H

#include "stm32f4xx_hal.h"

typedef __packed struct
{
  uint16_t data_length;	// 数据帧中data的长庿
  uint8_t seq;	// 包序叿
  uint8_t CRC8;	// 包头CRC8校验
}frame_header;

typedef __packed struct
{
  uint16_t data_length;	// 数据长度
  uint8_t * data;	// 指向数据区域的指针
}frame_data;

typedef __packed struct
{
  frame_header header;
  uint16_t cmd_id;
  frame_data data;
  uint16_t frame_tail;
}frame;

/* Judge System Data */

// 1. 比赛状态数据：0x0001, 发送频率：1Hz
typedef __packed struct
{
	uint8_t game_type : 4;
	uint8_t game_progress : 4;
	uint16_t stage_remain_time;
}ext_game_status_t;

// 2. 比赛结果数据：0x0002, 发送频率：1Hz
typedef __packed struct
{
	uint8_t winner; 
}ext_game_result_t;

// 3. 机器人血量数据：0x0003。发送频率：1Hz
typedef __packed struct
{   
	uint16_t red_1_robot_HP;
	uint16_t red_2_robot_HP;
	uint16_t red_3_robot_HP;
	uint16_t red_4_robot_HP;
	uint16_t red_5_robot_HP;
	uint16_t red_7_robot_HP;
	uint16_t red_base_HP;
	uint16_t blue_1_robot_HP;
	uint16_t blue_2_robot_HP;
	uint16_t blue_3_robot_HP;
	uint16_t blue_4_robot_HP;
	uint16_t blue_5_robot_HP;
	uint16_t blue_7_robot_HP;
	uint16_t blue_base_HP; 
}ext_game_robot_HP_t; 

// 4. 场地事件数据：0x0101。发送频率：事件改变后发送 
typedef __packed struct
{
	uint32_t event_type;
}ext_event_data_t; 

// 5. 补给站动作标识：0x0102。发送频率：动作改变后发送
typedef __packed struct
{
	uint8_t supply_projectile_id;	// 1: 一号补给口； 2: 二号补给口
	uint8_t supply_robot_id;	// 补弹机器人ID
	uint8_t supply_projectile_step;	// 出弹口开启状态
	uint8_t supply_projectile_num;	// 补弹数量
}ext_supply_projectile_action_t; 

// 6. 请求补给站补弹子弹：cmd_id (0x0103)。发送频率：上限 10Hz。RM 对抗赛尚未开放
typedef __packed struct
{   
	uint8_t supply_projectile_id;
	uint8_t supply_robot_id;
	uint8_t supply_num;
} ext_supply_projectile_booking_t;

// 7. 裁判警告信息：cmd_id (0x0104)。发送频率：警告发生后发送
typedef __packed struct
{   
	uint8_t level;
	uint8_t foul_robot_id;
}ext_referee_warning_t; 

// 8. 比赛机器人状态：0x0201。发送频率：10Hz
typedef __packed struct 
{   
	uint8_t robot_id;
	uint8_t robot_level;
	uint16_t remain_HP;
	uint16_t max_HP;
	uint16_t shooter_heat0_cooling_rate;
	uint16_t shooter_heat0_cooling_limit;
	uint16_t shooter_heat1_cooling_rate;
	uint16_t shooter_heat1_cooling_limit;
	uint8_t mains_power_gimbal_output : 1;
	uint8_t mains_power_chassis_output : 1;
	uint8_t mains_power_shooter_output : 1;
}ext_game_robot_status_t; 

// 9. 实时功率热量数据：0x0202。发送频率：50Hz
typedef __packed struct
{
	uint16_t chassis_volt;	// mV
	uint16_t chassis_current;	// mA
	float chassis_power;	// W
	uint16_t chassis_power_buffer;	// J
	uint16_t shooter_heat0;	// 17mm
	uint16_t shooter_heat1;	// 42mm
}ext_power_heat_data_t;

// 10. 机器人位置：0x0203。发送频率：10Hz 
typedef __packed struct
{
	float x;
	float y;
	float z;
	float yaw;
}ext_game_robot_pos_t;

// 11. 机器人增益：0x0204。发送频率：状态改变后发送 
typedef __packed struct
{   
	uint8_t power_rune_buff; 
}ext_buff_t;

// 12. 空中机器人能量状态：0x0205。发送频率：10Hz 
typedef __packed struct
{   
	uint8_t energy_point;
	uint8_t attack_time; 
}aerial_robot_energy_t; 

// 13. 伤害状态：0x0206。发送频率：伤害发生后发送 
typedef __packed struct
{   
	uint8_t armor_id : 4;
	uint8_t hurt_type : 4;
}ext_robot_hurt_t;

// 14. 实时射击信息：0x0207。发送频率：射击后发送
typedef __packed struct
{   
	uint8_t bullet_type;
	uint8_t bullet_freq;
	float bullet_speed;
}ext_shoot_data_t; 

// 15. 子弹剩余发射数：0x0208。发送频率：1Hz 周期发送，空中机器人以及哨兵机器人主控发送 
typedef __packed struct
{   
	uint16_t bullet_remaining_num;
}ext_bullet_remaining_t;

typedef __packed struct
{
	ext_game_status_t GameStatus;
	ext_game_result_t GameResult;
	ext_game_robot_HP_t RobotHP;
	ext_event_data_t EventData;
	ext_supply_projectile_action_t SupplyAction;
	ext_supply_projectile_booking_t SupplyBooking;
	ext_referee_warning_t ReferWarn;
	ext_game_robot_status_t RobotStatus;
	ext_power_heat_data_t PowerHeat;
	ext_game_robot_pos_t RobotPos;
	ext_buff_t RobotBuff;
	aerial_robot_energy_t AerEnergy;
	ext_robot_hurt_t RobotHurt;
	ext_shoot_data_t ShootData;
	ext_bullet_remaining_t BulletRemain;
}judge_data;

short int find_head(uint8_t *buffer);
short int unpack(uint8_t *buffer, frame *frame_read);
short int getPowerHeat(frame frame_read, ext_power_heat_data_t *PowerHeat);
short int getSupplyAction(frame frame_read, ext_supply_projectile_action_t *SupplyAction);
short int getJData(frame frame_read, judge_data *JData);

#define RX_BUF_LEN 150

#define GAME_STAT_ID 0x0001
#define GAME_RESULT_ID 0x0002
#define ROBOT_HP_ID 0x0003
#define EVENT_ID 0x0101
#define SUPPLY_ACTION_ID 0x0102
#define SUPPLY_BOOKING_ID 0x0103
#define REFER_WARN_ID 0x0104
#define ROBOT_STAT_ID 0x0201
#define POWER_HEAT_ID 0x0202
#define ROBOT_POS_ID 0x0203
#define ROBOT_BUF_ID 0x0204
#define AER_ENERGY_ID 0x0205
#define ROBOT_HURT_ID 0x0206
#define SHOOT_DATA_ID 0x0207
#define BULLET_REMAIN_ID 0x0208
#define BETWEEN_ROBOTS_ID 0x0301

extern judge_data JData;
extern uint8_t RX_BUF[RX_BUF_LEN];		// 定义数据接收的缓冲区
extern frame tmp_frame;	// 拆包得到的frame


#endif
