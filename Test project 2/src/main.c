/**
 * \file
 *
 * \brief Empty user application template
 *
 */
/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */
/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "shift_can/shift_can.h"


/** UART Interface */
#define CONF_UART            CONSOLE_UART
/** Baudrate setting */
#define CONF_UART_BAUDRATE   (115200UL)
/** Character length setting */
#define CONF_UART_CHAR_LENGTH  US_MR_CHRL_8_BIT
/** Parity setting */
#define CONF_UART_PARITY     US_MR_PAR_NO
/** Stop bits setting */
#define CONF_UART_STOP_BITS    US_MR_NBSTOP_1_BIT

void SetupPeripherals();
static void configure_console();
void message_received(shift_can_message_t* message);


int interruptCounter = 0;

int main (void)
{
				

    //Initialize System Clock
    sysclk_init();
	
	board_init();
	
	configure_console();


    //Peripheral Setup
    SetupPeripherals();    
	
    pio_set_output(PIOA, PIO_PA23, PIO_TYPE_PIO_OUTPUT_0, PIO_DEFAULT, PIO_DEFAULT);
    
	init_can_module();
	
	
	
    //Main Loop
    while(1)
    {
        //Flash LED at 1Hz
        //pio_toggle_pin(PIO_PA23_IDX);
        //delay_ms(500);
    }
}
//Initialize MCU Peripherals
void SetupPeripherals(void)
{
    //Disable watchdog timer
    wdt_disable(WDT);
    //Set PA19 (blue LED) as an output
    pio_set_output(PIOA, PIO_PA19, PIO_TYPE_PIO_OUTPUT_0, PIO_DEFAULT, PIO_DEFAULT);
    //Set PA20 (green LED) as an output
    pio_set_output(PIOA, PIO_PA20, PIO_TYPE_PIO_OUTPUT_0, PIO_DEFAULT, PIO_DEFAULT);
    //Enable Timer/Counter Clock
    sysclk_enable_peripheral_clock(ID_TC0); //Use TC0
    //Setup the Timer (note you will need the datasheet)
    tc_init(TC0, 0,                                 //Timer 0, Channel 0
    TC_CMR_TCCLKS_TIMER_CLOCK4 |     //Timer clock = internal MCK/128
    TC_CMR_WAVE |                    //Waveform generation mode
    TC_CMR_WAVSEL_UP_RC);            //Up count, clear on match RC
    //Set Compare match value
    tc_write_rc(TC0, 0, 46875);                        //Roughly 20Hz (120e6/128/46875)  
    //Enable the Interrupt in the TC
    tc_enable_interrupt(TC0, 0, TC_IER_CPCS);          //Timer 0, Channel 0, Compare match C
    //Load NVIC Vectors (SAM4S Interrupt Controller)
    irq_initialize_vectors();                          //Technically not required for SAM4S
    //Configure TC Interrupt in NVIC
    irq_register_handler(TC0_IRQn, 0);
    //Start the timer
    tc_start(TC0, 0);
}
//Timer/Counter 0 ISR Handler
ISR(TC0_Handler)
{
    //Read TC Status (required to reset this interrupt)
    uint32_t status = tc_get_status(TC0, 0);
    //If Compare Match, toggle LED
    if((status & TC_SR_CPCS) == TC_SR_CPCS)
    {
		interruptCounter++;
		if(interruptCounter == 200) {
			
			init_can_module();

			shift_can_message_t customMsg;
			
			pod_states_struct_t test_struct;
			test_struct.state = 69;
			
			uint8_t buffer[64];

			serialize_data(&test_struct, sizeof(test_struct), buffer);
			customMsg.data = buffer;
			customMsg.dataSize = sizeof(test_struct);
			customMsg.id = 0x15;
			
			send_message_module_0(&customMsg);
			
			pio_toggle_pin(PIO_PA23_IDX);
			interruptCounter = 0;			
			
		}
		
    }
}
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

void message_received(shift_can_message_t* message)
{
	printf("Message received!\n");
	//simple_struct_t* test_struct = (simple_struct_t*) message->data;
	pod_states_struct_t* recieved_msg = (pod_states_struct_t*) message->data;
	uint32_t id = message->id;
	uint32_t dataSize = message->dataSize;

	pio_toggle_pin(PIO_PA23_IDX);
	
	if(message->id == 0x15) {
		printf("WAZZZZZZZZZZZZZZZZZZAAAAAAAAAAAAAAAAAAAAAAAAAA in the beninging\n");
		
	}
	
	
	//test_message_received(message);
}