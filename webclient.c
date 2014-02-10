#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#define closesocket(S) close(S)
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#include "webclient.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/types.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define USER_AGENT "Generic Web Client"



struct URI init_uri(char const *str)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	struct URI uri = {"","",NULL,0};
	char const *left;
	char const *right;
	left = str;
	right = strstr(left, "://");
	if (right) {
		strncpy(uri.scheme, str, right - str);
		left = right + 3;
	}
	right = strchr(left, '/');
	if (right) {
		char const *p = strchr(left, ':');
		if (p && left < p && p < right) {
			int n = 0;
			char const *q = p + 1;
			while (q < right) {
				if (isdigit(*q & 0xff)) {
					n = n * 10 + (*q - '0');
				} else {
					n = -1;
					break;
				}
				q++;
			}
			strncpy(uri.host, left, p - left + 1);
			if (n > 0 && n < 65536) {
				uri.port = n;
			}
		} else {
			strncpy(uri.host, left, right - left);
		}
		uri.path = right;
	}
	return uri;
}

int isssl(struct URI uri)
{
	#ifdef DEBUG
	puts(__func__);
	#endif
	if (!strcmp(uri.scheme, "https")) {return 1;}
	if (!strcmp(uri.scheme, "http")) {return 0;}
	if (uri.port == 443) {return 1;}
	return 0;
}



char *get_ssl_error(void)
{
	static char tmp[1024] = {0};
	unsigned long e = ERR_get_error();
	ERR_error_string_n(e, tmp, sizeof(tmp));
	return tmp;
}

static int get_port(struct URI const *uri, char const *scheme, char const *protocol)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	int port = uri->port;
	if (port < 1 || port > 65535) {
		struct servent *s;
		s = getservbyname(uri->scheme, protocol);
		if (s) {
			port = ntohs(s->s_port);
		} else {
			s = getservbyname(scheme, protocol);
			if (s) {
				port = ntohs(s->s_port);
			}
		}
		if (port < 1 || port > 65535) {
			port = 80;
		}
	}
	return port;
}

inline static char *to_s(size_t n)
{
	static char tmp[100];
	sprintf(tmp, "%lu", n);
	return tmp;
}

char **set_default_headers(struct URI const *uri, char const *post,	char **hdrs)
{
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	alloc_strcat(hdrs, "Host: ");
	alloc_strcat(hdrs, uri->host);
	alloc_strcat(hdrs,"\r\n");
	alloc_strcat(hdrs, "User-Agent: ");
	alloc_strcat(hdrs, USER_AGENT);
	alloc_strcat(hdrs,"\r\n");
	alloc_strcat(hdrs, "Accept: */*");
	alloc_strcat(hdrs,"\r\n");
	alloc_strcat(hdrs, "Connection: close");
	alloc_strcat(hdrs,"\r\n");
	alloc_strcat(hdrs, "Content-Length: ");
	alloc_strcat(hdrs, to_s(post?strlen(post):0));
	alloc_strcat(hdrs,"\r\n");
	alloc_strcat(hdrs, "Content-Type: application/x-www-form-urlencoded");
	alloc_strcat(hdrs,"\r\n");
	alloc_strcat(hdrs, "Authorization: OAuth");
	alloc_strcat(hdrs,"\r\n");

	return hdrs;
}

char **make_http_request(struct URI const *uri, char const *post, char **str)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	char *hdr = NULL;
	set_default_headers(uri, post, &hdr);

	alloc_strcat(str, post&&strlen(post) ? "POST " : "GET ");
	alloc_strcat(str, uri->path);
	alloc_strcat(str, " HTTP/1.0");
	alloc_strcat(str,"\r\n");
	alloc_strcat(str, hdr);
	alloc_strcat(str,"\r\n");
	
	free(hdr);hdr = NULL;

	return str;
}
struct RESULT **parse_http_result(char *begin, char *end, struct RESULT **out)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (begin < end) {
		char *left = begin;
		char *right = left;
		while (right < end) {
			if (*right == '\r' || *right == '\n') {
				if (right + 1 < end && *right == '\r' && *(right + 1) == '\n') {
					*right = '\n';
					*(right + 1) = '\0';
					right++;
				}
				
				if (left < right) {
					alloc_strcat(&((*out)->headers), left);
				}
				right++;
				/*if (*right == '\r' || *right == '\n') {
					if (right + 1 < end && *right == '\r' && right[1] == '\n') {
						right++;
					}
					right++;
					left = right;
					break;
				}*///最高にクール
				left = right;
			} else {
				right++;
			}
		}
		if (left < end) {
			alloc_strcat(&((*out)->content), left);
		}
	}
	return out;
}




static void ssend_(SSL *ssl, char const *ptr, int len)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	int n = 0;
	while (len > 0) {		
		n = SSL_write(ssl, ptr, len);
		if (n < 1 || n > len) {
			fprintf(stderr,"%s\n", get_ssl_error());	
		}
		ptr += n;
		len -= n;
	}
}

char **https_get(struct URI const *uri, char const *post, char **out)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	do {
		int ret;
		socket_t s;
		struct hostent *servhost  = NULL; 
		struct sockaddr_in server;

		SSL *ssl;
		SSL_CTX *ctx;

		servhost = gethostbyname(uri->host);
		if (!servhost) {
			fprintf(stderr,"gethostbyname failed.\n");
			break;
		}

		memset((char *)&server, 0, sizeof(server));
		server.sin_family = AF_INET;

		memcpy((char *)&server.sin_addr, servhost->h_addr_list[0], servhost->h_length);

		server.sin_port = htons(get_port(uri, "https", "tcp"));

		s = socket(AF_INET, SOCK_STREAM, 0); 
		if (s == INVALID_SOCKET) {
			fprintf(stderr,"socket failed.\n");
			break;
		}

		if (connect(s, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR) {
			fprintf(stderr,"connect failed.\n");
			break;
		}

		SSL_load_error_strings();
		SSL_library_init();
		ctx = SSL_CTX_new(SSLv23_client_method());
		if (!ctx) {
			fprintf(stderr,"%s\n", get_ssl_error());
			break;
		}

		ssl = SSL_new(ctx);
		if (!ssl) {
			fprintf(stderr,"%s\n", get_ssl_error());
			break;
		}

		ret = SSL_set_fd(ssl, s);
		if (ret == 0) {
			fprintf(stderr,"%s\n", get_ssl_error());
			break;
		}

		RAND_poll();
		srand(time(NULL));
		while (RAND_status() == 0) {
			unsigned short rand_ret = rand() % 65536;
			RAND_seed(&rand_ret, sizeof(rand_ret));
		}

		ret = SSL_connect(ssl);
		if (ret != 1) {
			fprintf(stderr,"%s\n", get_ssl_error());
			break;
		}


		char *request = NULL;
		make_http_request(uri, post, &request);
		printf("request = %s\n",request);
		ssend_(ssl, request, strlen(request));
		if (post && post[0]) {
			printf("post = %s\n",post);
			ssend_(ssl, post, strlen(post));
		}

		char buf[4096] = {0};
		for(int n = 0;;) {
			if (buf[0]) {
				memset(buf,0,strlen(buf));
			}			
			n = SSL_read(ssl, buf, sizeof(buf));
			buf[strlen(buf)-1] = '\0';
			if (n < 1) break;
			alloc_strcat(out, buf);
		}

		SSL_shutdown(ssl); 
		closesocket(s);

		SSL_free(ssl); 
		SSL_CTX_free(ctx);
		ERR_free_strings();

		free(request);request = NULL;
		return out;

	} while(0);
	
	return NULL;
}

struct RESULT *send_req(struct URI const *uri, char const *post, struct RESULT *out)
{
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	char *res = NULL;
	if (isssl(*uri)) {
		https_get(uri, post, &res);
	} 

	if (*res) {
		char *begin = res;
		char *end = begin + strlen(res);
		parse_http_result(begin, end, &out);
	}
	free(res);res = NULL;
	return out;
}


char **alloc_strcat(char **dest, char const *src)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	int destlen = 0, srclen = 0;
	destlen = (*dest)?strlen(*dest):0;
	srclen = strlen(src);
	*dest = (char*)realloc(*dest, (destlen + srclen + 1)*sizeof(char));
	if (dest) {
		if (!(*dest)) {
			memset(*dest, 0, strlen(src) + 1);
		}
		strncat(*dest, src, strlen(src));
	} else {
		fprintf(stderr,"realloc failed\n");
	}
	return dest;
}

