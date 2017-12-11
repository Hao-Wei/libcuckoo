from pprint import pprint
import matplotlib.pyplot as plt
import os

if __name__ == '__main__':
	ll = os.listdir("results")
	ll.sort()
	result_dir = 'results/' + max(ll) + '/'
	graph_dir = 'graphs/' + max(ll) + '/'
	if(not os.path.isdir("graphs")):
		os.mkdir("graphs")
	if(not os.path.isdir(graph_dir)):
		os.mkdir(graph_dir)
	
	d = {}
	
	for f in os.listdir(result_dir):
		s = open(result_dir + f, 'r').read()
		print(s)
		if(s[:5] == 'FATAL'):
			print('Failed to parse ' + f)
			continue
		s = eval(s)
		table = s['table']
		throughput = s['output']['throughput']
		runtime = s['output']['time_elapsed']
		
		test = s['input_file']
		if test not in d:
			d[test] = {}
		if table not in d[test]:
			d[test][table] = {}
		d[test][table][s['threads']] = {'throughput': throughput, 'runtime': runtime,}
	
	colors = {'FOLKLORE': 'blue', 'HOPSCOTCH': 'pink', 'LIBCUCKOO': 'red', 'NDHASH': 'green', 'DHASH': 'lightgreen'}
	
	for test in d:
		for table in d[test]:
			dd = d[test][table]
			x = dd.keys()
			x.sort()
			y = []
			for t in x:
				y.append(dd[t]['throughput']['value'])
			plt.plot(x, y, color=colors[table], linewidth=3)
		
		plt.show()
	
		
	
	
	
