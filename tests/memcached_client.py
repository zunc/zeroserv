# memcached client test for memcached handler

import sys
import memcache
import time

NUM_STEP = 100000
mcrouter = memcache.Client(["127.0.0.1:" + sys.argv[1]])

def regionTest():
	for i in xrange(NUM_STEP):
		key = "key" + str(i)
		mcrouter.set(key, "value" + str(i))
		print "set: " + key

	def countCacheValue(mc_object):
		cnt = 0
		for i in xrange(NUM_STEP):
			key = "key" + str(i)
			value = mc_object.get(key)
			if (value is not None):
				cnt += 1
				print("> [" + key + "] -> " + value)
		print "> hit = " + str(cnt)
	countCacheValue(mcrouter)

start = time.clock()
regionTest()
end = time.clock()
print "elapsed time: " + str(end - start) + "s"
