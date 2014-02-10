#include <stdio.h>
#include <stdlib.h>

#include "tweet.h"
#include "webclient.h"
#include <oauth.h>

#include "keys/mykey.h"
//#include "keys/soramimi_jp.h"

int main(void)
{
	char *rep = NULL;
	init_keys((union KEYS){.keys_array = {c_key, c_sec, t_key, t_sec}});
	printf("%d\n",post_update("home_timelineに対応", &rep, 0, 0, (union L_L){.cood = {0, 0}}, 0, -1, -1));
	if(rep) {puts(rep);}
	free(rep);
	rep = NULL;
	printf("%d\n",get_home_timeline(&rep, 5, 0, 0, -1, -1, -1, -1) );
	if (rep){puts(rep);}
	free(rep);
	rep = NULL;
	return 0;
}

