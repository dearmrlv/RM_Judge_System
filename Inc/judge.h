/* USER CODE BEGIN Header */

/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __JUDGE_H
#define __JUDGE_H

#ifdef __cplusplus
extern "C" {
#endif

struct frame_header
{
  // uint8_t SOF;   // 数据帧起始字节，固定值为0xA5 
  uint16_t data_length;   // 数据帧中data的长度
  uint8_t seq;    // 包序号
  uint8_t CRC8;   // 包头CRC8校验
};

struct frame_data
{
  uint16_t data_length;   // 数据长度
  uint8_t * data;     // 指向数据区域的指针
};

struct frame
{
  frame_header header;
  uint16_t cmd_id;
  frame_data data;
  uint16_t frame_tail;
};


#define RX_BUF_LEN 100

extern uint8_t SOF = 0xA5;
extern uint8_t rx_buf[RX_BUF_LEN];

int find_head(uint8_t *buffer);

bool unpack(uint8_t *buffer, frame * frame_read);

#ifdef __cplusplus
}
#endif

#endif /* __JUDGE_H */