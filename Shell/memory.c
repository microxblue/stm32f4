#include "common.h"

void write_mem (UINT32 addr, UINT32 value, int unit)
{
  volatile BYTE    *pb;
  volatile WORD    *pw;
  volatile DWORD   *pd;

  if      (unit==1) {
    pb = (volatile BYTE *)addr;
    *pb =(BYTE)value;
  } else if (unit==2) {
    pw = (volatile WORD *)(addr&0xFFFFFFFE);
    *pw =(WORD)value;
  } else if (unit==4) {
    pd = (volatile DWORD *)(addr&0xFFFFFFFC);
    *pd =(DWORD)value;
  }
  return;
}

void read_mem (UINT32 addr, UINT32 len, int unit)
{
  DWORD dat;
  volatile BYTE  *pb = 0;
  volatile WORD  *pw = 0;
  volatile DWORD *pd = 0;
  UINT32 addrstart,oldaddr,i,j;

  unit &= 0x0F;

  if      (unit==1) {
    pb = (volatile BYTE  *)addr;
    oldaddr=(UINT32)pb;
  } else if (unit==2) {
    pw = (volatile WORD  *)(addr&0xFFFFFFFE);
    oldaddr=(UINT32)pw;
  } else if (unit==4) {
    pd = (volatile DWORD *)(addr&0xFFFFFFFC);
    oldaddr=(UINT32)pd;
  } else return;

  addrstart = addr & 0xFFFFFFF0;
  printf ("%08X: ",addrstart);
  for (i=0; i<len; i+=0) {
    if (unit == 1) {  //Read Flash
      if ((addrstart+i)>=oldaddr) {
        dat = *pb++;
        printf ("%02X ", (BYTE)dat);
      } else  {
        printf ("   ");
        len+=1;
      }
      j=1;
    } else if (unit == 2) {
      if ((addrstart+i)>=oldaddr) {
        dat = *pw++;
        printf ("%04X ", (WORD)dat);
      }
      else {
        printf ("     ");
        len+=2;
      }
      j=2;
    } else {
      if ((addrstart+i)>=oldaddr) {
        dat = *pd++;
        printf ("%08X ", dat);
      } else {
        printf ("         ");
        len+=4;
      }
      j=4;
    }
    i+=j;
    addr+=j;
    if (!(i&0xf) && (i<len)) printf ("\n%08X: ",addrstart+i);
  }
  printf ("\n");
}

