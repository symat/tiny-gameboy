// minimal helper functions to read/write EEPROM

// ATTiny85 has 512 bytes EEPROM, with these functions one can 
// address the first 256 bytes

#ifndef __TINY_GAMEBOY_EEPROM_UTILS
#define __TINY_GAMEBOY_EEPROM_UTILS

uint8_t EEPROM_read(uint8_t address) {
  /* Wait for completion of previous write */
  while(EECR & (1<<EEPE))   ;
  /* Set up address register */
  EEAR = address;
  /* Start eeprom read by writing EERE */
  EECR |= (1<<EERE);
  /* Return data from data register */ 
  return EEDR;
}



void EEPROM_update(uint8_t address, uint8_t data) {
  if(EEPROM_read(address) == data) {
    return;
  }
  
  /* Wait for completion of previous write */ 
  while(EECR & (1<<EEPE));

  /* Set Programming mode */
  EECR = (0<<EEPM1)|(0<<EEPM0);

  /* Set up address and data registers */ 
  EEAR = address;
  EEDR = data;

  /* Write logical one to EEMPE */
  EECR |= (1<<EEMPE);
  
  /* Start eeprom write by setting EEPE */ 
  EECR |= (1<<EEPE);
}

#endif // __TINY_GAMEBOY_EEPROM_UTILS
