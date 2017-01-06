def endsWith(str1,str2):
	if (str1[len(str1)-len(str2):len(str1)] == str2):
		return 1
	else:
		return 0

# Returns one library from a list of libraries.

def filterLib(fileNodes,libName):
	node =  filter (lambda x: endsWith(str(x),libName),fileNodes)[0]
	return node
