from pprint import pprint

if __name__ == '__main__':
	s = open('results/2017-12-02_17:56:11/folklore_1_threads_100_0_0', 'r').read()
	s = eval(s)
	pprint(s)
	
