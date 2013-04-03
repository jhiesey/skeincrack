# What is this?

This was one of the (and perhaps the main) program used by Stanford students
for the xkcd hashing competition on April 1, 2013 (<http://xkcd.com/1193/>).
Even though Stanford didn't win, we were in the lead for a good part of April
first.

It is designed for the specific purpose of trying to win the xkcd competition.
It has the hash in question hardcoded, and automatically submits promising
results to almamater.xkcd.com under Stanford's name.

# Authors

I wrote an initial version of this code in the very early morning of April 1.
Over the course of the day, Justin Lebar and Reid Watson made significant
improvements and created a git repo.

# Instructions

First of all, why are you bothering, the competition's over ;)

I only know for a fact that this works on Mac OS X as it stands, although I
have heard that it works fine on Linux as well. No special libraries are
needed, although the command-line version of curl does need to be in your
PATH.

For the fastest version, run `make pgo` for a version with profile-guided
optimization. If you don't want to wait for a profile to be generated, just
run `make` instead.

Then run `./skeincrack` and wait! Every time an input better than any seen
before during this run is encountered, it will be printed and submitted.
The data sent back from the HTTP POST will be printed as well.