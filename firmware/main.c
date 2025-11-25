/*******************************************************************************************************************//**
	AmiCA USB PD power supply
	\author Peter Mulholland
***********************************************************************************************************************/

#define F_CPU	8000000UL
#define STM8S003

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stm8s.h"

#include "husb238.h"

//----------------------------------------------------------------------------------------------------------------------
// Definitions
/*
	STM8S003F3P6

	PA1		GPIO_Output, Green LED
	PA2		GPIO_Output, Red LED
	PA3		GPIO_Input, HUSB238 "GATE" pin state

	PB4		I2C_SCL
	PB5		I2C_SDA

	PC4		ADC_IN2 - VBUS voltage
	PC5		GPIO_Output, PWRON (high = on)

	PD1		SWIM
	PD2		ADC_IN3 - current sense 
	PD4		GPIO_Input, Power switch (low = on)
	PD5		UART1_TXD
	PD6		UART1_RXD
*/

// Bits to set in PA_ODR for the LEDs
#define RED_LED		(1 << 2)
#define GREEN_LED	(1 << 1)
#define AMBER_LED	((1 << 1) | (1 << 2))

//#define _DEBUG

// Retry I2C this many times before giving up
#define MAX_I2C_TRIES		5

#define MAX_ATTACH_TRIES	5

//----------------------------------------------------------------------------------------------------------------------
// Global variables

uint8_t volatile outon;

//----------------------------------------------------------------------------------------------------------------------
// Local Functions

inline void delay_ms(uint32_t ms)
 {
	for (uint32_t i = 0; i < ((F_CPU / 18 / 1000UL) * ms); i++)
		__asm__("nop");
}

#ifdef _DEBUG
int putchar(int c)
{
	UART1_SendData8((uint8_t) c);
	while(UART1_GetFlagStatus(UART1_FLAG_TC) == RESET)
		;

	return 0;
}
#endif

void i2c_sendaddr(uint8_t Address, I2C_Direction_TypeDef Direction)
{
	for(ErrorStatus match = ERROR, nack = ERROR; !match;)
	{
		I2C_Send7bitAddress(Address, Direction);

		do
		{
			nack = I2C_CheckEvent(I2C_EVENT_SLAVE_ACK_FAILURE);

			match = (Direction == I2C_DIRECTION_TX) ?
				I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) :
				I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
		} while (!match || nack);

		if (nack)
			puts("i2c_sendaddr: Address NACK!");
	}
}

void husb238_write_reg(uint8_t reg, uint8_t val)
{
	while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY))
		;

	I2C_GenerateSTART(ENABLE);
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
		;

	i2c_sendaddr(HUSB238_I2CADDR, I2C_DIRECTION_TX);

	I2C_SendData(reg);
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (I2C_CheckEvent(I2C_EVENT_SLAVE_ACK_FAILURE))
			puts("husb238_write_reg: register Tx NACK!");
	}

	I2C_SendData(val);
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (I2C_CheckEvent(I2C_EVENT_SLAVE_ACK_FAILURE))
			puts("husb238_write_reg: value Tx NACK!");
	}

	I2C_GenerateSTOP(ENABLE);
}

void husb238_read_reg(uint8_t reg, uint8_t *val)
{
	while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY))
		;

	I2C_GenerateSTART(ENABLE);
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
		;

	i2c_sendaddr(HUSB238_I2CADDR, I2C_DIRECTION_TX);

	I2C_SendData(reg);
	while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (I2C_CheckEvent(I2C_EVENT_SLAVE_ACK_FAILURE))
			puts("husb238_read_reg: register Tx NACK!");
	}

	I2C_GenerateSTART(ENABLE);

	while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
		;

	i2c_sendaddr(HUSB238_I2CADDR, I2C_DIRECTION_RX);

	I2C_AcknowledgeConfig(I2C_ACK_NONE);
	I2C_GenerateSTOP(ENABLE);

	while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
		;

	*val = I2C_ReceiveData();
}

//----------------------------------------------------------------------------------------------------------------------
// ISRs

void trap_isr(void) __trap
{
	// A trap wil cause the LED to blink Amber/Red
	disable_interrupts();
	while(1)
	{
		PA_ODR = AMBER_LED;
		delay_ms(500);
		PA_ODR = RED_LED;
		delay_ms(500);
	}
}

// IRQ when HUSB238 GATE pin changes
void porta_isr(void) __interrupt(EXTI0_ISR)
{
//	gateon = ((PA_IDR & (1<<3)) == 0);
}

// IRQ when power switch state changes
void portc_isr(void) __interrupt(EXTI3_ISR)
{
//	outon = ((PD_IDR & (1<<4)) == 0);
}

// IRQ on TIM1 
void tim1_isr(void) __interrupt(TIM1_OVF_ISR)
{
}

//----------------------------------------------------------------------------------------------------------------------
// Main

void main(void)
{
	// ------------------------------------------------------------------------
	// Configure system clocks

	// Use HSI clock at 8MHz (default)
	//CLK_CKDIVR = 0x18;				// HSI / 8, CPU / 1

	// Enable clock for I2C, UART1, ADC and TIMER1
	CLK_PCKENR1 = (CLK_PCKENR1_I2C | CLK_PCKENR1_UART1 | CLK_PCKENR1_TIM1);
	CLK_PCKENR2 = CLK_PCKENR2_ADC;

	// ------------------------------------------------------------------------
	// Configure GPIO pins

	// Port A
	// PA1 GPIO_Output, Green LED
	// PA2 GPIO_Output, Red LED
	// PA3 GPIO_Input, HUSB238 "GATE" pin state

	PA_DDR = 0b00000110;			// PA1-2 output, PA3 input
	PA_CR1 = 0b00000110;			// PA1-2 p-p out, PA3 no pullup
	PA_CR2 = 0b00001000;			// PA1-2 2MHz out, PA3 interrupt

	// Port B - PB4 and PB5 are used for I2C

	PB_DDR = 0;
	PB_CR1 = PB_CR2 = 0;

	// Port C
	// PC3 = unused
	// PC4 = ADC_IN2 - VBUS voltage
	// PC5 = GPIO_Output, output on (high = on)
	// PC6 = unused
	// PC7 = unused
	
	PC_DDR = 0b00100000;			// PC5 output, all others input
	PC_CR1 = 0b00100000;			// PC5 p-p out
	PC_CR2 = 0b00000000;			// PC5 2MHz out

	// Port D
	// PD1 = SWIM
	// PD2 = ADC_IN3 - current sense 
	// PD3 = not used
	// PD4 = GPIO_Input, Power switch (low = on)
	// PD5 = UART1_TXD
	// PD6 = UART1_RXD

	PD_DDR = 0b00000000;			// All inputs
	PD_CR1 = 0b00010010;			// PD4 pullup on, SWIM pullup on
	PD_CR2 = 0b00010000;			// PD4 interrupt on state change

	// ------------------------------------------------------------------------
	// Configure ADC

	ADC1_TDRL = 0xFF;					// Disable input schmitt triggers
	ADC1_TDRH = 0xFF;
	
	ADC1_CSR = 2;						// no EOC interrupt, no analog watchdog, default to ADC_IN2

	ADC1_CR3 = 0;						// No data buffer
	ADC1_CR2 = (1 << ADC1_CR2_ALIGN);	// RH aligned data
	ADC1_CR1 = (1 << ADC1_CR1_ADON);	// Clock prescaler /2, Single shot mode, enable

	// ------------------------------------------------------------------------
	// Configure interrupt controller

	// PDIS = 11 (rising and falling edge interrupts)
	// PCIS = 11 (rising and falling edge interrupts)
	EXTI_CR1 = (1<<7) | (1<<6) | (1<<5) | (1<<4);
	EXTI_CR2 = 0;

	// ------------------------------------------------------------------------
	// If debug build, Initialise UART 1

	#ifdef _DEBUG
		UART1_Init(9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
		UART1_Cmd(ENABLE);
		(void) UART1_SR;	// dummy read - clear the status register
	#endif

	// ------------------------------------------------------------------------
	// Initialise I2C bus

	I2C_Init(100000, 0x00, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, F_CPU / 1000000UL);
	I2C_Cmd(ENABLE);

	// ------------------------------------------------------------------------
	// Wait for the HUSB238 to attach to the PD source

	PA_ODR = RED_LED;

	bool is_attached = false;
	for (int try = 0; try < MAX_ATTACH_TRIES; try++)
	{
		uint8_t sr1;
		husb238_read_reg(HUSB238_PD_STATUS1, &sr1);

		// Are we attached?
		if (sr1 & HUSB238_STATUS1_ATTACH)
		{
			is_attached = true;
			break;
		}

		// Wait a little while and retry
		delay_ms(500);
	}

	if (!is_attached)
	{
		// We cant attach - either a cable issue or the source isnt PD
		// Blink LED red and stop
	}

	// ------------------------------------------------------------------------
	// Look at the supported PDOs and find one to use

	uint8_t selected_pdo = 0;

	for (uint8_t pdoreg = HUSB238_SRC_PDO_15V; pdoreg <= HUSB238_SRC_PDO_20V; pdoreg++)
	{
		uint8_t pdoval;

		// Read this PDO's data
		husb238_read_reg(pdoreg, &pdoval)

		// Is it supported ? If not, try the next one
		if ((pdoval & 0x80) == 0)
			continue;

		// Can we support at least 2.5A ?
		if ((pdoval & 0x0F) >= CURRENT_2_50_A)
		{
			selected_pdo = pdoreg;
			break;
		}
	}

	// Did we get a usable PDO?
	if (selected_pdo == 0)
	{
		// Cant find a usable PDO
		// Blink LED red and stop
	}

	// ------------------------------------------------------------------------
	// Select the PDO

	husb238_write_reg(HUSB238_SRC_PDO,(uint8_t) selected_pdo << 4);
	husb238_write_reg(HUSB238_GO_COMMAND, HUSB238_CMD_REQUEST_PDO);

	// Wait for it to take effect
	
	for (;;)
	{
		uint8_t sr0, pdo_voltage, pdo_current;
		husb238_read_reg(HUSB238_PD_STATUS0, &sr0);

		pdo_voltage = (sr0 >> 4) & 0x0f;
		pdo_current = sr0 & 0x0f

		if (pdo_voltage == selected_pdo)
			break;

		// TODO timeout here?
	}

	// ------------------------------------------------------------------------
	// Loop here

	while (1)
	{
		bool outon = ((PD_IDR & (1<<4)) == 0);

		// Update output state

		if (outon)
		{
			// power on
			PC_ODR |= (1 << 5);
			PA_ODR = GREEN_LED;
		}
		else
		{
			// power off
			PC_ODR &= ~(1 << 5);
			PA_ODR = RED_LED;
		}

		// Wait for interrupt
		__asm__("wfi");
	}

	// We shouldnt get here!
	disable_interrupts();
	halt();
}
