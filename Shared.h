#pragma once
#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>

//These queue handles need to be shared between main and exceptions files
extern QueueHandle_t queue_handle;
extern QueueHandle_t uart_byte_queue_handle;

//Holds data for MIDI 'Note On' message
struct Note_on_struct {
	uint8_t note_number {0}	;
	uint8_t velocity {0}	;
	//TODO: Is this needed? 
	uint32_t get_frequecy()
	{
		return note_number;
	}
}
;

//Holds data for MIDI 'Control Change' message
struct Controller_change_struct {
	uint8_t controller_number {0}	;
	uint8_t controller_data {0}	;
};







