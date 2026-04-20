#ifndef FMC_H_
#define FMC_H_

/**
  * @brief     Erase a page. The page size is 512 bytes.
  * @param addr   Flash page address. Must be a 512-byte aligned address.
  * @retval    void
  */
void hal_fmc_erase_page(uint32_t addr);

/**
  * @brief     Writes a word data to specified flash address.
  * @param addr  Destination address, Must be a 4-byte aligned address.
  * @param data  Word data to be written
  * @retval   void
  */
void hal_fmc_write_word(uint32_t addr, uint32_t data);

#endif /* FMC_H_ */
