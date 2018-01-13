/* tftputils.c */

#include <string.h>
#include "lwip/inet.h"
#include "tftputils.h"


/* Extract the opcode from a TFTP message in a buffer */
tftp_opcode tftp_decode_op(char *buf)
{
  return (tftp_opcode)(buf[1]);
}

/* Extract the block number from a TFTP message in a buffer */
u16_t tftp_extract_block(char *buf)
{
  u16_t *b = (u16_t*)buf;
  return ntohs(b[1]);
}

/* Extract the block number from a TFTP message in a buffer */
void tftp_extract_filename(s_tftp_cmd *p_tftp_cmd, char *buf, int buf_len)
{
	int i = 2, j = 0;
	for (; i < buf_len; i++){//file name
		if (buf[i] == 0){
			i++;
			p_tftp_cmd->file_name[j] = 0;
			j = 0;
			break;
		}else{
			p_tftp_cmd->file_name[j++] = buf[i];
		}
	}
	for (; i < buf_len; i++){//mode
		if (buf[i] == 0){
			i++;
			p_tftp_cmd->mode[j] = 0;
			j = 0;
			break;
		}else{
			p_tftp_cmd->mode[j++] = buf[i];
		}
	}
	for (; i < buf_len; i++){//op
		if (buf[i] == 0){
			i++;
			p_tftp_cmd->op[j] = 0;
			j = 0;
			break;
		}else{
			p_tftp_cmd->op[j++] = buf[i];
		}
	}
	for (; i < buf_len; i++){//op_v
		if (buf[i] == 0){
			i++;
			p_tftp_cmd->op_v[j] = 0;
			j = 0;
			break;
		}else{
			p_tftp_cmd->op_v[j++] = buf[i];
		}
	}
	//strcpy(fname, buf + 2);
}

/* Set the opcode in the 2 first bytes for RRQ / WRQ / DATA / ACK / ERROR */
void tftp_set_opcode(char *buffer, tftp_opcode opcode)
{

  buffer[0] = 0;
  buffer[1] = (u8_t)opcode;
}

/* Set the errorcode in the 2 second bytes for ERROR */
void tftp_set_errorcode(char *buffer, tftp_errorcode errCode)
{

  buffer[2] = 0;
  buffer[3] = (u8_t)errCode;
}

/* Set the error message in the last appropriate bytes for ERROR */
void tftp_set_errormsg(char * buffer, char* errormsg)
{
  strcpy(buffer + 4, errormsg);
}

/* Set the block number in the 2 second bytes ACK / DATA */
void tftp_set_block(char* packet, u16_t block)
{

  u16_t *p = (u16_t *)packet;
  p[1] = htons(block);
}

/* Set the message int the n last bytes for DATA */
void tftp_set_data_message(char* packet, char* buf, int buflen)
{
  memcpy(packet + 4, buf, buflen);
}

/* Check if the received acknowledgement is correct */
u32_t tftp_is_correct_ack(char *buf, int block)
{
  /* first make sure this is a data ACK packet */
  if (tftp_decode_op(buf) != TFTP_ACK)
    return 0;

  /* then compare block numbers */
  if (block != tftp_extract_block(buf))
    return 0;

  return 1;
}

