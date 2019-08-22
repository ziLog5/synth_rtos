/*
 **/
#include "main.h"
#include <math.h>


osThreadId LEDThread1Handle, LEDThread2Handle;

static void LED_Thread1(void const *argument);
static void LED_Thread2(void const *argument);

void SystemClock_Config(void);

#define NUM_SAMPLES_PER_WAVE 4096
class Waves {
	static float sine_array[NUM_SAMPLES_PER_WAVE];
public:
	static void init(void);
	static float get_sample_with_sample_number_sine(uint32_t);
};

//	static float sine_array[NUM_SAMPLES_PER_WAVE];
float Waves::sine_array[NUM_SAMPLES_PER_WAVE] {};


	
void Waves::init() {
	uint32_t i = 0;
	for (i = 0; i < NUM_SAMPLES_PER_WAVE; i++) {
		float phase_radians = (float)i / (float) NUM_SAMPLES_PER_WAVE * (float)  6.28318530718;
		float value = (float) sin(phase_radians);
		sine_array[i] = value;	
	}
}

float Waves::get_sample_with_sample_number_sine(uint32_t sample_number) {
	if (sample_number < NUM_SAMPLES_PER_WAVE)
		return sine_array[sample_number];
	else
		return 0;	
}
	

int main(void)
{
	HAL_Init(); 
	SystemClock_Config();
	Dac_1::init();
	Usart_2::init();
	Tim::init();
	Waves::init();
	char led1[] = "LED1"; 
	char led2[] = "LED2"; 
	//Dac_1::set_value(0xFFF);
	//Usart_2::transmit_byte('X');
	
	
	uint32_t test_sample_number {0};
	float test_sample {0};
	float test_sample_rel {0};
	bool buffer_add_success {false};
	
	
	while(1) {
		test_sample = Waves::get_sample_with_sample_number_sine(test_sample_number);
		test_sample_rel = test_sample * (float) 0.5 + (float) 0.5; 	

		buffer_add_success = Sample_buffer::add_sample(test_sample_rel);
		if (buffer_add_success)
			test_sample_number++;
		if (test_sample_number == NUM_SAMPLES_PER_WAVE)
			test_sample_number = 0;
	}
	
	/* Thread 1 definition */
	const osThreadDef_t os_thread_def_LED1 = \
     { led1, LED_Thread1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE };
  
	/*  Thread 2 definition */
	const osThreadDef_t os_thread_def_LED2 = \
	{ led2, LED_Thread2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE };
  
	/* Start thread 1 */	
	LEDThread1Handle = osThreadCreate(&os_thread_def_LED1, NULL);

	/* Start thread 2 */
	LEDThread2Handle = osThreadCreate(&os_thread_def_LED2, NULL);

	/* Start scheduler */
	osKernelStart();

	 /* We should never get here as control is now taken by the scheduler */
	for (;;);
}


extern "C" {
	void SysTick_Handler(void)
	{
		//Dac_1::set_value_fast(0xFFF);
		HAL_IncTick();
		osSystickHandler();
		//Dac_1::set_value_fast(0x000);
	}
}

void TIM2_IRQHandler_cpp(void) {
	//IRQ_objects::sample_tick++;
	float sample = Sample_buffer::get_next_sample().value;
	Dac_1::set_value_rel((float) sample);
}


extern "C" {
	void TIM2_IRQHandler(void)
	{
		TIM2_IRQHandler_cpp();
		HAL_TIM_IRQHandler(&(Tim::htim2));
	}
}

static void LED_Thread1(void const *argument)
{
	(void) argument;
	
	//while (1) ;
		//Dac_1::set_value_fast(0x7FF);

}


static void LED_Thread2(void const *argument)
{

	(void) argument;
	
	//while (1) ;
		//Dac_1::set_value_fast(0xFFF);

	
}


//Mainly autogenerated code
//180MHz clock
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct {}
	;
	RCC_ClkInitTypeDef RCC_ClkInitStruct {}
	;

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

