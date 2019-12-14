#include "judge.h"
#include "string.h"

uint8_t SOF = 0xA5;
uint8_t rx_buf[RX_BUF_LEN];

/*
  返回0xA5出现的位置, 找不到则返回-1
*/
int find_head(uint8_t *buffer)
{
  // 采用遍历的方法
  uint8_t i;
  for(i = 0; buffer[i] != SOF & i < RX_BUF_LEN; i++);
  return i == RX_BUF_LEN ? -1 : i;
}

bool unpack(uint8_t *buffer, frame * frame_read)
{
  /*
  1. 在buffer中找到0xA5，也就是先找到SOF
  2. 依次读取各个参数的值，赋给frame_read
  */
  int head;

  head = find_head(buffer);
  if(head < 0)
		return 0;
  frame_read.frame_header.data_length = buffer[head + 1]; 
}