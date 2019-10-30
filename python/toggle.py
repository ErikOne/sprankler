#! /usr/bin/python

import sys
from relay_lib_seeed import *

if len(sys.argv) > 1:
	try:
		if int(sys.argv[1]) != 0:
			port = int(sys.argv[1])
			if port > 0 and port < 5:
				relay_toggle_port(port)
			elif port == 5:
				relay_all_on()
			elif port == 6:
				relay_all_off()
	except ValueError:
		pass
