#include "main.h"
/*
 USART1_IRQHandler --uart_byte_queue--> Thread 2 --midi_command_queue_handle--> Thread 3
 MAIN TODO:produce test setup to at least give tests for refactoring
 TODO: make sure optimiation is on
 */
//Thread 2 recieves UART data and processes this to send midi messages to task 1
void thread2(void *);
void SystemClock_Config(void);
//TODO: rename this
//Queue from task 1 to task 2 with midi data
QueueHandle_t midi_command_queue_handle {nullptr};
//Queue from USART1_IRQHandler to thread2 , raw uart data
QueueHandle_t uart_byte_queue_handle {nullptr};

int main(void)
{
	HAL_Init(); 
	SystemClock_Config();
	//Dac1 used to output the waveform
	Dac_1::init();
	//Dac2 used for debug/measurement
	Dac_2::init();
	//Usart 1 used for MIDI input
	Usart_1::init();
	//Usart2 used for virtual com port on PC
	Usart_2::init();
	//Tim2 triggers systick which outputs the sample to Dac1
	Tim::init();
	//Waves holds precalculated waveform information. The calculation needs to be done before the main tasks start
	Waves::init();
	//For checking success of task creation
	BaseType_t taskCreateReturn;
	//delete?
	Usart_2::transmit_byte('x');
	//See queue descriptions above
	midi_command_queue_handle = xQueueCreate(10, sizeof(Midi_command_queue_message));
	uart_byte_queue_handle = xQueueCreate(10, sizeof(uint8_t));
	taskCreateReturn = xTaskCreate(thread1, "Add Voice and Calculate Sample", 2048, NULL, 1, NULL);	
	if (taskCreateReturn != pdPASS)
		//error
		while(1);
	taskCreateReturn = xTaskCreate(thread2, "Process Midi", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	if (taskCreateReturn != pdPASS)
		//error
		while(1);
	vTaskStartScheduler();
	 /* We should never get here as control is now taken by the scheduler */
	for (;;);
}

//TODO: Make sure 'front' is the right end of the queue
//TODO: This should be a seperate file for thread 1

//This is the callback for handling the note_on midi message
//This could be included in the Midi_in class but defineing it here makes the code clearer
void handle_note_on(Note_on_struct note_on_struct) {
	Midi_command_queue_message message;
	message.message_type = note_on;
	message.note_on_struct.velocity_byte = note_on_struct.velocity_byte;
	message.note_on_struct.note_number = note_on_struct.note_number;
	xQueueSendToFront(midi_command_queue_handle, &message, portMAX_DELAY);
} 
//See comments for previous function
void handle_controller_change(Controller_change_struct controller_change_struct) {
	Midi_command_queue_message message	;
	message.message_type = control_change;
	xQueueSendToFront(midi_command_queue_handle, &message, portMAX_DELAY);
} 

//This thread looks for messages from the UART1 IRQ and handles the data in the Midi_in class
void thread2(void *argument)
{
	(void) argument;
	uint8_t message {0}	;
	while (1) {
		message = 0;
		const BaseType_t xQueueReceiveReturn = xQueueReceive(uart_byte_queue_handle, &message, portMAX_DELAY);
		//if (message != 0) {
			//Midi_in::handle_midi_byte(message, handle_note_on, handle_controller_change);
		//}
		if (xQueueReceiveReturn == pdTRUE) {
			Midi_in::handle_midi_byte(message, handle_note_on, handle_controller_change);
		}	
	}
}

//Mainly autogenerated code
//180MHz clock
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct {}	;
	RCC_ClkInitTypeDef RCC_ClkInitStruct {}	;

	/** Configure the main internal regulator output voltage 
	*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	//Tuned for my microcontroller
	RCC_OscInitStruct.PLL.PLLN = 374;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		while (1) ;
	}
	/** Activate the Over-Drive mode 
	*/
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		while (1) ;
	}
	/** Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
	                            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		while (1) ;
	}
}

