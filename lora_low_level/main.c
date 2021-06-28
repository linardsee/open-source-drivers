/*
 * lora_transceiver_rev1.c
 *
 * Created: 18.05.2019 00:01:06
 * Author : Linards
 */


#include "lora.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>


int main(void)
{
	_delay_ms(3000);
	serial_Init(MYUBRR);
	TimerCounter1_Init();

	serial_Println("Initialization begins");
	sx1272_ON();

	e = sx1272_setMode(10);
	serial_Print("setMode, status: ");
	serial_Println_num(e);

	e = sx1272_setHeaderON();
	serial_Print("setHeaderON, status: ");
	serial_Println_num(e);

	// For USA
	//e = sx1272_setChannel(CH_12_900);
	e = sx1272_setChannel(CH_10_868);
	serial_Print("setChannel, status: ");
	serial_Println_num(e);

	e = sx1272_setCRC_ON();
	serial_Print("setCRC_ON, status: ");
	serial_Println_num(e);

	e = sx1272_setPower('X');
	serial_Print("setPower, status: ");
	serial_Println_num(e);

	e = sx1272_setNodeAddress(10);
	serial_Print("setNodeAddress, status: ");
	serial_Println_num(e);

	// Configure as receiver
	txRxSwitchInit();
	enableRXpath();

	serial_Print("Receiver has been configured!");
	serial_Println_num(e);

    while (1)
    {
		if(bit_is_clear(PINB, BTN))
		{
			e = sx1272_sendPacketTimeout(10, pakete1, 1000);
			serial_Print("Request Sent: ");
			serial_Println_num(e);
		}

		e = sx1272_receivePacketTimeout(1000);
		if(e == 0)
		{
			for(i = 0; i < 6; i++)
			{
				received_data[i] = received_payload[i];
			}
			serial_Print("Packet receive, status: ");
			itoa(e, serial_buffer, 10);
			serial_Println(serial_buffer);

			serial_Println(received_data);
			serial_Flush();
			PORTB |= (1 << PINB0);
			_delay_ms(300);

			//Atkomentejot visu, var redzet signala stiprumu (RSSI) un signala - troksna attiecibu (SNR)
			serial_Print("RSSIpacket: ");
			itoa(sx1272_getRSSIpacket(), serial_buffer, 10);
			serial_Println(serial_buffer);

			serial_Print("SNR: ");
			itoa(sx1272_getSNR(), serial_buffer, 10);
			serial_Println(serial_buffer);
		}
    }
}
