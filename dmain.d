import tweet;
import keys.mykey;

import std.stdio;
import std.conv;
import std.string;
import std.c.string;
import std.c.stdlib;
import std.json;

char[] ctod(const char* str){
	return str[0..strlen(str)].dup;
}

//curl functions
//I think they should be moved and hidden in the library like bear_init()/bear_cleanup()
//return type CURLcode is enum, so actually it is int
extern(C) int curl_global_init(int);
extern(C) void curl_global_cleanup();
enum CURL_GLOBAL_DEFAULT = (1<<0) | (1<<1);

void main()
{
	char *rep = null;
	KEYS key;
	key.c_key = c_key;
	key.c_sec = c_sec;
	key.t_key = t_key;
	key.t_sec = t_sec;
	register_keys(&key);
	curl_global_init(CURL_GLOBAL_DEFAULT);
	char[] unit = "\0".dup; 
	GEOCODE code = {0,0,0,unit.ptr};
	"Tweet.".writeln;
	char[] status_text = (stdin.readln ~ "\0").dup;
	//auto text = x"E3 81 82 E3 81 84 E3 81 86 E3 81 88 E3 81 8A 0A" ~ '\0';
	auto status = post_statuses_update(status_text.ptr, &rep, 0, 0, code, 0, -1, -1);
	//if(rep) {rep.ctod.writeln;}
	auto timeline = get_statuses_home_timeline(&rep, 5, 0, 0, -1, -1, -1, -1);
	//if (rep){rep.ctod.writeln;}
	auto json = parseJSON(rep.ctod);
	json.writeln;
	free(rep);
	rep = null;
	curl_global_cleanup();
}

