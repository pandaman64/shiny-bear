
#ifndef __TWEET_H
#define __TWEET_H

typedef unsigned long long int id_t;

enum APIS {
	MENTIONS_TIMELINE,
	USER_TIMELINE,
	HOME_TIMELINE,
	RETWEETS_OF_ME,
	RETWEETS_ID,
	SHOW,
	DESTROY_ID,
	UPDATE,
	RETWEET_ID,
	OEMBED,
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

#define LAT 0
#define LONG 1

union COOD {
	struct {
		double latitude;
		double longitude;
	} l_l;
	double cood[2];
};

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

int get_retweets_id (
	id_t id, //required
	char **res, //response
	int count, //optional. if not 0, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	);

int get_show (
	id_t id, //required
	char **res, //response
	int trim_user, //optional. if not -1, add it to argument.
	int include_my_retweet, //optional. if not -1, add it to argument.
	int include_entities //optional. if not -1, add it to argument.
	);
	
int post_destroy_id (
	id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	);
	
int post_update(
	char *update, //required
	char **res, // response
	id_t in_reply_to_status_id, //optional. if not 0, add it to argument.
	int do_add_l_l, //add it. whether add l_l to argument.
	union COOD l_l, //optional. if it is valid figure, add it to argument.
	id_t place_id, //optional. if not 0, add it to argument.
	int display_coordinates, //optional. if not -1, add it to argument.
	int trim_user //optional. if not -1, add it to argument.
	);

int post_retweet_id (
	id_t id, //required
	char **res, //response
	int trim_user //optional. if not -1, add it to argument.
	);
	
//POST statuses/update_with_media is too difficult to implement
#endif
