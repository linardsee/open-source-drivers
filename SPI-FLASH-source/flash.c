#define "flash.h"


void _sendCmd(uint8 cmd)
{
  SPIM_1_WriteTxData(cmd);
  while( (SPIM_1_ReadTxStatus() & SPIM_1_STS_SPI_DONE) == 0){}
}

void _writeEnable()
{
  cs_Write(0);
  _sendCmd(WRITE_EN);
  cs_Write(1);
}

void _writeDisable()
{
  cs_Write(0);
  _sendCmd(WRITE_DIS);
  cs_Write(1);
}

uint8 _readStatusReg()
{
  uint8 reg = 0;

  cs_Write(0);
  _sendCmd(READ_STATUS_REG);
  _sendCmd(SHIFT_OUT);
  cs_Write(1);

  reg = SPIM_1_ReadRxData();

  return reg;
}

BOOL _checkWIP()
{
  if((_readStatusReg() & WIP_BIT)) return 1;
  else return 0;
}

BOOL _checkIfProtectedBitsSet()
{
  BOOL protected = 1;

  if(_readStatusReg() & 0x3C) protected = 1;
  else protected = 0;

  return protected;
}

BOOL _checkIfProtected(uint32 address)
{
  BOOL protected = 1;

  uint8 bpb = (_readStatusReg() & 0x3C) >> 2;

  switch bpb
  {
    case 0:
      protected = 0;
    break;
    case 1:
      if( (address >= 0x1F0000) && (address <= 0x1FFFFF) ) protected = 1;
      else protected = 0;
    break;
    case 2:
      if( (address >= 0x1E0000) && (address <= 0x1FFFFF) ) protected = 1;
      else protected = 0;
    break;
    case 3:
      if( (address >= 0x1C0000) && (address <= 0x1FFFFF) ) protected = 1;
      else protected = 0;
    break;
    case 4:
      if( (address >= 0x180000) && (address <= 0x1FFFFF) ) protected = 1;
      else protected = 0;
    break;
    case 5:
      if( (address >= 0x100000) && (address <= 0x1FFFFF) ) protected = 1;
      else protected = 0;
    break;
    case 6:
    case 9:
      protected = 1;
    break;
    case 10:
      if( (address >= 0x000000) && (address <= 0x0F0000) ) protected = 1;
      else protected = 0;
    break;
    case 11:
      if( (address >= 0x000000) && (address <= 0x170000) ) protected = 1;
      else protected = 0;
    break;
    case 12:
      if( (address >= 0x000000) && (address <= 0x1B0000) ) protected = 1;
      else protected = 0;
    break;
    case 13:
      if( (address >= 0x000000) && (address <= 0x1D0000) ) protected = 1;
      else protected = 0;
    break;
    case 14:
      if( (address >= 0x000000) && (address <= 0x1E0000) ) protected = 1;
      else protected = 0;
    break;
    case 15:
      protected = 1;
    break;

    default:
      return protected = 1;
    break;
  }

  return protected;
}

cmdStatusTypedef writeStatusReg(uint8 code)
{
  cmdStatusTypedef status = ERROR;

  if(code & READ_ONLY_BITS) return INVALID_INPUT;
  if(_checkWIP()) return BUSY;

  cs_Write(0);
  _sendCmd(WRITE_STATUS_REG);
  _sendCmd(code);
  cs_Write(1);

  if( (_readStatusReg() & RW_BITS) == code)
  {
    status = OK;
  }
  else
  {
    status = ERROR;
  }

  return status;
}

cmdStatusTypedef readData(uint32 address, uint8* buff, uint32 size)
{
  cmdStatusTypedef status = ERROR;
  uint8 addr1 = (address & 0xFF0000) >> 16;
  uint8 addr2 = (address & 0xFF00) >> 8;
  uint8 addr3 = address & 0xFF;

  if(_checkWIP()) return BUSY;

  cs_Write(0);
  _sendCmd(READ_DATA);
  _sendCmd(addr1);
  _sendCmd(addr2);
  _sendCmd(addr3);

  for(int i = 0; i < size; i++)
  {
    _sendCmd(SHIFT_OUT);
  }
  cs_Write(1);

  if(SPIM_1_GetRxBufferSize() != size) return ERROR;
  else status = OK;

  for(int i = 0; i < size; i++)
  {
    buff[i] = SPIM_1_ReadRxData();
  }

  return status;
}
/* FAST READ */
//
//
//
//

/* FAST READ DUAL OUTPUT */
//
//
//
//

cmdStatusTypedef pageProgram(uint32 address, uint8* buff, uint8 size)
{
  cmdStatusTypedef status = ERROR;

  if(size > 256) return INVALID_INPUT;
  if(_checkWIP()) return BUSY;
  if(_checkIfProtected(address)) return PROTECTED;

  uint8 addr1 = (address & 0xFF0000) >> 16;
  uint8 addr2 = (address & 0xFF00) >> 8;
  uint8 addr3 = address & 0xFF;

  _writeEnable();
  cs_Write(0);
  _sendCmd(PAGE_PROG);
  _sendCmd(addr1);
  _sendCmd(addr2);
  _sendCmd(addr3);
  for(int i = 0; i < size; i++)
  {
    _sendCmd(buff[i]);
  }
  cs_Write(1);

  status = OK;

  return status;
}

cmdStatusTypedef sectorErase(uint32 address)
{
  cmdStatusTypedef status = ERROR;

  if(_checkWIP()) return BUSY;
  if(_checkIfProtected(address)) return PROTECTED;

  uint8 addr1 = (address & 0xFF0000) >> 16;
  uint8 addr2 = (address & 0xFF00) >> 8;
  uint8 addr3 = address & 0xFF;

  _writeEnable();
  cs_Write(0);
  _sendCmd(SECTOR_ERASE);
  _sendCmd(addr1);
  _sendCmd(addr2);
  _sendCmd(addr3);
  cs_Write(1);

  status = OK;

  return status;
}

cmdStatusTypedef blockErase(uint32 address)
{
  cmdStatusTypedef status = ERROR;

  if(_checkWIP()) return BUSY;
  if(_checkIfProtected(address)) return PROTECTED;

  uint8 addr1 = (address & 0xFF0000) >> 16;
  uint8 addr2 = (address & 0xFF00) >> 8;
  uint8 addr3 = address & 0xFF;

  _writeEnable();
  cs_Write(0);
  _sendCmd(BLK_ERASE);
  _sendCmd(addr1);
  _sendCmd(addr2);
  _sendCmd(addr3);
  cs_Write(1);

  status = OK;

  return status;
}

cmdStatusTypedef chipErase()
{
  cmdStatusTypedef status = ERROR;

  if(_checkWIP()) return BUSY;
  if(_checkIfProtectedBitsSet()) return PROTECTED;

  _writeEnable();
  cs_Write(0);
  _sendCmd(CHIP_ERASE_1);
  cs_Write(1);

  status = OK;

  return status;
}

cmdStatusTypedef deepPowerDown()
{
  cmdStatusTypedef status = ERROR;

  if(_checkWIP()) return BUSY;

  _writeEnable();
  cs_Write(0);
  _sendCmd(PWR_DOWN);
  cs_Write(1);

  status = OK;

  return status;
}

cmdStatusTypedef releaseDeepPowerDown()
{
  cmdStatusTypedef status = ERROR;

  cs_Write(0);
  _sendCmd(REL_PWR_DOWN_DEV_ID);
  cs_Write(1);

  status = OK;

  return status;
}

uint8 releaseDeepPowerDownDevID()
{
  uint8 devId = 0;

  cs_Write(0);
  _sendCmd(REL_PWR_DOWN_DEV_ID);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  cs_Write(1);

  devId = SPIM_1_ReadRxData();

  return devId;
}

cmdStatusTypedef readMnfctDevId(uint8* mnfct, uint8* devId)
{
  cmdStatusTypedef status = ERROR;

  cs_Write(0);
  _sendCmd(MANF_DEV_ID);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  cs_Write(1);

  // first byte is not needed
  SPIM_1_ReadRxData();
  *mnfct = SPIM_1_ReadRxData();
  *devId = SPIM_1_ReadRxData();

  status = OK;

  return status;
}

cmdStatusTypedef readJEDECId(uint8* mnfct, uint8* type, uint8* capacity)
{
  cmdStatusTypedef status = ERROR;

  if(_checkWIP()) return BUSY;

  cs_Write(0);
  _sendCmd(MANF_DEV_ID);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  _sendCmd(SHIFT_OUT);
  cs_Write(1);

  *mnfct = SPIM_1_ReadRxData();
  *type = SPIM_1_ReadRxData();
  *capacity = SPIM_1_ReadRxData();

  status = OK;

  return status;
}
