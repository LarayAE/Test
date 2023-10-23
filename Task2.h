#ifndef TASK2_H
#define TASK2_H
#include <stdint.h>

#define SIZE_BUF 	(uint8_t)10				//колличество пакетов ожидающих подтверждения со стороны мастера
#define NUM_REPEAT		(uint8_t)5		//колличество повторных попыток передачи

//выравнивание структур в 1 байт
#pragma pack(push, 1)

typedef enum
{
	REQ_ACK = 0,	//подтверждение приёма ведомым
	REPLY_ACK, 		//подтверждение приёма мастером
	DATA_ACK,			//ответ ведомого на READ_CMD
	READ_CMD,			//команда на чтение регистра
	WRITE_CMD,		//команда на запись регистра
	NUM_CMD
}TypeCMD;

//id_pack берётся ведомым из команд чтения\записи. Мастер подставляет id_pack в REPLY_ACK
//Мастер инкементирует id_pack при отправке нового запроса
typedef struct
{
	TypeCMD	type_ack; 
	uint8_t	id_pack; //номер пакета
}HeadType;

//структура пакета передаваемая мастером с командой READ_CMD
typedef struct
{
	HeadType	head;
	uint32_t addr;
}ReadCMDPackType;

//структура пакета передаваемая мастером с командой WRITE_CMD
//и передаваемая ведомым как ответ на READ_CMD от мастера
typedef struct
{
	HeadType	head;
	uint32_t addr;
	uint32_t val;
}WriteCMDPackType;

typedef struct
{
	uint8_t wait_reply_ack_flag;	//флаг ожидания ответа от мастера
	uint8_t num_repeat;						//количество выполненых попыток передачи
	WriteCMDPackType	pack;				
}SendBufType;

#pragma pack(pop)

#endif



