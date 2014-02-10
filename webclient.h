
#ifndef __WebClient_h
#define __WebClient_h


struct URI {
	char scheme[8]; //最大"https"の５文字と仮定して決め打ち
	char host[256];
	char const *path;
	int port;
};

struct URI init_uri(char const *str);

struct RESULT {
	char *headers;
	char *content;
};

char *get_ssl_error(void);
char **set_default_headers(struct URI const *uri, char const *post,char **hdrs);
char **make_http_request(struct URI const *uri, char const *post, char **str);
struct RESULT **parse_http_result(char *begin, char *end, struct RESULT **out);
char **https_get(struct URI const *uri, char const *post, char **out);

struct RESULT *send_req(struct URI const *uri, char const *post, struct RESULT *out);
char **alloc_strcat(char **dest, char const *src);


#endif
