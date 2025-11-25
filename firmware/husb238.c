/*******************************************************************************************************************//**
	AmiCA USB PD power supply
	\author Peter Mulholland
***********************************************************************************************************************/

#include <stdint.h>
#include "husb238.h"

#ifdef _DEBUG
const char *HUSB238_responsecode_str(uint8_t pd1)
{
	switch ((pd1 >> 3) & 0x07)
	{
		case RESP_NO_RESPONSE:			return "No Response";
		case RESP_SUCCESS:				return "Success";
		case RESP_INVALID_CMD_OR_ARG:	return "Invalid command or argument";
		case RESP_CMD_NOT_SUPPORTED:	return "Command not supported";
		case RESP_TRANSACTION_FAIL:		return "Transaction fail";
		default:						return "!!INVALID!!";
	}
}

const char *HUSB238_pdvoltage_str(uint8_t pd0)
{
	switch ((uint8_t)((pd0 >> 4) & 0x0f))
	{
		case PD_UNATTACHED:	return "Unattached";
		case PD_5V:			return "5V";
		case PD_9V:			return "9V";
		case PD_12V:		return "12V";
		case PD_15V:		return "15V";
		case PD_18V:		return "18V";
		case PD_20V:		return "20V";
		default:			return "!!INVALID!!";
	}

}

const char *HUSB238_current_str(uint8_t pdoval)
{
	if ((pdoval & 0x80) == 0)
		return "Unavailable";

	switch (pdoval & 0x0F)
	{
		case CURRENT_0_5_A:		return "0.5A";
		case CURRENT_0_7_A:		return "0.7A";
		case CURRENT_1_0_A:		return "1.0A";
		case CURRENT_1_25_A:	return "1.25A";
		case CURRENT_1_5_A:		return "1.5A";
		case CURRENT_1_75_A:	return "1.75A";
		case CURRENT_2_0_A:		return "2.0A";
		case CURRENT_2_25_A:	return "2.25A";
		case CURRENT_2_50_A:	return "2.50A";
		case CURRENT_2_75_A:	return "2.75A";
		case CURRENT_3_0_A:		return "3.0A";
		case CURRENT_3_25_A:	return "3.25A";
		case CURRENT_3_5_A:		return "3.5A";
		case CURRENT_4_0_A:		return "4.0A";
		case CURRENT_4_5_A:		return "4.5A";
		case CURRENT_5_0_A:		return "5.0A";
//		default:				return "!!INVALID!!";
	}
}

#endif
