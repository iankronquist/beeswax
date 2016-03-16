go-hpfeeds
==========

Basic Go client implementation of [hpfeeds](https://github.com/rep/hpfeeds), a simplistic
publish/subscribe protocol, written by Mark Schloesser ([rep](https://github.com/rep/)),
heavily used within the [Honeynet Project](https://honeynet.org/) for internal real-time
data sharing. Backend component of [Honeymap](https://github.com/fw42/honeymap) and
[hpfriends](http://hpfriends.honeycloud.net).

Usage
-----
See example and ```go doc```.

License
-------
BSD

TODO
----
* Test if everything actually works as intended, maybe write some unit tests 
* Implement wrapper for JSON channels
* Add some sanity checks for message field length values, in case server sends incorrect stuff
