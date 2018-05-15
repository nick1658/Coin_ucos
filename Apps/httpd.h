#ifndef __HTTPD_H__
#define __HTTPD_H__

#ifdef __cplusplus
extern "C" {
#endif

	
#define HTTP_BUF_SIZE (300*1024)
	
	
extern const char http_record_head[];
extern const char http_record_tail[];
extern char http_html[HTTP_BUF_SIZE];
extern char http_data_temp[HTTP_BUF_SIZE-1024];
extern char *str_p;
extern char str_temp[256];

#define HTTP_HEADER_ACTION "POST /his.txt HTTP/1.1\r\n"                         \
       "CONTENT-TYPE: text/xml ; charset=\"utf-8\"\r\n"    \
       "SOAPACTION: \"www.baidu.com\"\r\n"   \
       "Cache-Control: no-cache\r\n" \
       "Pragma: no-cache\r\n" \
     "HOST: 192.168.1.250\r\n"               \
       "Content-Length: 256 \r\n\r\n"
	
#define HTTP_START() str_p = http_data_temp;
#define HTTP_INSERT(format, arg...) sprintf(str_temp,format, ## arg); \
		strcpy (str_p, str_temp); \
		str_p += (strlen(str_temp));
#define HTTP_END()  sprintf (http_html, "%s%s%s", http_record_head, \
					http_data_temp, http_record_tail);	
					
#define HTTP_FIN() sprintf (http_html, "%s", \
					http_data_temp);	


extern U8 page_buf[PAGE_BYTE_SIZE];
	
void httpd_init(void);
void fill_http_data (s_db_item_info * db_item_info_temp);

#ifdef __cplusplus
}
#endif

#endif /* __HTTPD_H__ */
