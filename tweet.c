#include <errno.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <math.h>

#include <oauth.h>
#include "webclient.h"
#include "tweet.h"


static int http_request(char const *u, char const *p, char **reply)
{
	#ifdef DEBUG
	puts(__func__);
	#endif

	if (*reply) {
		memset(*reply,0,strlen(*reply));
	}
	struct URI uri = init_uri(u);
	struct RESULT res = {NULL,NULL};
	
	printf("u = %s\nscheme = %s\nhost = %s\npath = %s\nport = %d",u,uri.scheme,uri.host,uri.path,uri.port);

	send_req(&uri, p, &res);	
	
	if (strlen(res.headers) > 0) {

		int a, b, c;
		sscanf(res.headers, "HTTP/%d.%d %d", &a, &b, &c);
		free(res.headers);
		res.headers = NULL;
		if (reply && res.content && res.content[0]) {
			*reply = res.content;
		}
		if (c == 200) {
			return 1;
		}
	}
	return 0;
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
[UPDATE] = "statuses/update.json",
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
		fprintf(stderr, "need user_id or screen_name\n");
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


int post_update(
	char *status, //required
	char **res, // response
	id_t in_reply_to_status_id, //optional. if not 0, add it to argument.
	int do_add_l_l, //add it. whether add l_l to argument.
	union L_L l_l, //optional. if it is valid figure, add it to argument.
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
	
	char *escaped_msg = oauth_url_escape(status);
	alloc_strcat(&uri, "?status=");
	alloc_strcat(&uri, escaped_msg);
	free(escaped_msg);escaped_msg = NULL;
	
	if (in_reply_to_status_id) {
		char id[32] = {0};
		alloc_strcat(&uri, "&in_reply_to_status_id=");
		snprintf(id, sizeof(id), "%llu", in_reply_to_status_id);
		alloc_strcat(&uri, id);
	}
	
	
	if (do_add_l_l && (int)(fabs(l_l.l_l.latitude)) < 90 && (int)(fabs(l_l.l_l.longitude)) < 180) {
		char latitude[32];
		char longitude[32];
		alloc_strcat(&uri, "&latitude=");
		snprintf(latitude, sizeof(latitude), "%2.12f", l_l.l_l.latitude);
		alloc_strcat(&uri, latitude);
		alloc_strcat(&uri, "&longitude=");
		snprintf(longitude, sizeof(longitude), "%2.12f", l_l.l_l.longitude);
		alloc_strcat(&uri, longitude);
	}

	if (place_id) {
		char id[32] = {0};
		alloc_strcat(&uri, "&place_id=");
		snprintf(id, sizeof(id), "%llx", place_id);
		alloc_strcat(&uri, id);
	}

	if (display_coordinates != -1) {
		char boolian[2];
		alloc_strcat(&uri, "&display_coordinates=");
		snprintf(boolian, sizeof(boolian), "%d", !!display_coordinates);
		alloc_strcat(&uri, boolian);
	}

	if (trim_user != -1) {
		char boolian[2];
		alloc_strcat(&uri, "&trim_user=");
		snprintf(boolian, sizeof(boolian), "%d", !!trim_user);
		alloc_strcat(&uri, boolian);
	}


	char *post = NULL;
	char *request = oauth_sign_url2(uri, &post, OA_HMAC, NULL, keys.keys_struct.c_key, keys.keys_struct.c_sec, keys.keys_struct.t_key, keys.keys_struct.t_sec);
	printf("qwqwqwqw\nuri = %s\nrequest = %s\n",uri,request);
	int ret = http_request(request, post, res);
	
	free(uri);uri = NULL;
	free(request);request = NULL;
	free(post);post = NULL;

	return ret;
}

