#include <stdio.h>
#include <stdlib.h>

#include <oauth.h>
#include <curl/curl.h>

#include "tweet.h"
#include "keys/mykey.h"

int main(void)
{
	char *rep = NULL;
	register_keys(&(union KEYS){.keys_array = {c_key, c_sec, t_key, t_sec}});
	curl_global_init(CURL_GLOBAL_DEFAULT);
	printf("%d\n",post_statuses_update("筑波の後期発表まで後何日かをツイートするbotつくって後輩のサーバで動かす（予定）", &rep, 0, 0, (struct GEOCODE){0, 0, 0, ""}, 0, -1, -1));
	if(rep) {puts(rep);}
	free(rep);
	rep = NULL;
	printf("%d\n",get_statuses_home_timeline(&rep, 5, 0, 0, -1, -1, -1, -1) );
	if (rep){puts(rep);}
	free(rep);
	rep = NULL;
	curl_global_cleanup();
	return 0;
}

