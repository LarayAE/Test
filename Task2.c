#include "stdint.h"
#include "FreeRTOS_sockets.h"

static SendBufType	SendBuf[SIZE_BUF];
// Переменная для хранения созданного сокета. 
static Socket_t xSocket;
//TimeOut приёма 
static TickType_t xReceiveTimeout_ms = 1 / portTICK_PERIOD_MS;
//TimeOut передачи 
static TickType_t xSendTimeout_ms = 1 / portTICK_PERIOD_MS;
//адрес назначения и номер порта
struct freertos_sockaddr xDestinationAddress;

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

static uint8_t UDP_open_socket(void) 
{  
struct freertos_sockaddr xBindAddress; 

    // Создаём UDP- сокет. 
    xSocket = FreeRTOS_socket( FREERTOS_AF_INET, //Для сокета IPv4
                               FREERTOS_SOCK_DGRAM, 
                               FREERTOS_IPPROTO_UDP ); 
	
		//задаём адрес назначения и номер порта
		xDestinationAddress.sin_addr = FreeRTOS_inet_addr_quick( 192, 168, 0, 10 ); 
    xDestinationAddress.sin_port = FreeRTOS_htons(6666); 
	
    //Проверяем, что сокет создан успешно. 
    if( xSocket != FREERTOS_INVALID_SOCKET ) 
    { 

        /* Устанавливаем тайм-аут приема. */ 
        FreeRTOS_setsockopt( xSocket, 0,                     
                            FREERTOS_SO_RCVTIMEO, // Установка таймаута получения.  
                            &xReceiveTimeout_ms, //Значение таймаута. 
                            0 );                 
 

        /* Устанавливаем тайм-аут отправки. */ 
        FreeRTOS_setsockopt( xSocket, 0,                     
                            FREERTOS_SO_SNDTIMEO, //Установка таймаута отправки.
                            &xSendTimeout_ms,    // Значение таймаута. 
                            0 ); 
			
        xBindAddress.sin_port = FreeRTOS_htons(UDP_PORT); 
        if( FreeRTOS_bind( xSocket, &xBindAddress, sizeof( &xBindAddress ) ) == 0 ) 
        { 
          //Привязка прошла успешно.
					return 1;
        } 
				else
				{
					return 0;
				}
    } 
    else 
    { 
        //Недостаточно доступной памяти кучи для создания сокета.
				return 0;
    } 
}

//отправка подтверждения
static inline void Send_REQ_ACK(void * tx_buf, TypeCMD cmd, uint8_t id_pack)
{
	((HeadType *)tx_buf)->type_ack = cmd;
	((HeadType *)tx_buf)->id_pack = id_pack;
	//отправка подтверждения
	sixe_tx = FreeRTOS_sendto( xSocket, tx_buf, sizeof(HeadType), 0, &xDestinationAddress, sizeof( xDestinationAddress );
	//считаем, что отправка всегда проходит успешно
}

//задача, обработчик сетевого интерфейса
void vATaskRecvSend( void *pvParameters )
{
	int32_t sixe_rx = 0; //колличество принятых байт
	int32_t sixe_tx = 0; //колличество отправленных байт
	uint8_t rx_buf[sizeof(WriteCMDPackType)] = {0};
	uint8_t tx_buf[sizeof(WriteCMDPackType)] = {0};
	socklen_t xAddressLength = sizeof(xSourceAddress); 
	struct freertos_sockaddr xSourceAddress; 
	
	//создание сокета
	if(UDP_open_socket())
	{
		for( ;; )
		{
			//принимаем минимальный размер пакета
			sixe_rx = FreeRTOS_recvfrom( xSocket, rx_buf, sizeof(HeadType), 0, &xSourceAddress, &xAddressLength); 
			if(sixe_rx == sizeof(HeadType))
			{
				switch(((HeadType *)rx_buf)->type_ack)
				{
					case REPLY_ACK:
						for(uint8_t i = 0; i < SIZE_BUF; i++)
						{
							if((((HeadType *)rx_buf)->id_pack == SendBuf[i].pack.head.id_pack) && SendBuf[i].wait_reply_ack_flag)
							{
								//ответ получен от мастера
								SendBuf[i].wait_reply_ack_flag = 0;
							}
						}
					break;
					case READ_CMD:
						//принимаем адрес
						sixe_rx = FreeRTOS_recvfrom( xSocket, &((ReadCMDPackType *)rx_buf)->addr, sizeof(uint32_t), 0, &xSourceAddress, &xAddressLength);
						if(sixe_rx == sizeof(uint32_t))
						{
							//отправка подтверждения
							Send_REQ_ACK(tx_buf, REQ_ACK, ((HeadType *)rx_buf)->id_pack);
							
							((WriteCMDPackType *)tx_buf)->head.type_ack = DATA_ACK;
							((WriteCMDPackType *)tx_buf)->head.id_pack = ((HeadType *)rx_buf)->id_pack;
							((WriteCMDPackType *)tx_buf)->addr = ((ReadCMDPackType *)rx_buf)->addr;
							((WriteCMDPackType *)tx_buf)->val = reg_read(((ReadCMDPackType *)rx_buf)->addr);
							//отправка данных
							sixe_tx = FreeRTOS_sendto( xSocket, tx_buf, sizeof(WriteCMDPackType), 0, &xDestinationAddress, sizeof( xDestinationAddress );
              //считаем, что отправка всегда проходит успешно
							
							//ищем свободную структуру
							for(uint8_t i = 0; i < SIZE_BUF; i++)
							{
								if(!SendBuf[i].wait_reply_ack_flag)
								{
									//заполняем структуру на ожидание ответа
									SendBuf[i].wait_reply_ack_flag = 1;
									SendBuf[i].num_repeat = 0;
									SendBuf[i].pack = *((WriteCMDPackType *)tx_buf);
									break;
								}
							}
						}
					break;
					case WRITE_CMD:
						//принимаем адрес и значение
						sixe_rx = FreeRTOS_recvfrom( xSocket, &((WriteCMDPackType *)rx_buf)->addr, sizeof(uint32_t) * 2, 0, &xSourceAddress, &xAddressLength);
						if(sixe_rx == (sizeof(uint32_t) * 2))
						{
							if(reg_write(((WriteCMDPackType *)rx_buf)->addr, ((WriteCMDPackType *)rx_buf)->val))
							{
								//отправка подтверждения
								Send_REQ_ACK(tx_buf, REQ_ACK, ((HeadType *)rx_buf)->id_pack);
							}
							else
							{
								//отправка ошибки
								Send_REQ_ACK(tx_buf, ERR_nVALID, ((HeadType *)rx_buf)->id_pack);
							}
						}
					break;
					default:
					break;
				}
			}
			
			//повторная отправка 
			for(uint8_t i = 0; i < SIZE_BUF; i++)
			{
				if(SendBuf[i].wait_reply_ack_flag)
				{
					if(SendBuf[i].num_repeat == 0)//только отправили, повторная отправка не трбуется
					{
						SendBuf[i].num_repeat++;
					}
					else
					{
						//достигнуто максимальное количество повторных отправок
						if(SendBuf[i].num_repeat > NUM_REPEAT)
						{
							SendBuf[i].wait_reply_ack_flag = 0;
							continue;
						}
						
						//отправка данных
						sixe_tx = FreeRTOS_sendto( xSocket, &SendBuf[i].pack, sizeof(WriteCMDPackType), 0, &xDestinationAddress, sizeof( xDestinationAddress );
						//считаем, что отправка всегда проходит успешно						
					}
				}
			}
		}
	}
		
	vTaskDelete( NULL );
}

