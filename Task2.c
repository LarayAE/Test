#include "stdint.h"
#include "FreeRTOS_sockets.h"

static SendBufType	SendBuf[SIZE_BUF];

//чтение регистра
static uint32_t reg_read(uint32_t addr)
{
	return *((uint32_t *)addr);
}
 
//запись регистра
//возвращает 1 - при успешой записи, 0 - ошибка записи
static uint32_t reg_write(uint32_t addr, uint32_t value)
{
	if(VALID_ADDR(addr))
	{
		*((uint32_t *)addr) = value;
		return	1;
	}
	else
	{
		return 0;
	}
}

void vATaskRecvSend( void *pvParameters )
{
		for( ;; )
		{
				-- Task application code here. --
		}

		
		vTaskDelete( NULL );
}

