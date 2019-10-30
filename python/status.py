#! /usr/bin/python

import sys
from relay_lib_seeed import *

if len(sys.argv) > 1:
	try:
		if int(sys.argv[1]) != 0:
			port = int(sys.argv[1])
			if port > 0 and port < 5:
				if relay_get_port_status(port):
					print("Active")
				else:
					print("Inactive")
	except ValueError:
		pass
