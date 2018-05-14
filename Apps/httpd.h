#ifndef __HTTPD_H__
#define __HTTPD_H__

#ifdef __cplusplus
extern "C" {
#endif

extern const char http_record_head[];
extern const char http_record_tail[];
extern char http_html[8192];
extern char http_data_temp[7*1024];
extern char *str_p;
extern char str_temp[256];

#define HTTP_START() str_p = http_data_temp;
#define HTTP_INSERT(format, arg...) sprintf(str_temp,format, ## arg); \
		strcpy (str_p, str_temp); \
		str_p += (strlen(str_temp));
#define HTTP_END()  sprintf (http_html, "%s%s%s", http_record_head, \
					http_data_temp, http_record_tail);	
	
	
void httpd_init(void);
void fill_http_data (s_db_item_info * db_item_info_temp);

#ifdef __cplusplus
}
#endif

#endif /* __HTTPD_H__ */
