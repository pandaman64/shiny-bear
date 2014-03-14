import tweet;
import keys.mykey;

import std.stdio;
import std.conv;
import std.string;
import std.c.string;
import std.c.stdlib;

char[] ctod(const char* str){
	return str[0..strlen(str)].dup;
}

//curl functions
//I think they should be moved and hidden in the library like bear_init()/bear_cleanup()
//return type CURLcode is enum, so actually it is int
int curl_global_init(int);
void curl_global_cleanup();
enum CURL_GLOBAL_DEFAULT = (1<<0) | (1<<1);

void main()
{
	char *rep;
	KEYS key;
	key.c_key = c_key;
	key.c_sec = c_sec;
	key.t_key = t_key;
	key.t_sec = t_sec;
	register_keys(key);
	curl_global_init(CURL_GLOBAL_DEFAULT);
	char[] unit = "\0".dup; 
	GEOCODE code = {0,0,0,unit.ptr};
	char[] status_text = "status text\0".dup; 
	auto status = post_statuses_update(status_text.ptr, &rep, 0, 0, code, 0, -1, -1);
	if(rep) {rep.ctod.writeln;}
	free(rep);
	auto timeline = get_statuses_home_timeline(&rep, 5, 0, 0, -1, -1, -1, -1);
	if (rep){rep.ctod.writeln;}
	free(rep);
	curl_global_cleanup();
}

