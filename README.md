shiny-bear
==========

multi-platform twitter API library written in C.
Based on [twiapps](http://www.soramimi.jp/twicpps/index.html).

add your keys
---

You must add your OAuth keys to ./keys/mykey.h.
You can change the filename. if you do, change include filename.

```
#include "your-file.h"
```

compile
---

Before compilation, you need gcc, make, liboauth and libcurl.

```sh
$ make
```

That's all.

APIs
---

Currently this library has wrappers for following twitter REST APIs:

**Timelines**
- [statuses/mentions_timeline](https://dev.twitter.com/docs/api/1.1/get/statuses/mentions_timeline)
- [statuses/user_timeline](https://dev.twitter.com/docs/api/1.1/get/statuses/user_timeline)
- [statuses/home_timeline](https://dev.twitter.com/docs/api/1.1/get/statuses/home_timeline)
- [statuses/retweets_of_me](https://dev.twitter.com/docs/api/1.1/get/statuses/retweets_of_me)

**Tweets**
- [statuses/retweets/:id](https://dev.twitter.com/docs/api/1.1/get/statuses/retweets/%3Aid)
- [statuses/show/:id](https://dev.twitter.com/docs/api/1.1/get/statuses/show/%3Aid)
- [statuses/destroy/:id](https://dev.twitter.com/docs/api/1.1/post/statuses/destroy/%3Aid)
- [statuses/update](https://dev.twitter.com/docs/api/1.1/post/statuses/update)
- [statuses/retweet/:id](statuses/retweet/:id)
- ~~[statuses/update_with_media](https://dev.twitter.com/docs/api/1.1/post/statuses/update_with_media)~~
- [statuses/oembed](https://dev.twitter.com/docs/api/1.1/get/statuses/oembed)
- [statuses/retweeters/ids](statuses/retweeters/ids)

**Search**
- [search/tweets](https://dev.twitter.com/docs/api/1.1/get/search/tweets)
