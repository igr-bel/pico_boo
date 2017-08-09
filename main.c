/*
 * pico_boo_mega8.c
 *
 * Created: 03.08.2017 12:53:29
 * Author : Igor
 */ 
//===================================================================================================================================================
#include "prj.h"
//---------------------------------------------------------------------------------------------------------------------------------------------------
//#define DBG
//---------------------------------------------------------------------------------------------------------------------------------------------------
#define human_push_button_delay		1
//---------------------------------------------------------------------------------------------------------------------------------------------------
#define btn_rec_pin		0		//PORTB
#define btn_1_pin		1
#define btn_2_pin		2
#define btn_3_pin		3
#define btn_4_pin		4

#define led_rec_pin		1		//PORTC
#define led_1_pin		2
#define led_2_pin		3
#define led_3_pin		4
#define led_4_pin		5

#define btn_rec_pressed ( PINB & (1<<btn_rec_pin) )
#define btn_1_pressed	( PINB & (1<<btn_1_pin) )
#define btn_2_pressed	( PINB & (1<<btn_2_pin) )
#define btn_3_pressed	( PINB & (1<<btn_3_pin) )
#define btn_4_pressed	( PINB & (1<<btn_4_pin) )

#define led_rec_on  ( PORTC |= (1<<led_rec_pin) )
#define led_rec_off ( PORTC &= ~(1<<led_rec_pin) )

#define led_1_on  ( PORTC |= (1<<led_1_pin) )
#define led_1_off ( PORTC &= ~(1<<led_1_pin) )

#define led_2_on  ( PORTC |= (1<<led_2_pin) )
#define led_2_off ( PORTC &= ~(1<<led_2_pin) )

#define led_3_on  ( PORTC |= (1<<led_3_pin) )
#define led_3_off ( PORTC &= ~(1<<led_3_pin) )

#define led_4_on  ( PORTC |= (1<<led_4_pin) )
#define led_4_off ( PORTC &= ~(1<<led_4_pin) )

//---------------------------------------

#define trigger_pin		5		//PORTB
#define led_trigger_pin	0		//PORTC

#define led_trigger_on  ( PORTC |= (1<<led_trigger_pin) )
#define led_trigger_off ( PORTC &= ~(1<<led_trigger_pin) )

#define trigger_active	( !(PINB & (1<<trigger_pin)) )

//---------------------------------------

#define out_1_pin	7		//PORTD
#define out_2_pin	6
#define out_3_pin	5
#define out_4_pin	4

#define out_1_high	( PORTD |= (1<<out_1_pin) )
#define out_1_low	( PORTD &= ~(1<<out_1_pin) )

#define out_2_high	( PORTD |= (1<<out_2_pin) )
#define out_2_low	( PORTD &= ~(1<<out_2_pin) )

#define out_3_high	( PORTD |= (1<<out_3_pin) )
#define out_3_low	( PORTD &= ~(1<<out_3_pin) )

#define out_4_high	( PORTD |= (1<<out_4_pin) )
#define out_4_low	( PORTD &= ~(1<<out_4_pin) )

//---------------------------------------

#define df_busy_pin	0	//PORTD

#define df_busy	( !( PIND & (1<<df_busy_pin) ) )

#define play_1	length = 8; uart_tx_buffer( data_buff_1, &length );
#define play_2	length = 8; uart_tx_buffer( data_buff_2, &length );


//---------------------------------------------------------------------------------------------------------------------------------------------------
void init(void);
void prog(void);
//---------------------------------------------------------------------------------------------------------------------------------------------------

//---DF player commands
unsigned char data_buff_0[] = {0x7E, 0xFF, 0x06, 0x09, 0x00, 0x00, 0x02, 0xEF};	//SD card
unsigned char data_buff_1[] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x01, 0xEF};	//play 1st track from root directory
unsigned char data_buff_2[] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x02, 0xEF};	//play 2nd track from root directory
		 int length;
//---------------------------------------------------------------------------------------------------------------------------------------------------
//---DBG
#ifdef DBG 
	unsigned char dbg_buff_0[] = {"in main"};
	unsigned char dbg_buff_1[] = {"in init"};
	unsigned char dbg_buff_2[] = {"in prog"};
#endif

//---------------------------------------------------------------------------------------------------------------------------------------------------
unsigned char btn_state_prev;
unsigned char btn_state_crnt;
unsigned int addr_cntr;
	
unsigned int saved_time;	
//---------------------------------------------------------------------------------------------------------------------------------------------------
unsigned char voice_1_started_flag;
unsigned char voice_2_started_flag;
unsigned char rec_enable;
//---------------------------------------------------------------------------------------------------------------------------------------------------
unsigned char btn_1_state_arr[127];
unsigned char btn_2_state_arr[127];
unsigned char btn_3_state_arr[127];
unsigned char btn_4_state_arr[127];
//---------------------------------------------------------------------------------------------------------------------------------------------------

//===================================================================================================================================================
int main(void)
{	
	init();
		
	#ifdef DBG
		length = sizeof(dbg_buff_0);
		uart_tx_buffer( dbg_buff_0, &length );
		_delay_ms(100);
	#endif
	
	unsigned char saved_time_low;
	unsigned char saved_time_high;
	
	unsigned long stop_time;
	
	unsigned char red_flash_flag = 0;
	
//---------------------------------------------------------------------------------------------------------------------------------------------------	
    while (1) 
    {
		//record new sequence
		if (btn_rec_pressed && rec_enable) prog();
		
//------------------------------------------------------------
		
		//write protecting (toggle rec_enable)
		if (btn_1_pressed && btn_2_pressed)
		{
			_delay_ms(1000);
			if (btn_1_pressed && btn_2_pressed)
			{
				while(EECR & (1 << EEWE));
				EEAR = 511;
				EECR |= (1<<EERE);
				rec_enable = EEDR;
				
				rec_enable = !rec_enable;
				
				if (!rec_enable)
				{
					led_1_on;
					_delay_ms(300);
					led_rec_on;
					_delay_ms(50);
					led_1_off;
					_delay_ms(1000);
					led_rec_off;
				}//if
				else
				{
					led_rec_on;
					_delay_ms(300);
					led_1_on;
					_delay_ms(50);
					led_rec_off;
					_delay_ms(1000);
					led_1_off;
				}//else
				
				while(EECR & (1 << EEWE));
				EEAR = 511;
				EEDR = rec_enable;
				EECR |= (1<<EEMWE);
				EECR |=(1<<EEWE);
				
			}//if
		}//if (rec_enable toggle)
		
//------------------------------------------------------------		
		
		#ifndef DBG
			if (!voice_2_started_flag)
			{
				play_2;
				voice_2_started_flag = 1;
				voice_1_started_flag = 0;
			}//if
		#endif	

//------------------------------------------------------------
//------------------------------------------------------------
		
		addr_cntr = 0;		
		stop_time = 0;
		btn_state_crnt = 0;
		
		if ( btn_4_pressed || trigger_active )
		{					
			if (trigger_active) led_trigger_on;
			else led_rec_on;
			
			//read EEPROM
			while(EECR & (1 << EEWE));
			EEAR = addr_cntr;
			EECR |= (1<<EERE);
			saved_time_low = EEDR;
			addr_cntr++;
			
			while(EECR & (1 << EEWE));
			EEAR = addr_cntr;
			EECR |= (1<<EERE);
			saved_time_high = EEDR;
			addr_cntr++;
			
//---do-------------------------------			
			 //while ( (saved_time_high != 0) && (saved_time_low != 0) )
			 do
			 {
				stop_time = millis(start);	//start timer & stop_time = 0
				
				btn_state_crnt = (saved_time_high >> 4);
				saved_time = ( (unsigned int)(saved_time_high & 0x0F) << 8 );
				saved_time = saved_time | saved_time_low;

				#ifndef DBG
					if ( (btn_state_crnt != 0) && !voice_1_started_flag )
					{
						play_1;
						voice_1_started_flag = 1;
						voice_2_started_flag = 0;
					}//if
				#endif			
			
				//btn_leds and outputs
				//---LEDs
				PORTC &= ~( (1<<led_1_pin) | (1<<led_2_pin) | (1<<led_3_pin) | (1<<led_4_pin) );
				PORTC |= ( btn_state_crnt << 2 );

				//---outputs
				PORTD &= ~ ( (1<<out_1_pin) | (1<<out_2_pin) | (1<< out_3_pin) | (1<<out_4_pin) );
				PORTD |= ( (btn_state_crnt<<7) | ((btn_state_crnt<<5) & 0x40) | ((btn_state_crnt<<3) & 0x20) | ((btn_state_crnt<<1) & 0x10) );								
										
				while ( stop_time < saved_time )
				{
					//blink rec led
					if ( red_flash_flag < 50 )
					{
						led_rec_on;
						red_flash_flag++;					
					}//if
					else if ( (red_flash_flag >= 50) && (red_flash_flag < 100) )
					{
						led_rec_off;
						red_flash_flag++;
					}//else if
					else red_flash_flag = 0;
					
					stop_time = millis(get);
					_delay_ms(1);
					if ( (saved_time == 0x0FFF) && (stop_time > 0xFFA) ) break;		//
				}//while
				
				//read next pause value from EEPROM
				if (addr_cntr < 508)
				{				
					while(EECR & (1 << EEWE));
					EEAR = addr_cntr;
					EECR |= (1<<EERE);
					saved_time_low = EEDR;
					addr_cntr++;
					
					while(EECR & (1 << EEWE));
					EEAR = addr_cntr;
					EECR |= (1<<EERE);
					saved_time_high = EEDR;
					addr_cntr++;
				}//if
				
				if (btn_4_pressed) break;
				
			}while ( saved_time != 0 );
//---while-------------------------------
			
			PORTC &= ~( (1<<led_1_pin) | (1<<led_2_pin) | (1<<led_3_pin) | (1<<led_4_pin) );
			PORTD &= ~ ( (1<<out_1_pin) | (1<<out_2_pin) | (1<< out_3_pin) | (1<<out_4_pin) );
			
			led_trigger_off;
			
			led_rec_off;
			
			while (trigger_active);
			
		}//if(start the game)
		
    }//while(1)
}//main()

//===================================================================================================================================================
void init(void)
{
	#ifdef DBG
		length = sizeof(dbg_buff_1);
		uart_tx_buffer( dbg_buff_1, &length );
		_delay_ms(100);
	#endif

	//---------
	//ports
	//B
	DDRB &= ~( (1<<btn_rec_pin) | (1<<btn_1_pin) | (1<<btn_2_pin) | (1<<btn_3_pin) | (1<<btn_4_pin) | (1<<trigger_pin) );
	PORTB |= (1<<trigger_pin);	//pull_up
	//C
	DDRC |= ( (1<<led_rec_pin) | (1<<led_1_pin) | (1<<led_2_pin) | (1<<led_3_pin) | (1<<led_4_pin) | (1<<led_trigger_pin) );
	//D
	DDRD |= ( (1<<out_1_pin) | (1<<out_2_pin) | (1<<out_3_pin) | (1<<out_4_pin) );
	
	//---------
	//uart
	uart_init(MYUBRR);
	
	//---------
	//DF
	#ifndef DBG
		_delay_ms(3000);
		length = 8;
		uart_tx_buffer( data_buff_0, &length );
		_delay_ms(500);
	#endif
	//---------
	//millis()
	millis(on);
	
	//---------
	//
	voice_1_started_flag = 0;
	voice_2_started_flag = 0;
	PORTC &= ~( (1<<led_rec_pin) | (1<<led_1_pin) | (1<<led_2_pin) | (1<<led_3_pin) | (1<<led_4_pin) );
	PORTD &= ~ ( (1<<out_1_pin) | (1<<out_2_pin) | (1<< out_3_pin) | (1<<out_4_pin) );
	
	while(EECR & (1 << EEWE));
	EEAR = 511;
	EECR |= (1<<EERE);
	rec_enable = EEDR;
	
	 
}//init()
//---------------------------------------------------------------------------------------------------------------------------------------------------
void prog(void)
{
	#ifdef DBG
		length = sizeof(dbg_buff_2);
		uart_tx_buffer( dbg_buff_2, &length );
		_delay_ms(100);
	#endif

	btn_state_crnt = 0;
	addr_cntr = 0; 
	
	led_rec_on;
	while (btn_rec_pressed);
	
	//start prog blink
	for (unsigned char i=0; i<5; i++)
	{
		led_rec_off;
		_delay_ms(25);
		led_rec_on;
		_delay_ms(15);
	}//for
	
	millis(start);
	
	while ( (2+2) != 5 )
	{
		btn_state_crnt = ( (PINB >> 1) & 0x0F );
		
		if ( (btn_state_crnt != 0) && !voice_1_started_flag )
		{
			voice_1_started_flag = 1;
			play_1;
		}//if (start scare voice) 
		
		if (btn_state_crnt != btn_state_prev)
		{
			//EEPROM write
			saved_time = millis(get);
			while(EECR & (1 << EEWE));
			EEAR = addr_cntr;
			EEDR = (unsigned char)saved_time;
			EECR |= (1<<EEMWE);
			EECR |=(1<<EEWE);
			addr_cntr++;
						
			saved_time = (unsigned char)(saved_time >> 8);
			while(EECR & (1 << EEWE));
			EEAR = addr_cntr;
			EEDR = ( (btn_state_prev << 4) | saved_time );
			EECR |= (1<<EEMWE);
			EECR |=(1<<EEWE);
			addr_cntr++;
			
			do											//skip a short pause while pressing several buttons at the same time
			{
				btn_state_prev = btn_state_crnt;
				_delay_ms(human_push_button_delay);
				btn_state_crnt = ( (PINB >> 1) & 0x0F );
			} while (btn_state_prev != btn_state_crnt);

			//btn_leds and outputs
			//---leds
			PORTC &= ~( (1<<led_1_pin) | (1<<led_2_pin) | (1<<led_3_pin) | (1<<led_4_pin) );
			PORTC |= ( btn_state_crnt << 2 );
			//---outputs
			PORTD &= ~ ( (1<<out_1_pin) | (1<<out_2_pin) | (1<< out_3_pin) | (1<<out_4_pin) );
			PORTD |= ( (btn_state_crnt<<7) | ((btn_state_crnt<<5) & 0x40) | ((btn_state_crnt<<3) & 0x20) | ((btn_state_crnt<<1) & 0x10) );
			
			millis(start);

		}//if
		
		if (millis(get) >= 4096)
		{
			//EEPROM write
			while(EECR & (1 << EEWE));
			EEAR = addr_cntr;
			EEDR = 0xFF;
			EECR |= (1<<EEMWE);
			EECR |=(1<<EEWE);
			addr_cntr++;
						
			while(EECR & (1 << EEWE));
			EEAR = addr_cntr;
			EEDR = (unsigned char)( (btn_state_prev << 4) | 0x0F );
			EECR |= (1<<EEMWE);
			EECR |=(1<<EEWE);
			addr_cntr++;
			
			millis(start);
			
		}//if(time overflow 4096 ms)
		
		if (508 == addr_cntr) break;	//if(memory overflow)
		
		if (btn_rec_pressed) break;		//if(prog completed)
		
	}//while
	
	//---write prog sequence end attribute
	if (addr_cntr <= 508)
	{
		while(EECR & (1 << EEWE));
		EEAR = addr_cntr;
		EEDR = 0x00;
		EECR |= (1<<EEMWE);
		EECR |=(1<<EEWE);
		addr_cntr++;
		
		while(EECR & (1 << EEWE));
		EEAR = addr_cntr;
		EEDR = 0x00;
		EECR |= (1<<EEMWE);
		EECR |=(1<<EEWE);

	}//if
	
	//end prog blink
	for (unsigned char i=0; i<3; i++)
	{
		led_rec_off;
		_delay_ms(50);
		led_rec_on;
		_delay_ms(200);
	}//for
	
	voice_1_started_flag = 0;
	
	led_rec_off;
	
	return;
}//prog()

//---------------------------------------------------------------------------------------------------------------------------------------------------

//===================================================================================================================================================