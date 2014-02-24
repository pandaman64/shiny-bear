
#ifndef __TWEET_H
#define __TWEET_H

#ifdef __CYGWIN__
	#ifdef __id_t_defined
		#define id_t tweet_id_t
	#endif
#endif

typedef unsigned long long int id_t;

enum APIS {
	MENTIONS_TIMELINE,
	USER_TIMELINE,
	HOME_TIMELINE,
	RETWEETS_OF_ME,
	RETWEETS_BY_ID,
	SHOW_BY_ID,
	DESTROY_BY_ID,
	UPDATE,
	RETWEET_BY_ID,
	OEMBED,
	RETWEETERS_IDS,
	TWEETS,
	DIRECT_MESSAGES,
	DM_SENT,
	DM_SHOW,
	DM_DESTROY,
	DM_NEW,
	NUM_OF_APIS
};

enum ALIGN {
	NONE,
	LEFT,
	RIGHT,
	CENTER,
};

union KEYS {
	struct {
		char const *c_key;
		char const *c_sec;
		char const *t_key;
		char const *t_sec;
	} keys_struct;
	char const *keys_array[4];
};

union KEYS init_keys (union KEYS k);
int check_keys(void);

struct GEOCODE{
		double latitude;
		double longitude;
		int radius;
		char *unit;
};

#define MI "mi"
#define KM "km"
#define JA "ja"

#define MIXED 1
#define RECENT 2
#define POPULAR 4

int get_mentions_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_rts //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

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
	);

int get_home_timeline (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int exclude_replies, //optional. if not -1, add it to argument.
	int contributor_details, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);

int get_retweets_of_me (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int trim_user, //optional. if not -1, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int include_user_entities //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_retweets_by_id (
	id_t id, //required
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	);

int get_show_by_id (
	id_t id, //required
	char **res, //response
	int trim_user, //optional. if not -1, add it to argument.
	int include_my_retweet, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);
	
int post_destroy_by_id (
	id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	);
	
int post_update(
	char *update, //required
	char **res, // response
	id_t in_reply_to_status_id, //optional. if not 0, add it to argument.
	int do_add_l_l, //add it. whether add l_l to argument.
	struct GEOCODE l_l, //optional. if it is valid figure, add it to argument.
	id_t place_id, //optional. if not 0, add it to argument.
	int display_coordinates, //optional. if not -1, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	);

int post_retweet_by_id (
	id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	);

//POST statuses/update_with_media is too difficult to implement

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
	);
	
int get_retweeters_ids (
	id_t id, //required
	char **res, //response
	int cursor, //optional. if not 0, add it to argument.
	int stringify_ids //optional. if not -1, add it to argument.
	);

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
	);

int get_direct_messages (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int include_entities, //optional. if not -1, add it to argument.
	int skip_status //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	);

int get_dm_sent (
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	id_t since_id, //optional. if not 0, add it to argument.
	id_t max_id, //optional. if not 0, add it to argument.
	int pages, //optional. if not -1, add it to argument,however, 1 is recommended.see below.
	int include_entities //optional. if not -1, add it to argument.
	);

int get_dm_show (
	id_t id, //required
	char **res //response
	);

int post_dm_destroy (
	id_t id, //required
	char **res, //response
	int include_entities //optional. if not -1, add it to argument.
	);

int post_dm_new (
	id_t user_id, //One of user_id or screen_name are required.
	char *screen_name, //One of user_id or screen_name are required.
	char *text, //required.
	char **res //response
	);

#ifdef __CYGWIN__
	#ifdef __id_t_defined
		#undef tweet_id_t
	#endif
#endif



#endif
