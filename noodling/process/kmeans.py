#!/usr/bin/env python
#this little one linner preprocesses the data as dumped by "wmdump" into something that python can diggest
#for i in ../training-data-capture-20090610/*; do awk -F '[=, ]' 'BEGIN { print "data = [" } /Acc/ { printf "[%s, %s, %s],\n", $4, $7, $10 } END { print "]" }' "$i"; done  > data.py

import numpy
from scipy.cluster.vq import kmeans2
from data import data
points, res = kmeans2(numpy.array(data, dtype=float), 3)
print 'points: %s\n\n\nresults: %s' % (points, res)
