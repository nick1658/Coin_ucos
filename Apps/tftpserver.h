#ifndef __TFTP_H__
#define __TFTP_H__

#include "lwip/mem.h"
#include "lwip/udp.h"

/* TFTP opcodes as specified in RFC1350   */
typedef enum {
  TFTP_RRQ = 1,
  TFTP_WRQ = 2,
  TFTP_DATA = 3,
  TFTP_ACK = 4,
  TFTP_ERROR = 5
} tftp_opcode;


/* TFTP error codes as specified in RFC1350  */
typedef enum {
  TFTP_ERR_NOTDEFINED,
  TFTP_ERR_FILE_NOT_FOUND,
  TFTP_ERR_ACCESS_VIOLATION,
  TFTP_ERR_DISKFULL,
  TFTP_ERR_ILLEGALOP,
  TFTP_ERR_UKNOWN_TRANSFER_ID,
  TFTP_ERR_FILE_ALREADY_EXISTS,
  TFTP_ERR_NO_SUCH_USER,
} tftp_errorcode;



typedef struct TFTP_CMD
{
	char file_name[32];
	char mode[8];
	char op[16];
	char op_v[16];
}s_tftp_cmd;

void tftp_init(void);
int tftp_process_write(struct udp_pcb *upcb, struct ip_addr *to, int to_port, s_tftp_cmd *p_tftp_cmd);
int tftp_process_read(struct udp_pcb *upcb2, struct ip_addr *to, int to_port, char* FileName);

#endif /* __TFTP_H__ */
