#include "S3C2416.h"
#include "lwip/tcp.h"

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_OFF
#endif
  
struct http_state {
	uint8_t *pbuffer;
	uint32_t left;
};

const char http_record_head[] = 
	"<html xmlns=\"http://www.w3.org/1999/xhtml\"> \
	<head> \
		<meta http-equiv=\"Content-Type\" content=\"text/html; charset=GBK\"> \
		<!--meta http-equiv=\"refresh\" content=\"1\"--> \
		<title>历史清分记录</title> \
	</head> \
	<body class=\"bodystyle\"> \
	<div style=\"background-color:#FFFFFF;\"> \
		<div align=\"center\" style=\"background-color:#0066CC;color:#fff;margin-top:30px; \"> \
			<h3> \
				<span>广州畅阳电子科技有限公司</span> \
			</h3> \
			<h3> \
				<span>硬币清分机历史清分记录</span> \
			</h3> \
		</div> \
		<div align=\"center\" style = \"margin-bottom:10px;\"> \
			<a href=\"/record.txt\"> \
				<button>导出TXT格式</button> \
			</a> \
		</div> \
		<div align=\"center\"> \
			<table border=\"1\" cellpadding=\"5\" style=\"background-color:#0066CC;color:#fff;\"> \
				<tbody> \
					<tr> \
					  <td align=\"center\" width=\"50\">索引值</td> \
					  <td align=\"center\" width=\"130\">清分时间</td> \
					  <td align=\"center\" width=\"80\">1元</td> \
					  <td align=\"center\" width=\"80\">5角铜</td> \
					  <td align=\"center\" width=\"80\">5角钢</td> \
					  <td align=\"center\" width=\"80\">大1角</td> \
					  <td align=\"center\" width=\"80\">1角钢</td> \
					  <td align=\"center\" width=\"80\">1角铝</td> \
					  <td align=\"center\" width=\"80\">总金额</td> \
					  <td align=\"center\" width=\"80\">总枚数</td> \
					  <td align=\"center\" width=\"80\">异币数</td> \
					</tr>";

char http_html[HTTP_BUF_SIZE];
const char http_record_tail[] = 
			"</tbody> \
			</table> \
			<a href=\"/prepage\"> \
				<button>上一页</button> \
			</a> \
				&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp \
				&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp \
				&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp \
				&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp \
			<a href=\"/nextpage\"> \
				<button>下一页</button> \
			</a> \
		</div> \
	</div> \
	</body> \
</html>";

char http_data_temp[HTTP_BUF_SIZE-1024];
char *str_p = http_data_temp;
char str_temp[256];

static void http_err(void *arg, err_t err)
{
	struct http_state *hs;

	LWIP_DEBUGF(HTTPD_DEBUG, ("http_err: %s", lwip_strerr(err)));
	hs = (struct http_state *)arg;
	if (hs != NULL) {
		mem_free(hs);
	}
}
/*-----------------------------------------------------------------------------------*/
static void http_close_conn(struct tcp_pcb *pcb, struct http_state *hs)
{
	err_t err;
	LWIP_DEBUGF(HTTPD_DEBUG, ("Closing connection %p\n", (void*)pcb));

	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_err(pcb, NULL);
	tcp_poll(pcb, NULL, 0);
	mem_free(hs);

	err = tcp_close(pcb);
	if (err != ERR_OK) {
		LWIP_DEBUGF(HTTPD_DEBUG, ("Error %d closing %p\n", err, (void*)pcb));
	}
}
/*-----------------------------------------------------------------------------------*/
static err_t http_write(struct tcp_pcb *pcb, void *arg)
{
	struct http_state *hs;
	uint16_t len;
	err_t err;
	hs = (struct http_state *)arg;
	if ((hs == NULL) || (hs->pbuffer==NULL)) {
		return ERR_BUF;
	}

	if (tcp_sndbuf(pcb) < hs->left) {
		len = tcp_sndbuf(pcb);
	} else {
		len = hs->left;
	}

	err = tcp_write(pcb, hs->pbuffer, len, 0);
	if (err == ERR_OK) {
		hs->pbuffer += len;
		hs->left -= len;
	}
	return err;
}

static err_t http_poll(void *arg, struct tcp_pcb *pcb)
{
	struct http_state *hs;
	hs = (struct http_state *)arg;

	LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("http_poll: pcb=%p hs=%p pcb_state=%s\n",
	(void*)pcb, (void*)hs, tcp_debug_state_str(pcb->state)));

	if (hs == NULL) {
		/* arg is null, close. */
		LWIP_DEBUGF(HTTPD_DEBUG, ("http_poll: arg is NULL, close\n"));
		http_close_conn(pcb, hs);
	} else {
		LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("http_poll: try to send more data\n"));
		if (http_write(pcb, arg) == ERR_OK) {
			// If we wrote anything to be sent, go ahead and send it now
			tcp_output(pcb);
		}
	}

	return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	struct http_state *hs;

	LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("http_sent %p\n", (void*)pcb));

	hs = (struct http_state *)arg;
	if (hs == NULL) {
		return ERR_OK;
	}
	if (hs->left > 0) {
		http_write(pcb, hs);
	} else {
		http_close_conn(pcb, hs);
	}

	return ERR_OK;
}


void fill_http_data (s_db_item_info * db_item_info_temp)
{		
	int i;
	HTTP_INSERT ("<tr><td align=\"center\" width=\"50\">%d</td>", db_item_info_temp->index+1);
	HTTP_INSERT ("<td align=\"center\" width=\"180\">%s</td>", db_item_info_temp->time);
//	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->m_1yuan);
//	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->m_5jiao);
//	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->m_1jiao);
//	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->m_1jiao_big);
//	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->m_10yuan);
//	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->m_5yuan);	
	for (i = 0; i < NORMAL_COIN_TYPE_NUM; i++){
		HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->coin_ctr[i]);
	}
	HTTP_INSERT("<td align=\"center\" width=\"80\">%d.%d%d (元)</td>", (db_item_info_temp->total_money/100),
																((db_item_info_temp->total_money%100)/10),
																((db_item_info_temp->total_money%100)%10));
	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->total_good);
	HTTP_INSERT("<td align=\"center\" width=\"80\">%d (枚)</td>", db_item_info_temp->total_ng);
}

void fill_http_txt_data (s_db_item_info * db_item_info_temp)
{
	int i;
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HTTP_INSERT("%5d", db_item_info_temp->index+1);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HTTP_INSERT("   %s", db_item_info_temp->time);
	 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	HTTP_INSERT("     %6d", db_item_info_temp->m_1yuan);
//	HTTP_INSERT("     %6d", db_item_info_temp->m_5jiao);
//	HTTP_INSERT("     %6d", db_item_info_temp->m_1jiao);
//	HTTP_INSERT("     %6d", db_item_info_temp->m_1jiao_big);
//	HTTP_INSERT("     %6d", db_item_info_temp->m_10yuan);
//	HTTP_INSERT("     %6d", db_item_info_temp->m_5yuan);
	for (i = 0; i < NORMAL_COIN_TYPE_NUM; i++){
		HTTP_INSERT("     %6d", db_item_info_temp->coin_ctr[i]);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HTTP_INSERT("     %6d.%d%d", (db_item_info_temp->total_money/100),((db_item_info_temp->total_money%100)/10),((db_item_info_temp->total_money%100)%10));
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HTTP_INSERT("     %6d", db_item_info_temp->total_good);
	HTTP_INSERT("     %6d", db_item_info_temp->total_ng);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HTTP_INSERT ("\r\n");
	//HTTP_INSERT ("\r\n|-----|-------------------|----|--------------|----------------|-----------|\r\n");
}
U8 page_buf[PAGE_BYTE_SIZE];
void gen_http_txt (void)
{
	U8 r_code;
	uint16_t i,j;
	uint16_t record_num = para_set_value.data.db_total_item_num;
	uint16_t record_id = 0;
	s_db_item_info * db_item_info_temp;
	s_db_item_block * db_item_block_temp;
	if (record_num > 0){
		i = 0;
		HTTP_START();
		HTTP_INSERT ("                                                        广州畅阳电子科技有限公司\r\n\r\n");
		HTTP_INSERT ("                                                         硬币清分机历史清分记录\r\n\r\n");
		//HTTP_INSERT ("|-----|-------------------|------|--------------|----------------|-----------|\r\n");
		//HTTP_INSERT (" 索引              清分时间      1元(枚)    5角(枚)    1角(枚)  大1角(枚)   10元(枚)    5元(枚)    总金额(元) 总枚数(枚) 异币数(枚)\r\n");
		HTTP_INSERT (" 索引              清分时间      1元(枚)  5角铜(枚)  5角钢(枚)  大1角(枚)  1角钢(枚)  1角铝(枚)    总金额(元) 总枚数(枚) 异币数(枚)\r\n");
		//HTTP_INSERT ("|-----|-------------------|------|--------------|----------------|-----------|\r\n");
		while (record_id < record_num){
			cy_print ("---------------------------------------------------------------------------------------\n");
			cy_print ("Display pre page data, db_total_item_num = %d\n", para_set_value.data.db_total_item_num);
			cy_print("Block = %d page = %d addr = %d db_id = %d item_index = %d\n",
				YQNDHISTORY_DB_ID_PAGE_ADDR / BLOCK_BYTE_SIZE,
				(YQNDHISTORY_DB_ID_PAGE_ADDR - YQNDHISTORYBLOCK),
				YQNDHISTORY_DB_ID_PAGE_ADDR,
				record_id+1,
				record_id % PAGE_ITEM_NUM_SIZE);
			//r_code = Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (record_id * ITEM_SIZE / PAGE_BYTE_SIZE), yqnddata);
			r_code = Nand_ReadPage(HISTORY_START_BLOCK_NUM, HISTORY_START_PAGE_NUM + (record_id * ITEM_SIZE / PAGE_BYTE_SIZE), page_buf);
			if (r_code == 0){
				cy_println ("record_id = %d, Read Nand Page %d OK,", record_id, record_id/PAGE_ITEM_NUM_SIZE);
				j = 0;
				db_item_block_temp = (s_db_item_block *)page_buf;
				while ((i < record_num) && (j < 32)){
					db_item_info_temp = (s_db_item_info *)&(db_item_block_temp->item_info_array[j % PAGE_ITEM_NUM_SIZE]);
					fill_http_txt_data (db_item_info_temp);
					j++;
					i++;
				}
			}else{
				cy_println ("Nand Error");
				HTTP_INSERT ("Nand Error");
			}
			record_id += PAGE_ITEM_NUM_SIZE;
		}
		HTTP_FIN();
	}
}

/*-----------------------------------------------------------------------------------*/
static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	uint8_t *pBuffer;
	struct http_state *hs = (struct http_state *)arg;

	LWIP_DEBUGF(HTTPD_DEBUG | LWIP_DBG_TRACE, ("http_recv: pcb=%p pbuf=%p err=%s\n", (void*)pcb,
	(void*)p, lwip_strerr(err)));

	if ((err != ERR_OK) || (p == NULL) || (hs == NULL)) {
	  /* error or closed by other side? */
		if (p != NULL) {
			/* Inform TCP that we have taken the data. */
			tcp_recved(pcb, p->tot_len);
			pbuf_free(p);
		}
		if (hs == NULL) {
			/* this should not happen, only to be robust */
			LWIP_DEBUGF(HTTPD_DEBUG, ("Error, http_recv: hs is NULL, close\n"));
		}
		http_close_conn(pcb, hs);
		return ERR_OK;
	}
	/* Inform TCP that we have taken the data. */
	tcp_recved(pcb, p->tot_len);

	/* Is this an HTTP GET command? (only check the first 5 chars, since
	there are other formats for GET, and we're keeping it very simple )*/
	pBuffer = p->payload;
	if (strncmp((const char *)pBuffer, "GET /prepage", 12) == 0) {
		pbuf_free(p);
		/* Send the HTML header
		* subtract 1 from the size, since we dont send the \0 in the string
		* NETCONN_NOCOPY: our data is const static, so no need to copy it
		*/
		hs->pbuffer = (uint8_t *)http_html;
		if (para_set_value.data.db_total_item_num == 0){
			HTTP_START();
			HTTP_END();
		}else if (db_id == 0){
			db_id = para_set_value.data.db_total_item_num;
		}
		yqsql_exec(DBDISPLAY);	  //
		hs->left =  strlen (http_html) - 1;//sizeof(http_html)-1;
		http_write(pcb, hs);
	}else if (strncmp((const char *)pBuffer, "GET /nextpage", 13) == 0) {
		pbuf_free(p);
		/* Send the HTML header
		* subtract 1 from the size, since we dont send the \0 in the string
		* NETCONN_NOCOPY: our data is const static, so no need to copy it
		*/
		hs->pbuffer = (uint8_t *)http_html;
		if (para_set_value.data.db_total_item_num == 0){
			HTTP_START();
			HTTP_END();
		}
		yqsql_exec(DBDISPLAYBACK);
		hs->left =  strlen (http_html) - 1;//sizeof(http_html)-1;
		http_write(pcb, hs);
	}else if (strncmp((const char *)pBuffer, "GET /record.txt", 15) == 0) {
		pbuf_free(p);
		/* Send the HTML header
		* subtract 1 from the size, since we dont send the \0 in the string
		* NETCONN_NOCOPY: our data is const static, so no need to copy it
		*/
		hs->pbuffer = (uint8_t *)http_html;
		if (para_set_value.data.db_total_item_num == 0){
			HTTP_START();
				HTTP_INSERT ("暂无清分记录\r\n");
			HTTP_FIN();
		}else{
			gen_http_txt();
		}
		hs->left =  strlen (http_html) - 1;//sizeof(http_html)-1;
		http_write(pcb, hs);
	}else if (strncmp((const char *)pBuffer, "GET /", 5) == 0) {
		pbuf_free(p);
		/* Send the HTML header
		* subtract 1 from the size, since we dont send the \0 in the string
		* NETCONN_NOCOPY: our data is const static, so no need to copy it
		*/
		hs->pbuffer = (uint8_t *)http_html;
		if (para_set_value.data.db_total_item_num == 0){
			HTTP_START();
			HTTP_END();
		}else{
			db_id = para_set_value.data.db_total_item_num;
			yqsql_exec(DBDISPLAY);	  //
		}
		hs->left =  strlen (http_html) - 1;//sizeof(http_html)-1;
		http_write(pcb, hs);
	} else {
		pbuf_free(p);
		http_close_conn(pcb, hs);
	}
	return ERR_OK;
}

static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	struct http_state *hs;
	struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen*)arg;

	LWIP_DEBUGF(HTTPD_DEBUG, ("http_accept %p / %p\n", (void*)pcb, arg));
	/* Decrease the listen backlog counter */
	tcp_accepted(lpcb);
	/* Set priority */
	tcp_setprio(pcb, TCP_PRIO_MIN);

	/* Allocate memory for the structure that holds the state of the
	   connection - initialized by that function. */
	hs = mem_malloc(sizeof(struct http_state));
	if (hs == NULL) {
		LWIP_DEBUGF(HTTPD_DEBUG, ("http_accept: Out of memory, RST\n"));
		return ERR_MEM;
	}
	hs->pbuffer = NULL;
	hs->left = 0;

	/* Tell TCP that this is the structure we wish to be passed for our
	   callbacks. */
	tcp_arg(pcb, hs);

	/* Set up the various callback functions */
	tcp_recv(pcb, http_recv);
	tcp_err(pcb, http_err);
	tcp_poll(pcb, http_poll, 10);
	tcp_sent(pcb, http_sent);

	return ERR_OK;
}

void httpd_init(void)
{
	struct tcp_pcb *pcb;
	err_t err;

	pcb = tcp_new();
	LWIP_ASSERT("httpd_init: tcp_new failed", pcb != NULL);
	/* Set priority */
	tcp_setprio(pcb, TCP_PRIO_MIN);
	/* set SOF_REUSEADDR here to explicitly bind httpd to multiple interfaces */
	err = tcp_bind(pcb, IP_ADDR_ANY, 80);
	LWIP_ASSERT("httpd_init: tcp_bind failed", err == ERR_OK);
	/* Put the tcp into LISTEN state */
	pcb = tcp_listen(pcb);
	LWIP_ASSERT("httpd_init: tcp_listen failed", pcb != NULL);
	/* initialize callback arg and accept callback */
	tcp_arg(pcb, pcb);
	tcp_accept(pcb, http_accept);
}

