


#define F_CPU 8000000UL

#include "lora.h"

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>

// Globals
volatile unsigned long sk;
char serial_buffer[10];
int e;
unsigned char spreading, bandwidth, coding, node_address;
unsigned char destination, type, source, length;
unsigned char number = 1;
unsigned char retry = 1;

unsigned char destination_r, source_r, length_r, number_r;
unsigned char payload_length;

unsigned char received_payload[20];
unsigned char received_data[10];


unsigned long theTime, theTime2, laiks;
unsigned char stat, i;


ISR(TIMER1_COMPA_vect)
{
	sk++;
}

unsigned long millis(void)
{
	return sk;
}

void TimerCounter1_Init(void)
{
	TCNT1 = 0;
	OCR1A = 8000;
	TCCR1B |= (1 << CS00) | (1 << WGM02);
	TIMSK1 |= (1 << OCIE0A);
	sei();
}

void SPI_Master_Init(void)
{
		DataDirSPI |= (1 << MOSI) | (1 << SCK);
		PortSPI |= (1 << SS);
		DataDirSPI |= (1 << SS);
		SPCR = 0x51; // SPI0 enable; Master select; oscilator divider 16
		//SPSR0 |= (1 << SPI2X); // atkomentejot, divas reizes atraks SCK
}

void writeRegister(unsigned int address, unsigned int data)
{
	PortSPI &= ~(1<<SS);
	address |= 0b10000000;
	SPDR =  address;
	while(!(SPSR & (1<<SPIF)));
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	PortSPI |= (1<<SS);
}

unsigned int readRegister(unsigned int address)
{
	unsigned int value = 0x00;
	PortSPI &= ~(1<<SS);
	address &= 0b01111111;
	SPDR = address;
	while(!(SPSR & (1<<SPIF)));
	SPDR = 0x00;
	while(!(SPSR & (1<<SPIF)));
	value = SPDR;
	PortSPI |= (1<<SS);
	return(value);
}

void serial_Init(unsigned int ubrr)
{
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<USBS0) | (3<<UCSZ00);
}

void serial_Transmit(unsigned char dati)
{
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = dati;
}

void serial_Println(unsigned char *String)
{
	while(*String > 0)
	{
		serial_Transmit(*(String++));
	}
	serial_Transmit(0x0D);
	serial_Transmit(0x0A); // 0x0A pec ASCII ir jauna rinda
}

void serial_Print(unsigned char *String)
{
	while(*String > 0)
	{
		serial_Transmit(*(String++));
	}
}

void serial_Println_num(int num)
{
	itoa(num, serial_buffer, 10);
	serial_Println(serial_buffer);
}

void serial_Flush(void)
{
	unsigned char dummy;
	while(UCSR0A & (1<<RXC0)) dummy = UDR0;
}

unsigned char bitRead(unsigned char val, unsigned char shift)
{
	if(((val << (7 - shift)) & 0xFF) >= 0x80) return 1;
	else return 0;
}

void sx1272_ON(void)
{
	DataDirSPI |= (1 << SS);
	PortSPI |= (1 << SS);
	_delay_ms(100);
	
	SPI_Master_Init();
	_delay_ms(100);
	
	unsigned int version = readRegister(REG_VERSION);
	if(version == 0x22) serial_Println("sx1272 detected, starting");
	else serial_Println("Problem with sx1272 module");
	
	sx1272_setMaxCurrent();
	
	e = sx1272_setLora();
	serial_Print("setLORA, status: ");
	itoa(e, serial_buffer, 10);
	serial_Println(serial_buffer);
}

void sx1272_setMaxCurrent(void)
{
	unsigned char st0;

	st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	
	writeRegister(REG_OCP, 0x1B);
	
	writeRegister(REG_OP_MODE, st0);
	serial_Println("Setting max current done");
}

uint8_t sx1272_setLora(void)
{
	uint8_t state = 2;
	unsigned char st0;
	
	do 
	{
		_delay_ms(200);
		writeRegister(REG_OP_MODE, FSK_SLEEP_MODE);
		writeRegister(REG_OP_MODE, LORA_SLEEP_MODE);
		writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
		_delay_ms(250);
		st0 = readRegister(REG_OP_MODE);
	} while (st0 != LORA_STANDBY_MODE);
	
	if(st0 == LORA_STANDBY_MODE) state = 0;
	else state = 1;
	
	return state;
}
uint8_t sx1272_setMode(uint8_t mode)
{
	int8_t state = 2;
	unsigned char st0;
	unsigned char config1 = 0x00;
	unsigned char config2 = 0x00;
	
	st0 = readRegister(REG_OP_MODE);
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	
	if(mode == 1)
	{
		bandwidth = BW_125;
		spreading = SF_12;
		coding = CR_5;
		
		e = sx1272_setCR(CR_5);
		serial_Print("setCR, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
		
		e = sx1272_setSF(SF_12);
		serial_Print("setSF, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
		
		e = sx1272_setBW(BW_125);
		serial_Print("setBW, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
	}
	
	else if(mode == 4)
	{
		bandwidth = BW_500;
		spreading = SF_12;
		coding = CR_5;
		
		e = sx1272_setCR(CR_5);
		serial_Print("setCR, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
		
		e = sx1272_setSF(SF_12);
		serial_Print("setSF, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
		
		e = sx1272_setBW(BW_500);
		serial_Print("setBW, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
	}
	
	else if(mode == 10)
	{
		bandwidth = BW_500;
		spreading = SF_7;
		coding = CR_5;
		
		e = sx1272_setCR(CR_5);
		serial_Print("setCR, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
		
		e = sx1272_setSF(SF_7);
		serial_Print("setSF, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
		
		e = sx1272_setBW(BW_500);
		serial_Print("setBW, status: ");
		itoa(e, serial_buffer, 10);
		serial_Println(serial_buffer);
	}
	
	else
	{
		state = 1;
		serial_Println("Incorrect mode!");
	}
	
	state = 1;
	config1 = readRegister(REG_MODEM_CONFIG1);
	_delay_ms(100);
	
	// Error check, 
	if(mode == 1)
	{
		if( ( (config1 & 0x38) == 0x08) && ( (config1 & 0xC0) == 0) )
		{
			state = 0;
		}
		else state = 1;
	}
	
	if(mode == 4)
	{
		if( ( (config1 & 0x38) == 0x08) && ( (config1 & 0xC0) == 0x80) )
		{
			state = 0;
		}
		else state = 1;
	}
	
	if(mode == 10)
	{
		if( ( (config1 & 0x38) == 0x08) && ( (config1 & 0xC0) == 0x80) )
		{
			state = 0;
		}
		else state = 1;
	}
	
	writeRegister(REG_OP_MODE, st0);
	_delay_ms(100);
	return state;
}

uint8_t sx1272_setCR(uint8_t cod)
{
	unsigned char st0;
	int8_t state = 2;
	unsigned char config1;
	
	st0 = readRegister(REG_OP_MODE);
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	config1 = readRegister(REG_MODEM_CONFIG1);
	
	if(cod == CR_5)
	{
		config1 = config1 & 0b11000111;
		config1 = config1 | 0b00001000;
	}
	
	writeRegister(REG_MODEM_CONFIG1, config1);
	_delay_ms(100);
	
	config1 = readRegister(REG_MODEM_CONFIG1);
	
	if(cod == CR_5)
	{
		if( (config1 & 0x38) == 0x08 ) state = 0;
		else state = 1;
	}
	
	else 
	{
		state = 1;
		serial_Println("Incorrect coding rate input");
	}
	
	writeRegister(REG_OP_MODE, st0);
	_delay_ms(100);
	return state;
}

uint8_t sx1272_setSF(uint8_t spr)
{
	unsigned char st0;
	int8_t state = 2;
	unsigned char config1;
	unsigned char config2;
	
	st0 = readRegister(REG_OP_MODE);
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	config2 = readRegister(REG_MODEM_CONFIG2);
	
	if(spr == SF_7)
	{
		config2 = config2 & 0b00001111;
		config2 = config2 | 0b01110000;
	}
	
	if(spr == SF_12)
	{
		config2 = config2 & 0b11001111;
		config2 = config2 | 0b11000000;
		
		// Low data rate optimization
		config1 = readRegister(REG_MODEM_CONFIG1);
		config1 = config1 | 0b00000001;
		writeRegister(REG_MODEM_CONFIG1, config1);
	}
	
	sx1272_setHeaderON();
	writeRegister(REG_DETECT_OPTIMIZE, 0x03);
	writeRegister(REG_DETECION_THRESHOLD, 0x0A);
	
	config2 = config2 | 0b00000100;
	writeRegister(REG_MODEM_CONFIG1, config1);
	writeRegister(REG_MODEM_CONFIG2, config2);
	_delay_ms(100);
	
	unsigned char configAgc;
	uint8_t theLDRBit;
	
	config1 = readRegister(REG_MODEM_CONFIG1);
	config2 = (readRegister(REG_MODEM_CONFIG2));
	
	configAgc = config2;
	theLDRBit = 0;
	
	if(spr == SF_7)
	{
		if( (config2 >> 4) == 0x07 && bitRead(configAgc, 2) == 1 )
		{
			state = 0;
		}
		else state = 1;
	}
	
	if(spr == SF_12)
	{
		if( (config2 >> 4) == 0x0C && bitRead(configAgc, 2) == 1 && bitRead(config1, theLDRBit) == 1)
		{
			state = 0;
		}
		else state = 1;
	}
	
	writeRegister(REG_OP_MODE, st0);
	_delay_ms(100);
	
	return state;
}

uint8_t sx1272_setBW(uint16_t band)
{
	unsigned char st0;
	int8_t state = 2;
	unsigned char config1;
	bandwidth = band;
	
	st0 = readRegister(REG_OP_MODE);
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	config1 = readRegister(REG_MODEM_CONFIG1);
	
	if(band == BW_125)
	{
		config1 = config1 & 0b00111111;
	}
	
	if(band == BW_500)
	{
		config1 = config1 & 0b10111111;
		config1 = config1 | 0b10000000;
	}
	
	writeRegister(REG_MODEM_CONFIG1, config1);
	_delay_ms(100);
	
	config1 = readRegister(REG_MODEM_CONFIG1);
	
	if(band == BW_125)
	{
		if( (config1 >> 6) == SX1272_BW_125 ) state = 0;
	}
	
	if(band == BW_500) 
	{
		if( (config1 >> 6) == SX1272_BW_500 ) state = 0;
	}
	
	writeRegister(REG_OP_MODE, st0);
	_delay_ms(100);
	
	return state;
}

uint8_t sx1272_setHeaderON(void)
{
	int8_t state = 2;
	unsigned char config1;
	
	config1 = readRegister(REG_MODEM_CONFIG1);
	config1 = config1 & 0b11111011;
	
	writeRegister(REG_MODEM_CONFIG1, config1);
	
	_delay_ms(100);
	
	
	config1 = readRegister(REG_MODEM_CONFIG1);
	if( (config1 & 0b00000100) == 0x00 ) state = 0;
	else state = 1;
	
	return state;
}

uint8_t sx1272_setChannel(uint32_t ch)
{
	unsigned char st0;
	int8_t state = 2;
	unsigned int freq3;
	unsigned int freq2;
	uint8_t freq1;
	uint32_t freq;
	
	st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	
	freq3 = ((ch >> 16) & 0x0FF);		// frequency channel MSB
    freq2 = ((ch >> 8) & 0x0FF);		// frequency channel MIB
    freq1 = (ch & 0xFF);				// frequency channel LSB
	
	writeRegister(REG_FRF_MSB, freq3);
    writeRegister(REG_FRF_MID, freq2);
    writeRegister(REG_FRF_LSB, freq1);
	_delay_ms(100);
	
	// storing MSB in freq channel value
    freq3 = (readRegister(REG_FRF_MSB));
    freq = (freq3 << 8) & 0xFFFFFF;

    // storing MID in freq channel value
    freq2 = (readRegister(REG_FRF_MID));
    freq = (freq << 8) + ((freq2 << 8) & 0xFFFFFF);

    // storing LSB in freq channel value
    freq = freq + ((readRegister(REG_FRF_LSB)) & 0xFFFFFF);
	
	if(freq == ch) state = 0;
	else state = 1;
	
	writeRegister(REG_OP_MODE, st0);
	_delay_ms(100);
	return state;
}

uint8_t sx1272_setCRC_ON(void)
{
	uint8_t state = 2;
	unsigned char config1;
	
	config1 = readRegister(REG_MODEM_CONFIG1);
	
	config1 = config1 | 0b00000010;
	writeRegister(REG_MODEM_CONFIG1, config1);
	_delay_ms(100);
	
	config1 = readRegister(REG_MODEM_CONFIG1);
	if( (config1 & 0b00000010) == 0x02 ) state = 0;
	else state = 1;
	
	return state;	
}

int8_t sx1272_setPower(char p)
{
	unsigned char st0;
	int8_t state = 2;
	unsigned char value = 0x00;
	unsigned char power = 0x00;
	unsigned char RegPaDacReg = 0x5A;
	
	st0 = readRegister(REG_OP_MODE);
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	
	if( (p == 'x') || (p == 'X') || (p == 'M') ) 
	{
		value = 0x0F;
		power = 0x0F;
	}
	if( p == 'L') 
	{
		power = 0x02;
		value = 0x02;
	}
	if( p == 'H') 
	{
		power = 0x07;
		value = 0x07;
	}
	if( p == 'x')
	{
		value = 0x0C;
		power = 0x0C;
		value = value | 0b10000000;
		power = power | 0b10000000;
	}
	
	if( p == 'X' )
	{
		value = value | 0b10000000;
		power = power | 0b10000000;
		writeRegister(RegPaDacReg, 0x84);
	}
	
	writeRegister(REG_PA_CONFIG, value);
	_delay_ms(100);
	
	value = readRegister(REG_PA_CONFIG);
	
	if(value == power) state = 0;
	else state = 1;
	
	writeRegister(REG_OP_MODE, st0);
	_delay_ms(100);
	return state;
}

uint8_t sx1272_setNodeAddress(uint8_t addr)
{
	unsigned char st0;
	unsigned char value;
	uint8_t state = 2;
	
	if(addr > 255)
	{
		serial_Println("Too large address number!");
	}
	else
	{
		node_address = addr;
		st0 = readRegister(REG_OP_MODE);
		
		writeRegister(REG_OP_MODE, LORA_STANDBY_FSK_REGS_MODE);
		
		writeRegister(REG_NODE_ADRS, addr);
		writeRegister(REG_BROADCAST_ADRS, 0x00);
		
		value = readRegister(REG_NODE_ADRS);
		writeRegister(REG_OP_MODE, st0);
		
		if(value == addr) state = 0;
		else state = 1;
	}
	
	return state;
}

uint8_t sx1272_sendPacketTimeout(uint8_t dest, uint8_t *payload, uint16_t wait)
{
	uint8_t state = 2;
	destination = dest;
	type = PKT_TYPE_DATA;
	
	state = sx1272_setPacket(dest, payload);
	if(state == 0) state = sx1272_sendWithTimeout(wait);
	
	return state;
}

uint8_t sx1272_setPacketLength(unsigned char length)
{
	unsigned char st0;
	unsigned char value = 0x00;
	int8_t state = 2;
	
	st0 = readRegister(REG_OP_MODE);
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	writeRegister(REG_PAYLOAD_LENGTH_LORA, length);
	
	value = readRegister(REG_PAYLOAD_LENGTH_LORA);
	
	if(length == value) state = 0;
	else state = 1;
	
	writeRegister(REG_OP_MODE, st0);
	_delay_ms(250);
	return state;
}

uint8_t sx1272_setPacket(uint8_t dest, char *payload)
{
	int8_t state = 2;
	unsigned char st0;
	destination = dest;
	
	st0 = readRegister(REG_OP_MODE);
	
	sx1272_clearFlags();
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	
	length = 4 + sizeof(payload);
	sx1272_setPacketLength(length);
	
	writeRegister(REG_FIFO_ADDR_PTR, 0x80);
	
	writeRegister(REG_FIFO, destination);
	writeRegister(REG_FIFO, type);
	writeRegister(REG_FIFO, source);
	writeRegister(REG_FIFO, number);
	for(i = 0; i < length; i++)
	{
		writeRegister(REG_FIFO, payload[i]);
	}
	state = 0;
	
	writeRegister(REG_OP_MODE, st0);
	return state;
}

void sx1272_clearFlags(void)
{
	unsigned char st0;
	
	st0 = readRegister(REG_OP_MODE);
	
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	writeRegister(REG_IRQ_FLAGS, 0xFF);
	writeRegister(REG_OP_MODE, st0);
}

uint8_t sx1272_sendWithTimeout(uint16_t wait)
{
	uint8_t state = 2;
	unsigned char value = 0x00;
	unsigned long previous;
	
	previous = millis();
	sx1272_clearFlags();
	
	writeRegister(REG_OP_MODE, LORA_TX_MODE);
	
	value = readRegister(REG_IRQ_FLAGS);
	
	while( (bitRead(value, 3) == 0) && (millis() - previous < wait))
	{
		value = readRegister(REG_IRQ_FLAGS);
		if(millis() < previous)
		{
			previous = millis();
		}
	}
	
	state = 1;
	
	if( bitRead(value, 3) == 1) state = 0;
	else state = 1;
	
	sx1272_clearFlags();
	return state;
}

uint8_t sx1272_receivePacketTimeout(uint16_t wait)
{
	uint8_t state = 2;
	if(sx1272_availableData(wait) == 0) 
	{
		sx1272_receive();
	}
	
	state = sx1272_getPacket(wait);
	
	return state;
}

uint8_t sx1272_availableData(uint16_t wait)
{
	unsigned char value;
	unsigned char header = 0;
	unsigned char forme = 0;
	unsigned long previous;
	
	previous = millis();
	value = readRegister(REG_IRQ_FLAGS);
	while( (bitRead(value, 4) == 0) && ((millis() - previous) < (unsigned long)wait) )
	{
		value = readRegister(REG_IRQ_FLAGS);
		if(millis() < previous)
		{
			previous = millis();
		}
	}
	
	if(bitRead(value, 4) == 1)
	{	
		while( (header == 0) && (millis() - previous < (unsigned long)wait) )
		{
			header = readRegister(REG_FIFO_RX_BYTE_ADDR);
			if(millis() < previous)
			{
				previous = millis();
			}
		}
		
		if(header != 0)
		{
			destination_r = readRegister(REG_FIFO);
		}
	}
	else forme = 0;

	if(destination_r == node_address)
	{
		forme = 1;
	}
	else forme = 0;
	
	return forme;
}

uint8_t sx1272_receive(void)
{	
	writeRegister(REG_PA_RAMP, 0x08);
	writeRegister(REG_LNA, LNA_MAX_GAIN);
	writeRegister(REG_FIFO_ADDR_PTR, 0x00);
	
	if(spreading == SF_12) writeRegister(REG_SYMB_TIMEOUT_LSB, 0x05);
	else writeRegister(REG_SYMB_TIMEOUT_LSB, 0x08);
	writeRegister(REG_FIFO_RX_BYTE_ADDR, 0x00);
	
	sx1272_setPacketLength(20);
	writeRegister(REG_OP_MODE, LORA_RX_MODE);
}

uint8_t sx1272_getPacket(uint16_t wait)
{
	unsigned char state = 2;
	unsigned char value = 0x00;
	unsigned long previous;
	unsigned char received = 0;
	
	previous = millis();
	
	value = readRegister(REG_IRQ_FLAGS);
	
	while( (bitRead(value, 6) == 0) && (millis() - previous < (unsigned long)wait) )
	{
		value = readRegister(REG_IRQ_FLAGS);
		if( millis() < previous )
		{
			previous = millis();
		}
	}
	
	if( (bitRead(value, 6) == 1) && (bitRead(value, 5) == 0) )
	{
		received = 1;
		state = 0;
	}
	
	if(received == 1)
	{
		writeRegister(REG_FIFO_ADDR_PTR, 0x00);
		destination_r = readRegister(REG_FIFO);
		source_r = readRegister(REG_FIFO);
		length_r = readRegister(REG_FIFO);
		number_r = readRegister(REG_FIFO);
		
		payload_length = readRegister(REG_RX_NB_BYTES);
		
		for(i = 0; i < 6; i++)
		{
			received_payload[i] = readRegister(REG_FIFO);
		}
		state = 0;
	}
	
	writeRegister(REG_FIFO_ADDR_PTR, 0x00);
	sx1272_clearFlags();
	
	return state;
}

int8_t sx1272_getRSSI(void)
{
	int rssi = 0;
	int rssi_mean = 0;
	int total = 5;
	
	for(i = 0; i < total; i++)
	{
		rssi = -(OFFSET_RSSI) + readRegister(REG_RSSI_VALUE_LORA);
		rssi_mean += rssi;
	}
	
	rssi_mean = rssi_mean / total;
	
	return rssi_mean;
}

int8_t sx1272_getSNR(void)
{
	unsigned char value;
	int snr;
	
	value = readRegister(REG_PKT_SNR_VALUE);
	{
		if(value & 0x80)
		{
			value = ((~value + 1) & 0xFF) >> 2;
			snr = -value;
		}
		else snr = (value & 0xFF) >> 2;
	}
	
	return snr;
}

double sx1272_getRSSIpacket(void)
{
	double RSSIpacket;
	int current_snr = 0;
	
	current_snr = sx1272_getSNR();
	RSSIpacket = readRegister(REG_PKT_RSSI_VALUE);
	
	if(current_snr < 0)
	{
		RSSIpacket = -(OFFSET_RSSI) + (double)RSSIpacket + (double)current_snr*0.25;
	}
	else
	{
		RSSIpacket = -(OFFSET_RSSI) + (double)RSSIpacket;
	}
	
	return RSSIpacket;
}

unsigned int ADCoutput(void)
{
	unsigned int ADCresult;
	ADCSRA |= (1 << ADSC);
	ADCSRA &= ~(1 << ADIF);
	while( (ADCSRA & 0x10) == 0)
	{}

	ADCresult |= (ADCL >> 6);
	ADCresult |= (ADCH << 2);
	
	return ADCresult;
}

void WDT_off(void)
{
	cli();
	asm volatile("wdr");
	/* Clear WDRF in MCUSR */
	MCUSR &= ~(1<<WDRF);
	/* Write logical one to WDCE and WDE */
	/* Keep old prescaler setting to prevent unintentional
	time-out */
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	/* Turn off WDT */
	WDTCSR = 0x00;
	sei();
}

void WDT_Prescaler_Change(void)
{
	cli();
	asm volatile("wdr");
	/* Start timed sequence */
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	/* Set new prescaler(time-out) value = 64K cycles (~0.5
	s) */
	WDTCSR = (1<<WDE) | (1<<WDP3);
	sei();
}

void sx1272_Sleep(void)
{
	writeRegister(REG_OP_MODE, LORA_STANDBY_MODE);
	writeRegister(REG_OP_MODE, LORA_SLEEP_MODE);
}

void txRxSwitchInit()
{
	DDRC |= (1 << TX_SWITCH) | (1 << RX_SWITCH);
}

void enableRXpath()
{
	PORTC &= ~(1 << TX_SWITCH);
	PORTC |= (1 << RX_SWITCH);
}

void enableTXpath()
{
	PORTC |= (1 << TX_SWITCH);
	PORTC &= ~(1 << RX_SWITCH);
}

void initButton()
{
	DDRB &= ~(1 << BTN);
	PORTB |= (1 << BTN);
}