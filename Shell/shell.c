#include "board.h"
#include "shell.h"
#include "usbapi.h"

BYTE        gCmdPos;
CHAR        gCmdLine[MAX_LINE_LEN];

extern void send_status_resp ();

void usb_download ()
{
  DWORD  addr;
  DWORD  len;
  BYTE   mode;
  BYTE   loop;
  BYTE   err;
  BYTE   plen;
  WORD   rxlen;
  BYTE  *buf;
  BYTE  *txbuf;


  mode = gCommonApi->params[1].param_l;
  printf ("Waiting for data from USB ...\n");

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
        switch (mode) {
        case 0:
          // Write SRAM
          if ((len & 0xFFFF) == 0) {
            printf ("Writing SRAM 0x%08X -- ", addr);
          }
          memcpy ((void *)addr, buf, plen);
          if ((len & 0xFFFF) == 0) {
            printf ("DONE\n");
          }
          addr += plen;
          len  += plen;
          break;

        case 1:
          // Write Flash
          break;

        case 2:
          // Write perf test
          break;

        default:
          break;
        }

      } else if (plen == CMD_PACKET_LEN) { // Command packet

        if (buf[1] == (BYTE)'A' ) { // Change address
          addr = *(DWORD *)(buf + 4);
          printf ("Setting address  0x%08X\n", addr);
        } else if (buf[1] == (BYTE)'S' ) { // Skip page
          addr += *(DWORD *)(buf + 4);
          len  += *(DWORD *)(buf + 4);
        } else if (buf[1] == (BYTE)'H') {  // Status query
          if (buf[3]) {
            send_status_resp ();
          }
        } else if (buf[1] == (BYTE)'D' ) { // Sent done
          plen = 0xF0;
        }
      }
    } else {
      if (mode == 3) {
        // Read perf test
        txbuf = usb_get_tx_buf(EP_MAX_SIZE);
        if (txbuf) {
          memset (txbuf, 0xaa, EP_MAX_SIZE);
          usb_add_tx_buf (EP_MAX_SIZE);
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

  if (len) {
    if (err)
      printf ("Write FAIL!\n");
    else
      printf ("Write 0x%08X bytes successfully!\n", len);
  }

}

void print_banner (void)
{
  // Main loop
  printf ("\n\n"
          "*******************************\n"
          "* Welcome to STM32F407 %3dMHz *\n"
          "*    Strong Shell "
#if RECOVERY
          "R"
#else
          " "
#endif
          "  V1.01    *\n"
          "*         Micro  Blue         *\n"
          "*******************************\n"
          "\n\n>", SystemCoreClock / 1000000);
}

void print_helper (void)
{
  printf ("RB    address [length]      ;Read  Memory BYTE\n"
          "WB    address  value        ;Write Memory BYTE\n"
          "RW    address [length]      ;Read  Memory WORD\n"
          "WW    address  value        ;Write Memory WORD\n"
          "RD    address [length]      ;Read  Memory DWORD\n"
          "WD    address  value        ;Write Memory DWORD\n"
          "DL    mode                  ;Download Data from USB\n"
          "FP                          ;Program IROM flash\n"
          "RT                          ;Reset system\n"
          );
}

void parse_cmdline (char *str)
{
  BYTE           argcnt;
  DWORD          addr;
  DWORD          len;
  DWORD          dat;
  WORD           cmd;
  BYTE           cmdstr[2];
  char          *cmd_next;
  API_MODENTRY   modentry;

  cmd_next = NULL;

  if (!str)
    str = gCmdLine;

  // ignore while space
  if (*str == '@' || *str == '!') {
    *str = ' ';
  }

  str = skipchar (str, ' ');

  if (*str == 0) {
    goto Quit;
  }

  if (*str == '?') {
    print_helper ();
    goto Quit;
  }

  gCommonApi->params[0].param_l = (DWORD)str;

  if (str[1] && ((str[2] == ' ') || !str[2])) {
    cmdstr[0] = tolower(str[0]);
    cmdstr[1] = tolower(str[1]);
    cmd = CMD_HASH(cmdstr[0], cmdstr[1]);
  } else {
    cmd = 0xFFFF;
  }

  cmd_next = findchar (str, ';');
  if (*cmd_next == ';') {
    *cmd_next++ = 0;
  } else {
    cmd_next = NULL;
  }

  argcnt = 1;
  do {
    str = findchar (str, ' ');
    str = skipchar (str, ' ');
    if (*str) {
      gCommonApi->params[argcnt].param_l = xtoi(str);
      argcnt++;
    }
  } while ((*str) && (argcnt < 4));

  for (dat = argcnt; dat < 4; dat++)
      gCommonApi->params[dat].param_l = 0;

  argcnt--;
  gCommonApi->paramcnt = argcnt;

  addr = gCommonApi->params[1].param_l;
  len  = gCommonApi->params[2].param_l;
  dat  = gCommonApi->params[3].param_l;

  if (argcnt < 2) {
     len = 16;
  }

  switch(cmd) {

    case (CMD_HASH('r' , 'b')): //"rb"
      read_mem (addr, len, 1);
      break;
    case (CMD_HASH('r' , 'w')): //"rw"
      read_mem (addr, len, 2);
      break;
    case (CMD_HASH('r' , 'd')): //"rd"
      read_mem (addr, len, 4);
      break;
    case (CMD_HASH('w' , 'b')): //"wb"
      write_mem (addr, len, 1);
      break;
    case (CMD_HASH('w' , 'w')): //"ww"
      write_mem (addr, len, 2);
      break;
    case (CMD_HASH('w' , 'd')): //"wd"
      write_mem (addr, len, 4);
      break;
    case (CMD_HASH('f' , 'p')): //"fp"
      irom_prog();
      break;
    case (CMD_HASH('t' , 't')): //"rt"
      break;
    case (CMD_HASH('r' , 't')): //"rt"
      if (argcnt > 0) {
        *(UINT8 *)USER_REG_BASE = (UINT8)addr;
      }
      NVIC_SystemReset ();
      break;
    case (CMD_HASH('d' , 'l')): //"dl"
      usb_download();
      break;
    case (CMD_HASH('g' , 'o')): //"go"
      if (argcnt == 1) {
        modentry = (API_MODENTRY)addr;
        modentry ();
      }
      break;
      break;
    default:
      cmd = 0xFFFF;
      break;
  }

  if (cmd == 0xFFFF) {
    // extended command
    printf ("Unknown command!\n", len);
    goto Quit;
  }

Quit:
  printf ("%s", cmd_next ? "\n" : "\n>");

  if (cmd_next) {
    parse_cmdline (cmd_next);
  }

  return;
}


BYTE get_ser_cmdline (char *cmdline)
{
  char          ch;
  BYTE          result;

  if (!haschar())
    return 0;

  if (!cmdline)
    cmdline = gCmdLine;

  result = 0;
  ch = (int)getchar();
  if (ch == 27) {
    ch = (int)getchar();
    if (ch=='[') {
      ch = (int)getchar();
      if (ch>='A' && ch<='D') ch=0x09;
      else ch=0x80;
    }
  }

  switch (ch) {
  	case 0x09:    /* Tab */
  	  if (!gCmdPos) {
  	  	printf("%s\n", cmdline);
  	  	result = findchar(cmdline, 0) - cmdline;
  	  }
  	  break;
    case 0x08:    /* Backspace */
    case 0x7F:    /* Delete */
      if (gCmdPos > 0) {
        gCmdPos -= 1;
        putchar(0x08);  /* backspace */
        putchar(' ');
        putchar(0x08);  /* backspace */
      }
      break;
    case 0x0D:
    case 0x0A:
      putchar(0x0D);  /* CR */
      putchar(0x0A);  /* CR */
      cmdline[gCmdPos] = 0;
      result = gCmdPos + 1;
      gCmdPos = 0;
      break;
    default:
      if ((gCmdPos+1) < MAX_LINE_LEN) {
        /* only printable characters */
        if (ch > 0x1f) {
          cmdline[gCmdPos++] = (char)ch;
          putchar((char)ch);
        }
      }
      break;
  }
  return result;
}

BYTE get_mem_cmdline (char *cmdline)
{
  BYTE          result;
  BYTE         *ptr;

  result = 0;

  if (!cmdline) {
    cmdline = gCmdLine;
  }

  ptr = (BYTE *)SHELL_CMD_BASE;
  if (ptr[0] == '@') {
    memcpy (cmdline, (void *)SHELL_CMD_BASE, MAX_LINE_LEN);
    // printf ("%s\n", cmdline);
    memset (ptr, 0, MAX_LINE_LEN);
    result = findchar(cmdline, 0) - cmdline;
  } else {
    result = 0;
  }

  return result;
}

BYTE get_usb_cmdline (char *cmdline)
{
  BYTE          result;
  WORD          len;
  BYTE         *buf;

  result = 0;

  if (!cmdline)
    cmdline = gCmdLine;

  buf = usb_get_rx_buf(&len);
  if (buf) {
    if (len == MAX_LINE_LEN) {
      // It is a command string
      memcpy (cmdline, buf, MAX_LINE_LEN);
      result = MAX_LINE_LEN;
    } else if (len == CMD_PACKET_LEN) {
      if (buf[1] == 'H') {
        if (buf[3]) {
          send_status_resp ();
        }
      }
      result = 0;
    }
    usb_free_rx_buf();
  }

  if (result) {
    if (cmdline[0] != '!')  printf("%s\n", cmdline);
  }

  return result;
}
