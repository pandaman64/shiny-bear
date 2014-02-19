// vim: set foldmethod=syntax :
#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <math.h>

#include <oauth.h>
#include "tweet.h"

static char **alloc_strcat(char **dest, char const *src) {
	#ifdef DEBUG
	puts(__func__);
	#endif

	int destlen = 0, srclen = 0,wasdestnull = 0;
	destlen = (*dest)?strlen(*dest):0;
	srclen = strlen(src);
	wasdestnull = !(*dest);
	*dest = (char*)realloc(*dest, (destlen + srclen + 1)*sizeof(char));
	if (dest) {
		if (wasdestnull) {
			memset(*dest, 0, strlen(src) + 1);
		}
		strncat(*dest, src, strlen(src));
	} else {
		fprintf(stderr,"realloc failed\n");
	}
	return dest;
}

static size_t write_data(char *buffer, size_t size, size_t nmemb, void *rep) {
	*(buffer + size * nmemb) = '\0';
	alloc_strcat((char**)rep, buffer);

	return size * nmemb;
}

static int http_request(char const *u, char const *p, char **rep) {
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (*rep) {
		memset(*rep,0,strlen(*rep));
	}
	CURL *curl;
	CURLcode ret;
	curl = curl_easy_init();
	if (!curl) {
		fprintf(stderr, "failed to initialize curl\n");
	}
	curl_easy_setopt (curl, CURLOPT_URL, request);
	if (p && *p) {
		curl_easy_setopt (curl, CURLOPT_POSTFIELDS, (void *) p);
	}
	//is it good? i dont know.
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) res);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, write_data);

	ret = curl_easy_perform (curl);
	if (ret != CURLE_OK) {
		fprintf (stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror (ret));
	}

	curl_easy_cleanup(curl);
	
	return ret;
}

static union KEYS keys = {.keys_array = {NULL,NULL,NULL,NULL}}; 

union KEYS init_keys(union KEYS k) {
	return keys = k;
}

int check_keys(void) {
	return keys.keys_array[0]&&keys.keys_array[1]&&keys.keys_array[2]&&keys.keys_array[3];
}

char const *api_uri_1_1 = "https://api.twitter.com/1.1/";

char const * api_uri[] = {
[MENTIONS_TIMELINE] = "statuses/mentions_timeline.json",
[USER_TIMELINE] = "statuses/user_timeline.json",
[HOME_TIMELINE]  = "statuses/home_timeline.json",
[RETWEETS_OF_ME] = "statuses/retweets_of_me.json",
[RETWEETS_ID] = "statuses/retweets/",
[SHOW] = "statuses/show.json",
[DESTROY_ID] = "statuses/destroy/",
[RETWEET_ID] = "statuses/retweet/",
[UPDATE] = "statuses/update.json",
[OEMBED] = "statuses/oembed.json",
[RETWEETERS_IDS] = "retweeters/ids.json",
[TWEETS] = "search/tweets.json",
};

inline static char **add_que_or_amp(enum APIS api, char **uri) {
	alloc_strcat(uri, strlen(*uri)==(strlen(api_uri_1_1) + strlen(api_uri[api]))?"?":"&");
	return uri;
}

static char **add_count(enum APIS api, char **uri, int count) {
	if (count) {
		char cnt[8] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "count=");
		snprintf(cnt, sizeof(cnt), "%d", count<201?count:200);
		alloc_strcat(uri, cnt);
	}
	return uri;
}

static char **add_id(enum APIS api, char **uri, id_t id) {
	if (id) {
		char i[32] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "id=");
		snprintf(i, sizeof(i), "%llu", id);
		alloc_strcat(uri, i);
	}
	return uri;
}

static char **add_since_id(enum APIS api, char **uri, id_t since_id) {
	if (since_id) {
		char id[32] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "since_id=");
		snprintf(id, sizeof(id), "%llu", since_id);
		alloc_strcat(uri, id);
	}
	return uri;
}

static char **add_max_id(enum APIS api, char **uri, id_t max_id) {
	if (max_id) {
		char id[32] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "max_id=");
		snprintf(id, sizeof(id), "%llu", max_id);
		alloc_strcat(uri, id);
	}
	return uri;
}

static char **add_trim_user(enum APIS api, char **uri, int trim_user) {
	if (trim_user != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "trim_user=");
		snprintf(boolian, sizeof(boolian), "%d", !!trim_user);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_contributor_details(enum APIS api, char **uri, int contributor_details) {
	if (contributor_details != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "contributor_details=");
		snprintf(boolian, sizeof(boolian), "%d", !!contributor_details);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_include_entities(enum APIS api, char **uri, int include_entities) {
	if (include_entities != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "include_entities=");
		snprintf(boolian, sizeof(boolian), "%d", !!include_entities);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_include_rts(enum APIS api, char **uri, int include_rts, int count) {
	if (count || (include_rts != -1)) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "include_rts=");

		snprintf(boolian, sizeof(boolian), "%d", count || (include_rts != -1));
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_user_id(enum APIS api, char **uri, id_t user_id) {
	if (user_id) {
		char id[32] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "user_id=");
		snprintf(id, sizeof(id), "%llu", user_id);
		alloc_strcat(uri, id);
	}
	return uri;
}

static char **add_screen_name(enum APIS api, char **uri, char *screen_name) {
	if (screen_name && *screen_name) {
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "screen_name=");
		alloc_strcat(uri, screen_name);
	}
	return uri;
}

static char **add_exclude_replies(enum APIS api, char **uri, int exclude_replies) {
	if (exclude_replies != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "exclude_replies=");
		snprintf(boolian, sizeof(boolian), "%d", !!exclude_replies);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_include_user_entities(enum APIS api, char **uri, int include_user_entities) {
	if (include_user_entities != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "include_user_entities=");
		snprintf(boolian, sizeof(boolian), "%d", !!include_user_entities);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_include_my_retweet(enum APIS api, char **uri, int include_my_retweet) {
	if (include_my_retweet != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "include_my_retweet=");
		snprintf(boolian, sizeof(boolian), "%d", !!include_my_retweet);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_status(enum APIS api, char **uri, char *status) {
	if (status && *status)	{
	char *escaped_msg = oauth_url_escape(status);
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "status=");
		alloc_strcat(uri, escaped_msg);
		free(escaped_msg);escaped_msg = NULL;
	}
	return uri;
}

static char **add_in_reply_to_status_id(enum APIS api, char **uri, id_t in_reply_to_status_id) {
	if (in_reply_to_status_id) {
		char id[32] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "in_reply_to_status_id=");
		snprintf(id, sizeof(id), "%llu", in_reply_to_status_id);
		alloc_strcat(uri, id);
	}
	return uri;
}

static char **add_coods(enum APIS api, char **uri, struct GEOCODE l_l) {
	if ((int)(fabs(l_l.latitude)) < 90 && (int)(fabs(l_l.longitude)) < 180) {
		char latitude[32];
		char longitude[32];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "latitude=");
		snprintf(latitude, sizeof(latitude), "%2.12f", l_l.latitude);
		alloc_strcat(uri, latitude);
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "longitude=");
		snprintf(longitude, sizeof(longitude), "%2.12f", l_l.longitude);
		alloc_strcat(uri, longitude);
	}
	return uri;
}

static char **add_place_id(enum APIS api, char **uri, id_t place_id) {
	if (place_id) {
		char id[32] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "place_id=");
		snprintf(id, sizeof(id), "%llx", place_id);
		alloc_strcat(uri, id);
	}
	return uri;
}

static char **add_display_coordinates(enum APIS api, char **uri, int display_coordinates) {
	if (display_coordinates != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "display_coordinates=");
		snprintf(boolian, sizeof(boolian), "%d", !!display_coordinates);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_url(enum APIS api, char **uri, char *url){
	if(url && *url) {
		char *escaped_msg = oauth_url_escape(url);
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "url=");
		alloc_strcat(uri, escaped_msg);
		free(escaped_msg);escaped_msg = NULL;
	}
	return uri;
}

static char **add_maxwidth(enum APIS api, char **uri, int maxwidth) {
	if (249 < maxwidth && maxwidth < 551) {
		char cnt[8] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "maxwidth=");
		snprintf(cnt, sizeof(cnt), "%d", maxwidth);
		alloc_strcat(uri, cnt);
	}
	return uri;
}

static char **add_hide_media(enum APIS api, char **uri, int hide_media) {
	if (hide_media != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "hide_media=");
		snprintf(boolian, sizeof(boolian), "%d", !!hide_media);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_hide_thread(enum APIS api, char **uri, int hide_thread) {
	if (hide_thread != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "hide_thread=");
		snprintf(boolian, sizeof(boolian), "%d", !!hide_thread);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_omit_script(enum APIS api, char **uri, int omit_script) {
	if (omit_script != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "omit_script=");
		snprintf(boolian, sizeof(boolian), "%d", !!omit_script);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_align(enum APIS api, char **uri, enum ALIGN align) {
	if (align < (CENTER + 1)) {
		char const *algn[] = {"none", "left", "right", "center"};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "align=");
		alloc_strcat(uri, algn[align]);
	}
	return uri;
}

static char **add_related(enum APIS api, char **uri, char *related) {
	if (related && *related) {
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "related=");
		alloc_strcat(uri, related);
	}
	return uri;
}

static char **add_lang(enum APIS api, char **uri, char *lang) {
	if (lang && *lang) {
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "lang=");
		alloc_strcat(uri, lang);
	}
	return uri;
}

static char **add_cursor(enum APIS api, char **uri, int cursor) {
	if (cursor) {
		char cur[32] = {0};
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "cursor=");
		snprintf(cur, sizeof(cur), "%u", cursor);
		alloc_strcat(uri, cur);
	}
	return uri;
}

static char **add_stringify_ids(enum APIS api, char **uri, int stringify_ids) {
	if (stringify_ids != -1) {
		char boolian[2];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "stringify_ids=");
		snprintf(boolian, sizeof(boolian), "%d", !!stringify_ids);
		alloc_strcat(uri, boolian);
	}
	return uri;
}

static char **add_q(enum APIS api, char **uri, char *q){
	if(q && *q) {
		char *escaped_msg = oauth_url_escape(q);
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "q=");
		alloc_strcat(uri, escaped_msg);
		free(escaped_msg);escaped_msg = NULL;
	}
	return uri;
}

static char **add_geocode(enum APIS api, char **uri, struct GEOCODE geocode) {
	if ((int)(fabs(geocode.latitude)) < 90 && (int)(fabs(geocode.longitude)) < 180 && geocode.radius != 0 && geocode.unit && *geocode.unit) {
		char latitude[32];
		char longitude[32];
		char rad[8];
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "geocode=");
		snprintf(latitude, sizeof(latitude), "%2.12f,", geocode.latitude);
		alloc_strcat(uri, latitude);
		snprintf(longitude, sizeof(longitude), "%2.12f,", geocode.longitude);
		alloc_strcat(uri, longitude);
		snprintf(rad, sizeof(rad), "%d", geocode.radius);
		alloc_strcat(uri, rad);
		alloc_strcat(uri, geocode.unit);
	}
	return uri;
}

static char **add_locale(enum APIS api, char **uri, char *locale) {
	if (locale && *locale) {
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "locale=");
		alloc_strcat(uri, locale);
	}
	return uri;
}

static char **add_result_type(enum APIS api, char **uri, int result_type) {
	if (result_type) {
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "result_type=");
		if (result_type & MIXED) {
			alloc_strcat(uri, "mixed");
		}
		if (result_type & RECENT) {
			if (result_type & MIXED) {
				alloc_strcat(uri, ",");
			}
			alloc_strcat(uri, "recent");
		}
		if (result_type & POPULAR) {
			if (result_type & (MIXED | RECENT)) {
				alloc_strcat(uri, ",");
			}
			alloc_strcat(uri, "popular");
		}
	}
	return uri;
}

static char **add_until(enum APIS api, char **uri, char *until) {
	if (until && *until) {
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "until=");
		alloc_strcat(uri, until);
	}
	return uri;
}

static char **add_callback(enum APIS api, char **uri, char *callback) {
	if (callback && *callback) {
		add_que_or_amp(api, uri);
		alloc_strcat(uri, "callback=");
		alloc_strcat(uri, callback);
	}
	return uri;
}

int get_mentions_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/mentions_timeline.json
Parameters
count optional

Specifies the number of tweets to try and retrieve, up to a maximum of 200. The value of count is best thought of as a limit to the number of tweets to return because suspended or deleted content is removed after the count has been applied. We include retweets in the count, even if include_rts is not supplied. It is recommended you always send include_rts=1 when using this API method.

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = MENTIONS_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	
	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_contributor_details(api, &uri, contributor_details);
	add_include_entities(api, &uri, include_entities);
	add_include_rts(api, &uri, include_rts, count);
	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_user_timeline (
	id_t user_id, //Always specify either an user_id or screen_name when requesting a user timeline.
	char *screen_name, //Always specify either an user_id or screen_name when requesting a user timeline.
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int exclude_replies, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*


Resource URL
https://api.twitter.com/1.1/statuses/user_timeline.json
Parameters

Always specify either an user_id or screen_name when requesting a user timeline.

user_id optional

The ID of the user for whom to return results for.

Example Values: 12345

screen_name optional

The screen name of the user for whom to return results for.

Example Values: noradio

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

count optional

Specifies the number of tweets to try and retrieve, up to a maximum of 200 per distinct request. The value of count is best thought of as a limit to the number of tweets to return because suspended or deleted content is removed after the count has been applied. We include retweets in the count, even if include_rts is not supplied. It is recommended you always send include_rts=1 when using this API method.

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

exclude_replies optional

This parameter will prevent replies from appearing in the returned timeline. Using exclude_replies with the count parameter will mean you will receive up-to count tweets — this is because the count parameter retrieves that many tweets before filtering out retweets and replies. This parameter is only supported for JSON and XML responses.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_rts optional

When set to false, the timeline will strip any native retweets (though they will still count toward both the maximal length of the timeline and the slice selected by the count parameter). Note: If you're using the trim_user parameter in conjunction with include_rts, the retweets will still contain a full user object.

Example Values: false


*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!(user_id || (screen_name && screen_name[0]))) {
		fprintf(stderr, "need user_id number or screen_name text\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = USER_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	
	add_user_id(api, &uri, user_id);
	add_screen_name(api, &uri, screen_name);
	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_exclude_replies(api, &uri, exclude_replies);
	add_contributor_details(api, &uri, contributor_details);
	add_include_rts(api, &uri, include_rts, count);
	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_home_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int exclude_replies, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/home_timeline.json
Parameters
count optional

Specifies the number of records to retrieve. Must be less than or equal to 200. Defaults to 20.

Example Values: 5

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

exclude_replies optional

This parameter will prevent replies from appearing in the returned timeline. Using exclude_replies with the count parameter will mean you will receive up-to count tweets — this is because the count parameter retrieves that many tweets before filtering out retweets and replies.

Example Values: true

contributor_details optional

This parameter enhances the contributors element of the status response to include the screen_name of the contributor. By default only the user_id of the contributor is included.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = HOME_TIMELINE;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	
	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_exclude_replies(api, &uri, exclude_replies);
	add_contributor_details(api, &uri, contributor_details);
	add_include_entities(api, &uri, include_entities);
	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_retweets_of_me (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	) {
/*


Resource URL
https://api.twitter.com/1.1/statuses/retweets_of_me.json
Parameters
count optional

Specifies the number of records to retrieve. Must be less than or equal to 100. If omitted, 20 will be assumed.

Example Values: 5

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

include_entities optional

The tweet entities node will be disincluded when set to false.

Example Values: false

include_user_entities optional

The user entities node will be disincluded when set to false.

Example Values: false


*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = RETWEETS_OF_ME;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	
	add_count(api, &uri, count);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_trim_user(api, &uri, trim_user);
	add_include_entities(api, &uri, include_entities);
	add_include_user_entities(api, &uri, include_user_entities);

	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_retweets_id (
	id_t id, //required
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweets/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

count optional

Specifies the number of records to retrieve. Must be less than or equal to 100.

Example Values: 5

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = RETWEETS_ID;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%lld.json", id);
	alloc_strcat(&uri, i);
	
	count%=101;
	add_count(api, &uri, count);
	add_trim_user(api, &uri, trim_user);

	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_show (
	id_t id, //required
	char **res, //response
	int trim_user, //optional. if not -1, add it to argument.
	int include_my_retweet, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/show.json
Parameters
id required

The numerical ID of the desired Tweet.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

include_my_retweet optional

When set to either true, t or 1, any Tweets returned that have been retweeted by the authenticating user will include an additional current_user_retweet node, containing the ID of the source status for the retweet.

Example Values: true

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = SHOW;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_id(api, &uri, id);
	add_trim_user(api, &uri, trim_user);
	add_include_my_retweet(api, &uri, include_my_retweet);
	add_include_entities(api, &uri, include_entities);
	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int post_destroy_id (
	id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/destroy/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = DESTROY_ID;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%lld.json", id);
	alloc_strcat(&uri, i);
	
	add_trim_user(api, &uri, trim_user);

	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, &post, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, post, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}


int post_update(
	char *status, //required
	char **res, // response
	id_t in_reply_to_status_id, //optional. if not 0, add it to argument.
	int do_add_l_l, //add it. whether add l_l to argument.
	struct GEOCODE l_l, //optional. if it is valid figure, add it to argument.
	id_t place_id, //optional. if not 0, add it to argument.
	int display_coordinates, //optional. if not -1, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	)
{
/*

Resource URL

https://api.twitter.com/1.1/statuses/update.json
Parameters
status required


The text of your status update, typically up to 140 characters. URL encode as necessary. t.co link wrapping may effect character counts.


There are some special commands in this field to be aware of. For instance, preceding a message with "D " or "M " and following it with a screen name can create a direct message to that user if the relationship allows for it.

in_reply_to_status_id optional


The ID of an existing status that the update is in reply to.


Note:: This parameter will be ignored unless the author of the tweet this parameter references is mentioned within the status text. Therefore, you must include @username, where username is the author of the referenced tweet, within the update.


lat optional

The latitude of the location this tweet refers to. This parameter will be ignored unless it is inside the range -90.0 to +90.0 (North is positive) inclusive. It will also be ignored if there isn't a corresponding long parameter.


Example Values: 37.7821120598956


long optional

The longitude of the location this tweet refers to. The valid ranges for longitude is -180.0 to +180.0 (East is positive) inclusive. This parameter will be ignored if outside that range, if it is not a number, if geo_enabled is disabled, or if there not a corresponding lat parameter.


Example Values: -122.400612831116


place_id optional

A place in the world. These IDs can be retrieved from GET geo/reverse_geocode.


Example Values: df51dec6f4ee2b2c


display_coordinates optional

Whether or not to put a pin on the exact coordinates a tweet has been sent from.


Example Values: true


trim_user optional


When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true


*/

	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!status[0]) {
		fprintf(stderr, "need status text\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = UPDATE;
	alloc_strcat(&uri, api_uri_1_1);
	alloc_strcat(&uri, api_uri[api]);
	
	add_status(api, &uri, status);
	add_in_reply_to_status_id(api, &uri, in_reply_to_status_id);
	if (do_add_l_l) {
		add_coods(api, &uri, l_l);
	}
	add_place_id(api, &uri, place_id);
	add_display_coordinates(api, &uri, display_coordinates);
	add_trim_user(api, &uri, trim_user);


	char *post = NULL;
	char *request = oauth_sign_url2(uri, &post, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, post, res);
	
	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;

	return ret;
}

int post_retweet_id (
	id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweet/:id.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 123

trim_user optional

When set to either true, t or 1, each tweet returned in a timeline will include a user object including only the status authors numerical ID. Omit this parameter to receive the complete user object.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = RETWEET_ID;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	char i[32] = {0};
	snprintf(i, sizeof(i), "%lld.json", id);
	alloc_strcat(&uri, i);
	
	add_trim_user(api, &uri, trim_user);

	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, &post, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, post, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_oembed (
	id_t id, //required. It is not necessary to include both.
	char *url, //required. It is not necessary to include both.
	char **res, //response
	int maxwidth, //optional? It must be between 250 and 550.
	int hide_media, //optional? If not -1, add it to argument.
	int hide_thread, //optional? If not -1, add it to argument.
	int omit_script, //optional? If not -1, add it to argument.
	enum ALIGN align, //optional? If not NONE, add it to argument.
	char *related, //optional? If it is valid, add it to argument.
	char *lang //optional? If it is valid, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/oembed.json
Parameters

Either the id or url parameters must be specified in a request. It is not necessary to include both.
id required

The Tweet/status ID to return embed code for.

Example Values: 99530515043983360

url required

The URL of the Tweet/status to be embedded.

Example Values:

To embed the Tweet at https://twitter.com/#!/twitter/status/99530515043983360, use:

https%3A%2F%2Ftwitter.com%2F%23!%2Ftwitter%2Fstatus%2F99530515043983360

To embed the Tweet at https://twitter.com/twitter/status/99530515043983360, use:

https%3A%2F%2Ftwitter.com%2Ftwitter%2Fstatus%2F99530515043983360

maxwidth

The maximum width in pixels that the embed should be rendered at. This value is constrained to be between 250 and 550 pixels.

Note that Twitter does not support the oEmbed maxheight parameter. Tweets are fundamentally text, and are therefore of unpredictable height that cannot be scaled like an image or video. Relatedly, the oEmbed response will not provide a value for height. Implementations that need consistent heights for Tweets should refer to the hide_thread and hide_media parameters below.

Example Values: 325

hide_media

Specifies whether the embedded Tweet should automatically expand images which were uploaded via POST statuses/update_with_media. When set to either true, t or 1 images will not be expanded. Defaults to false.

Example Values: true

hide_thread

Specifies whether the embedded Tweet should automatically show the original message in the case that the embedded Tweet is a reply. When set to either true, t or 1 the original Tweet will not be shown. Defaults to false.

Example Values: true

omit_script

Specifies whether the embedded Tweet HTML should include a <script> element pointing to widgets.js. In cases where a page already includes widgets.js, setting this value to true will prevent a redundant script element from being included. When set to either true, t or 1 the <script> element will not be included in the embed HTML, meaning that pages must include a reference to widgets.js manually. Defaults to false.

Example Values: true

align

Specifies whether the embedded Tweet should be left aligned, right aligned, or centered in the page. Valid values are left, right, center, and none. Defaults to none, meaning no alignment styles are specified for the Tweet.

Example Values: center

related

A value for the TWT related parameter, as described in Web Intents. This value will be forwarded to all Web Intents calls.

Example Values:

twitterapi,twittermedia,twitter

lang

Language code for the rendered embed. This will affect the text and localization of the rendered HTML.

Example Values: fr

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!id || !url || !(*url)) {
		fprintf(stderr, "need id number or url text.\n");
		return 0;
	}
	
	
	char *uri = NULL;
	enum APIS api = OEMBED;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_id(api, &uri, id);
	add_url(api, &uri, url);
	add_maxwidth(api, &uri, maxwidth);
	add_hide_media(api, &uri, hide_media);
	add_hide_thread(api, &uri, hide_thread);
	add_omit_script(api, &uri, omit_script);
	add_align(api, &uri, align);
	add_related(api, &uri, related);
	add_lang(api, &uri, lang);

	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_retweeters_ids (
	id_t id, //required
	char **res, //response
	int cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/statuses/retweeters/ids.json
Parameters
id required

The numerical ID of the desired status.

Example Values: 327473909412814850

cursor semi-optional

Causes the list of IDs to be broken into pages of no more than 100 IDs at a time. The number of IDs returned is not guaranteed to be 100 as suspended users are filtered out after connections are queried. If no cursor is provided, a value of -1 will be assumed, which is the first "page."

The response from the API will include a previous_cursor and next_cursor to allow paging back and forth. See Using cursors to navigate collections for more information.

While this method supports the cursor parameter, the entire result set can be returned in a single cursored collection. Using the count parameter with this method will not provide segmented cursors for use with this parameter.

Example Values: 12893764510938

stringify_ids optional

Many programming environments will not consume our ids due to their size. Provide this option to have ids returned as strings instead. Read more about Twitter IDs, JSON and Snowflake.

Example Values: true

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!id) {
		fprintf(stderr, "need id number\n");
		return 0;
	}
	
	char *uri = NULL;
	enum APIS api = RETWEETERS_IDS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);
	
	add_id(api, &uri, id);
	add_cursor(api, &uri, cursor);
	add_stringify_ids(api, &uri, stringify_ids);

	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}

int get_tweets (
	char *q, //required
	char **res, //response
	struct GEOCODE geocode, //optional. If it is valid, add it to argument.
	char *lang, //optional. If not 0, add it to argument.
	char *locale, //optional. If not 0, add it to argument. Only ja is currently effective
	int result_type, //optional. If not 0, add it to argument. 1 = "mixed",2="recent",4="popular"
	int count, //optional. If not 0, add it to argument.
	char *until, //optional. If not 0, add it to argument.
	id_t since_id, //optional. If not 0, add it to argument.
	id_t max_id, //optional. If not 0, add it to argument.
	int include_entities, //optional. If not -1, add it to argument.
	char *callback //optional. If not 0, add it to argument.
	) {
/*

Resource URL
https://api.twitter.com/1.1/search/tweets.json
Parameters
q required

A UTF-8, URL-encoded search query of 1,000 characters maximum, including operators. Queries may additionally be limited by complexity.

Example Values: @noradio

geocode optional

Returns tweets by users located within a given radius of the given latitude/longitude. The location is preferentially taking from the Geotagging API, but will fall back to their Twitter profile. The parameter value is specified by "latitude,longitude,radius", where radius units must be specified as either "mi" (miles) or "km" (kilometers). Note that you cannot use the near operator via the API to geocode arbitrary locations; however you can use this geocode parameter to search near geocodes directly. A maximum of 1,000 distinct "sub-regions" will be considered when using the radius modifier.

Example Values: 37.781157,-122.398720,1mi

lang optional

Restricts tweets to the given language, given by an ISO 639-1 code. Language detection is best-effort.

Example Values: eu

locale optional

Specify the language of the query you are sending (only ja is currently effective). This is intended for language-specific consumers and the default should work in the majority of cases.

Example Values: ja

result_type optional

Optional. Specifies what type of search results you would prefer to receive. The current default is "mixed." Valid values include:
  * mixed: Include both popular and real time results in the response.
  * recent: return only the most recent results in the response
  * popular: return only the most popular results in the response.

Example Values: mixed, recent, popular

count optional

The number of tweets to return per page, up to a maximum of 100. Defaults to 15. This was formerly the "rpp" parameter in the old Search API.

Example Values: 100

until optional

Returns tweets generated before the given date. Date should be formatted as YYYY-MM-DD. Keep in mind that the search index may not go back as far as the date you specify here.

Example Values: 2012-09-01

since_id optional

Returns results with an ID greater than (that is, more recent than) the specified ID. There are limits to the number of Tweets which can be accessed through the API. If the limit of Tweets has occured since the since_id, the since_id will be forced to the oldest ID available.

Example Values: 12345

max_id optional

Returns results with an ID less than (that is, older than) or equal to the specified ID.

Example Values: 54321

include_entities optional

The entities node will be disincluded when set to false.

Example Values: false

callback optional

If supplied, the response will use the JSONP format with a callback of the given name. The usefulness of this parameter is somewhat diminished by the requirement of authentication for requests to this endpoint.

Example Values: processTweets

*/
	#ifdef DEBUG
	puts(__func__);
	#endif
	
	if (!check_keys()) {
		fprintf(stderr, "need init_keys()\n");
		return 0;
	}
	
	if (!q || !(*q)) {
		fprintf(stderr, "need q text\n");
		return 0;
	}
	
	if (strlen(q) > 1000) {
		fprintf(stderr, "too long q text\n");
		return 0;
	}
	char *uri = NULL;
	enum APIS api = TWEETS;
	alloc_strcat(&uri, api_uri_1_1); 
	alloc_strcat(&uri, api_uri[api]);

	add_q(api, &uri, q);
	add_geocode(api, &uri, geocode);
	add_lang(api, &uri, lang);
	add_locale(api, &uri, locale);
	add_result_type(api, &uri, result_type);
	add_count(api, &uri, count);
	add_until(api, &uri, until);
	add_since_id(api, &uri, since_id);
	add_max_id(api, &uri, max_id);
	add_include_entities(api, &uri, include_entities);
	add_callback(api, &uri, callback);


	
	char *post = NULL;
	char *request = oauth_sign_url2(uri, NULL, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	int ret = http_request(request, NULL, res);


	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;


	return ret;
}
