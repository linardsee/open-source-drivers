#ifndef S25FL216K
#define S25FL216K

/* Command codes*/
#define   0x00    SHIFT_OUT
#define   0x06    WRITE_EN
#define   0x04    WRITE_DIS
#define   0x05    READ_STATUS_REG
#define   0x01    WRITE_STATUS_REG
#define   0x03    READ_DATA
#define   0x0B    FAST_READ
#define   0x3B    FAST_READ_DUAL_OUT
#define   0x02    PAGE_PROG
#define   0xD8    BLK_ERASE
#define   0x20    SECTOR_ERASE
#define   0xC7    CHIP_ERASE_1
#define   0x60    CHIP_ERASE_2
#define   0xB9    PWR_DOWN
#define   0xAB    REL_PWR_DOWN_DEV_ID
#define   0x90    MANF_DEV_ID
#define   0x9F    JEDEC_ID

/* Status register bits*/
#define   SRP_BIT   (1 << 8)
#define   REV_BIT   (1 << 7)
#define   BP3_BIT   (1 << 6)
#define   BP2_BIT   (1 << 5)
#define   BP1_BIT   (1 << 4)
#define   BP0_BIT   (1 << 3)
#define   WEL_BIT   (1 << 2)
#define   WIP_BIT    1

#define   READ_ONLY_BITS    0x43
#define   RW_BITS           0xBC

typedef BOOL uint8;

typedef enum cmdStatusTypedef
{
  OK,
  ERROR,
  INVALID_INPUT,
  BUSY,
  PROTECTED
};

/* Function prototypes */
cmdStatusTypedef writeStatusReg(uint8);
cmdStatusTypedef readData(uint32, uint8*, uint32);
cmdStatusTypedef pageProgram(uint32, uint8*, uint8);
cmdStatusTypedef sectorErase(uint32);
cmdStatusTypedef blockErase(uint32);
cmdStatusTypedef chipErase();
cmdStatusTypedef deepPowerDown();
cmdStatusTypedef releaseDeepPowerDown();
uint8 releaseDeepPowerDownDevID();
cmdStatusTypedef readMnfctDevId(uint8*, uint8*);
cmdStatusTypedef readJEDECId(uint8*, uint8*, uint8*);

#endif
