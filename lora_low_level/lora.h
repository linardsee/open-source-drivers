
#include <avr/io.h>


// sx1272 REGISTERS
#define 		REG_VERSION					0x42
#define			REG_OP_MODE					0x01
#define			REG_OCP						0x0B
#define			REG_MODEM_CONFIG1			0x1D
#define			REG_MODEM_CONFIG2			0x1E
#define			REG_DETECT_OPTIMIZE			0x31
#define			REG_DETECION_THRESHOLD		0x37
#define        	REG_FRF_MSB    				0x06
#define        	REG_FRF_MID    				0x07
#define        	REG_FRF_LSB    				0x08
#define        	REG_PA_CONFIG    			0x09
#define       	REG_NODE_ADRS	  			0x33
#define        	REG_BROADCAST_ADRS	 	 	0x34
#define        	REG_FIFO_ADDR_PTR  			0x0D
#define         REG_FIFO        			0x00
#define        	REG_IRQ_FLAGS	    		0x12
#define        	REG_PAYLOAD_LENGTH_LORA		0x22
#define        	REG_PA_RAMP    				0x0A
#define       	REG_LNA    					0x0C
#define        	REG_SYMB_TIMEOUT_LSB  		0x1F
#define        	REG_RX_NB_BYTES	    		0x13
#define        	REG_FIFO_RX_BYTE_ADDR 		0x25
#define        	REG_RSSI_VALUE_LORA	  		0x1B
#define         REG_PKT_SNR_VALUE	  		0x19
#define        	REG_PKT_RSSI_VALUE	  		0x1A
#define 		LNA_MAX_GAIN                0x23

// UART
#define BAUD 38400
#define MYUBRR F_CPU/(16L*BAUD)-1

// SPI
#define DataDirSPI	DDRB
#define PortSPI		PORTB
#define SS 			2
#define MOSI		3
#define MISO		4
#define SCK			5

// TX and RX switch GPIO
#define TX_SWITCH	0
#define RX_SWITCH	1
#define BTN			1

// Lora modes
#define FSK_SLEEP_MODE 0x00
#define LORA_SLEEP_MODE 0x80
#define LORA_STANDBY_MODE 0x81
#define LORA_TX_MODE 0x83
#define LORA_RX_MODE 0x85
#define LORA_STANDBY_FSK_REGS_MODE 0xC1

// Lora bandwidth
#define SX1272_BW_125 0x00
#define SX1272_BW_500 0x02

// Lora bandwidth
#define BW_125 0x07
#define BW_500 0x09

// Lora coding rate
#define CR_5 0x01

// Lora spreading factor
#define SF_7 0x06
#define SF_12 0x0C

// Additional constants
#define OFFSET_RSSI 137

// Lora channels
#define CH_10_868 0xD84CCC // channel 10, central freq = 865.20MHz
#define CH_11_868 0xD86000 // channel 11, central freq = 865.50MHz
#define CH_12_868 0xD87333 // channel 12, central freq = 865.80MHz
#define CH_13_868 0xD88666 // channel 13, central freq = 866.10MHz
#define CH_14_868 0xD89999 // channel 14, central freq = 866.40MHz
#define CH_15_868 0xD8ACCC // channel 15, central freq = 866.70MHz
#define CH_16_868 0xD8C000 // channel 16, central freq = 867.00MHz
#define CH_17_868 0xD90000 // channel 17, central freq = 868.00MHz

#define CH_00_900 0xE1C51E // channel 00, central freq = 903.08MHz
#define CH_01_900 0xE24F5C // channel 01, central freq = 905.24MHz
#define CH_02_900 0xE2D999 // channel 02, central freq = 907.40MHz
#define CH_03_900 0xE363D7 // channel 03, central freq = 909.56MHz
#define CH_04_900 0xE3EE14 // channel 04, central freq = 911.72MHz
#define CH_05_900 0xE47851 // channel 05, central freq = 913.88MHz
#define CH_06_900 0xE5028F // channel 06, central freq = 916.04MHz
#define CH_07_900 0xE58CCC // channel 07, central freq = 918.20MHz
#define CH_08_900 0xE6170A // channel 08, central freq = 920.36MHz
#define CH_09_900 0xE6A147 // channel 09, central freq = 922.52MHz
#define CH_10_900 0xE72B85 // channel 10, central freq = 924.68MHz
#define CH_11_900 0xE7B5C2 // channel 11, central freq = 926.84MHz
#define CH_12_900 0xE4C000 // default channel 915MHz, the module is configured with it

// Packet types
#define PKT_TYPE_DATA   0x10
#define PKT_TYPE_ACK    0x20

// SPI
void SPI_Master_Init(void);
void writeRegister(unsigned int address, unsigned int data);
unsigned int readRegister(unsigned int address);

// UART
void serial_Init(unsigned int ubrr);
void serial_Transmit(unsigned char data);
void serial_Println(unsigned char *String);
void serial_Print(unsigned char *String);
void serial_Println_num(int num);
void serial_Flush(void);

// Timer/Counter 
void TimerCounter1_Init(void);
unsigned long millis(void);

// Miscellaneous
unsigned char bitRead(unsigned char val, unsigned char shift);

// SX1272
void sx1272_ON(void);
void sx1272_setMaxCurrent(void);
uint8_t sx1272_setLora(void);

uint8_t sx1272_setMode(unsigned char mode);
uint8_t sx1272_setCR(uint8_t cod);
uint8_t sx1272_setBW(uint16_t band);
uint8_t sx1272_setSF(uint8_t spr);

uint8_t sx1272_setHeaderON(void);

uint8_t sx1272_setChannel(uint32_t ch);

uint8_t sx1272_setCRC_ON(void);

int8_t sx1272_setPower(char p);

uint8_t sx1272_setNodeAddress(uint8_t addr);

uint8_t sx1272_sendPacketTimeout(uint8_t dest, uint8_t *payload, uint16_t wait);
uint8_t sx1272_setPacketLength(unsigned char length);
uint8_t sx1272_setPacket(uint8_t dest, char *payload);
void sx1272_clearFlags(void);
uint8_t sx1272_sendWithTimeout(uint16_t wait);

uint8_t sx1272_receivePacketTimeout(uint16_t wait);
uint8_t sx1272_availableData(uint16_t wait);
uint8_t sx1272_receive(void);
uint8_t sx1272_getPacket(uint16_t wait);

int8_t sx1272_getRSSI(void);
int8_t sx1272_getSNR(void);
double sx1272_getRSSIpacket(void);

void WDT_off(void);
void WDT_Prescaler_Change(void);

void sx1272_Sleep(void);

void initButton(void);