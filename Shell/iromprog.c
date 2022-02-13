#include "board.h"
#include "usbapi.h"

#define IROM_BASE  0x08000000
#define IROM_END   0x08100000

#define  MCU_TYPE_STM32F407  1

void send_status_resp ()
{
  BYTE  *buf;

  // Request a echo packet back
  do {
    buf = usb_get_tx_buf(EP_MAX_SIZE);
  } while (!buf);
  // fill data to buffer
  memset (buf, 0, EP_MAX_SIZE);
  *(DWORD *)buf = 0x53545343; // CSTS
  buf[4] = EP_MAX_SIZE;
  buf[7] = MCU_TYPE_STM32F407;
  memcpy (buf + 16, gCommonApi->status, 8);
  usb_add_tx_buf (EP_MAX_SIZE);
}

uint32_t flash_sector_size (uint8_t sector)
{
  if (sector >= 0 && sector <= 3)
      return 16 * 1024;
  else if (sector == 4)
      return 64 * 1024;
  else if (sector >= 5 && sector <= 11)
      return 128 * 1024;
  return 0;
}

uint32_t flash_sector_start (uint8_t sector)
{
  uint32_t address = IROM_BASE;
  while (sector > 0) {
      --sector;
      address += flash_sector_size(sector);
  }
  return address;
}

uint8_t flash_sector_at (uint32_t address)
{
  uint8_t sector = 0;

  if ((address < IROM_BASE) || (address >= IROM_END)) {
    return 0xFF;
  }

  while (address >= flash_sector_start (sector + 1))  sector++;
  return sector;
}

void irom_prog (void)
{
  DWORD  addr;
  DWORD  len;
  DWORD  baddr;
  BYTE   loop;
  BYTE   idx;
  BYTE   err;
  BYTE   dat;
  BYTE   plen;
  WORD   rxlen;
  BYTE  *buf;
  BYTE  *txbuf;
  WORD   sector;

  extern char _flash_start[];
  extern char _flash_end[];

  printf ("Waiting for data from USB ...\n");

  // Program Flash
  FLASH_Unlock ();
  FLASH_ClearFlag  (FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

  addr   = 0;
  len    = 0;
  err    = 0;
  loop   = 1;

  while (loop) {
    plen = 0xFF;
    buf = usb_get_rx_buf(&rxlen);
    if (buf) {
      plen = (BYTE)rxlen;
      if (plen == EP_MAX_SIZE) { // Data packet
        // Write Flash 64 bytes
        if ((len & 0xFFF) == 0) {
          printf ("Writing flash 0x%08X - ", addr);
        }
        for (idx = 0; idx < EP_MAX_SIZE; idx += sizeof(DWORD)) {
          if ((addr >= (DWORD)_flash_start) && (addr <= (DWORD)_flash_end)) {
            dat = 2;
          } else {
            if (FLASH_ProgramWord(addr, *(DWORD *)buf) == FLASH_COMPLETE)
              dat = 0;
            else
              dat = 1;
          }
          addr += sizeof(DWORD);
          buf  += sizeof(DWORD);
        }

        if ((len & 0xFFF) == 0) {
          if (dat==1) {
            printf ("FAIL\n");
          } else if (dat==2) {
            printf ("UNSUPPORTED\n");
          } else {
            printf ("DONE\n");
          }
        }

        err  += dat;
        len  += EP_MAX_SIZE;

      } else if (plen == CMD_PACKET_LEN) { // Command packet
        if (buf[1] == (BYTE)'A' ) { // Change address
          addr = *(DWORD *)(buf + 4);
          printf ("Setting address  0x%08X\n", addr);
        } else if (buf[1] == (BYTE)'S' ) { // Skip page
          addr += *(DWORD *)(buf + 4);
          len  += *(DWORD *)(buf + 4);
        } else if (buf[1] == (BYTE)'E' ) { // Erase block
          baddr  = *(DWORD *)&buf[4];
          sector = flash_sector_at (baddr);
          printf ("Erasing block 0x%08X (SEC %d) - ", baddr, sector);
          if (sector == 0xFF) {
            printf ("UNSUPPORTED\n");
          } else {
            if ((flash_sector_start (sector) >= (DWORD)_flash_start) && (flash_sector_start (sector+1) <= (DWORD)_flash_start + SHELL_SIZE)) {
              printf ("UNSUPPORTED\n");
            } else {
              if (flash_sector_start (sector) == baddr) {
                sector = sector << 3;
                if (FLASH_EraseSector(sector, VoltageRange_3) != FLASH_COMPLETE) {
                  printf ("FAIL\n");
                } else {
                  printf ("DONE\n");
                }
              } else {
                printf ("SKIP\n");
              }
            }
          }
        } else if (buf[1] == (BYTE)'R' ) { // Read data
            addr  = *(DWORD *)(buf + 4);
            baddr = addr + *(DWORD *)(buf + 8);
            printf ("Reading block 0x%08X - ", addr);

            for (; addr < baddr; addr += EP_MAX_SIZE) {
              do {
                txbuf = usb_get_tx_buf(EP_MAX_SIZE);
              } while (!txbuf);
              if (txbuf) {
                dat = EP_MAX_SIZE;
                for (dat = 0; dat < EP_MAX_SIZE; dat++)
                  *txbuf++ = *(BYTE *)(addr+dat);
                usb_add_tx_buf (EP_MAX_SIZE);
              }
            }
            printf ("DONE\n");
        } else if (buf[1] == (BYTE)'H') {
          if (buf[3]) {
            send_status_resp ();
          }
        } else if (buf[1] == (BYTE)'D' ) { // Sent done
          plen = 0xF0;
        }
      }
    }

    if ((plen == 0xF0) || haschar()) {
      if (haschar()) {
        getchar();
      }
      loop = 0;
    }

    if (plen != 0xFF) {
      usb_free_rx_buf();
    }
  }

  FLASH_Lock ();

  if (len) {
    if (err)
      printf ("Write FAIL!\n");
    else
      printf ("Write 0x%08X bytes successfully!\n", len);
  }
}
