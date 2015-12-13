#!/usr/bin/python2
import itertools
import random

url_base="http://s3-eu-west-1.amazonaws.com/puzzleinabucket/"
url_extn = ".html"

urls = []

for i in itertools.product('ABCDEF', repeat=6):
    prod = ''.join(i)
    s = ''.join([url_base, prod, url_extn])
    urls.append(s)

random.shuffle(urls)
for u in urls:
    print u
