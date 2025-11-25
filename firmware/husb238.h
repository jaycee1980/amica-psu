/*******************************************************************************************************************//**
	AmiCA USB PD power supply
	\author Peter Mulholland
***********************************************************************************************************************/

#pragma once

#define HUSB238_I2CADDR				(0x08<<1)	// I2C address

#define HUSB238_PD_STATUS0			0x00
#define HUSB238_PD_STATUS1			0x01
#define HUSB238_SRC_PDO_5V			0x02
#define HUSB238_SRC_PDO_9V			0x03
#define HUSB238_SRC_PDO_12V			0x04
#define HUSB238_SRC_PDO_15V			0x05
#define HUSB238_SRC_PDO_18V			0x06
#define HUSB238_SRC_PDO_20V			0x07
#define HUSB238_SRC_PDO				0x08
#define HUSB238_GO_COMMAND			0x09

#define HUSB238_STATUS1_CCDIR		(1<<7)
#define HUSB238_STATUS1_ATTACH		(1<<6)

#define HUSB238_CMD_REQUEST_PDO		0b00001
#define HUSB238_CMD_GET_SRC_CAP		0b00100
#define HUSB238_CMD_HARD_RESET		0b10000

/**  Available currents per PD output */
typedef enum _husb_currents
{
	CURRENT_0_5_A  = 0b0000,
	CURRENT_0_7_A  = 0b0001,
	CURRENT_1_0_A  = 0b0010,
	CURRENT_1_25_A = 0b0011,
	CURRENT_1_5_A  = 0b0100,
	CURRENT_1_75_A = 0b0101,
	CURRENT_2_0_A  = 0b0110,
	CURRENT_2_25_A = 0b0111,
	CURRENT_2_50_A = 0b1000,
	CURRENT_2_75_A = 0b1001,
	CURRENT_3_0_A  = 0b1010,
	CURRENT_3_25_A = 0b1011,
	CURRENT_3_5_A  = 0b1100,
	CURRENT_4_0_A  = 0b1101,
	CURRENT_4_5_A  = 0b1110,
	CURRENT_5_0_A  = 0b1111	
} HUSB238_CurrentSetting;

/** Available voltages for PD output */
typedef enum _husb_voltages
{
	PD_UNATTACHED = 0b0000,
	PD_5V  = 0b0001,
	PD_9V  = 0b0010,
	PD_12V = 0b0011,
	PD_15V = 0b0100,
	PD_18V = 0b0101,
	PD_20V = 0b0110
} HUSB238_VoltageSetting;

/** Responses to PD query */
typedef enum _husb_response_codes
{
	RESP_NO_RESPONSE        = 0b000,	// No response
	RESP_SUCCESS            = 0b001,	// Success
	RESP_INVALID_CMD_OR_ARG = 0b011,	// Invalid command or argument
	RESP_CMD_NOT_SUPPORTED  = 0b100,	// Command not supported
	RESP_TRANSACTION_FAIL   = 0b101		// Transaction fail. No GoodCRC is received after sending
} HUSB238_ResponseCodes;

/** Default 5V output current */
typedef enum _husb_5v_current_contract
{
	CURRENT5V_DEFAULT = 0b00,
	CURRENT5V_1_5_A   = 0b01,
	CURRENT5V_2_4_A   = 0b10,
	CURRENT5V_3_A     = 0b11
} HUSB238_5VCurrentContract;

/** What voltage is selected for PD sink */
typedef enum _husb_pd_selection
{
	PD_NOT_SELECTED = 0b0000,
	PD_SRC_5V  = 0b0001,	
	PD_SRC_9V  = 0b0010,	
	PD_SRC_12V = 0b0011,
	PD_SRC_15V = 0b1000,
	PD_SRC_18V = 0b1001,
	PD_SRC_20V = 0b1010	
} HUSB238_PDSelection;


#ifdef _DEBUG
const char *HUSB238_responsecode_str(uint8_t pd1);
const char *HUSB238_pdvoltage_str(uint8_t pd0);
const char *HUSB238_current_str(uint8_t pdoval);
#endif
