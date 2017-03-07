import numpy as np
import sys


def get_query(elems, rangel=2000000):
	return np.random.choice(rangel, elems)


if __name__ == "__main__":
	query = get_query(int(sys.argv[1]))
	f = open('query_' + sys.argv[1], 'w')
	f.write(sys.argv[1] + '\n')
	for idee in query:
		f.write(str(idee) + '\n')
	f.close()
