/*-----------------------------------------------------------------------------
 *      Name:    RETARGET.C 
 *      Purpose: Retarget low level functions
 *-----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <rt_sys.h>
#include "S3C2416.h"

/*
 * Disable semihosting apis
 */
#pragma import(__use_no_semihosting_swi)

/* Standard IO device handles. */
#define STDIN   0x1
#define STDOUT  0x2
#define STDERR  0x3

/* Standard IO device name defines. */
const char __stdin_name[]  = "STDIN";
const char __stdout_name[] = "STDOUT";
const char __stderr_name[] = "STDERR";

struct __FILE { int handle; /* Add whatever you need here */ };

/*-----------------------------------------------------------------------------
  Write character to the Serial Port
 *----------------------------------------------------------------------------*/
int sendchar (int c) {
	if (c == '\n')  {
		Uart0_SendByte('\r');
	}
	Uart0_SendByte(c);
	return (c);
}

/*-----------------------------------------------------------------------------
  Read character from the Serial Port
 *----------------------------------------------------------------------------*/
int getkey (void) {
	int ch = 'A';//Uart_WaitChar();

	if (ch < 0) {
		return 0;
	}
	return ch;
}

/*--------------------------- _ttywrch ---------------------------------------*/
void _ttywrch (int ch) {
	sendchar(ch);
}

/*--------------------------- _sys_open --------------------------------------*/
FILEHANDLE _sys_open (const char *name, int openmode) {
	/* Register standard Input Output devices. */
	if (strcmp(name, "STDIN") == 0) {
	return (STDIN);
	} else if (strcmp(name, "STDOUT") == 0) {
		return (STDOUT);
	} else if (strcmp(name, "STDERR") == 0) {
		return (STDERR);
	}	

	return -1;	  
}

/*--------------------------- _sys_close -------------------------------------*/
int _sys_close (FILEHANDLE fh) {
	if (fh > 0 && fh < 4) {
		return (0);
	}
    return (-1);
}

/*--------------------------- _sys_write -------------------------------------*/
int _sys_write (FILEHANDLE fh, const uint8_t *buf, uint32_t len, int32_t mode) {
  if (fh > 0 && fh < 4) { 
	if (fh == STDOUT) {
		/* Standard Output device. */
		for (  ; len; len--) {
			sendchar (*buf++);
		}
		return (0);
	} else {
		return (-1);
	}
  }
 
  return -1;
}

/*--------------------------- _sys_read --------------------------------------*/
int _sys_read (FILEHANDLE fh, uint8_t *buf, uint32_t len, int32_t mode) {
  if (fh > 0 && fh < 4) { 	
	if (fh == STDIN) {
		/* Standard Input device. */
		for (  ; len; len--) {
			*buf++ = getkey ();
		}
		return (0);
	} else {
		return (-1);
	}
  }
  return (-1);
}

/*--------------------------- _sys_istty -------------------------------------*/
int _sys_istty (FILEHANDLE fh) {
  if (fh > 0 && fh < 4) {
    return (1);
  }
  return (0);
}

/*--------------------------- _sys_seek --------------------------------------*/
int _sys_seek (FILEHANDLE fh, long pos) {
  return (-1);
}

/*--------------------------- _sys_ensure ------------------------------------*/
int _sys_ensure (FILEHANDLE fh) {
  return -1;
}

/*--------------------------- _sys_flen --------------------------------------*/
long _sys_flen (FILEHANDLE fh) {
    return (0);
}

/*--------------------------- _sys_tmpnam ------------------------------------*/
int _sys_tmpnam (char *name, int sig, unsigned maxlen) {
  return (1);
}

/*--------------------------- _sys_command_string ----------------------------*/
char *_sys_command_string (char *cmd, int len) {
  return (cmd);
}

/*--------------------------- _sys_exit --------------------------------------*/
void _sys_exit (int return_code) {
  /* Endless loop. */
  while (1);
}

/*-----------------------------------------------------------------------------
 * end of file
 *----------------------------------------------------------------------------*/

