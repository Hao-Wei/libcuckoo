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
		if(s[:5] == 'FATAL'):
			print('Failed to parse ' + f)
			continue
		s = eval(s)
		table = s['table']
		throughput = s['output']['throughput']
		runtime = s['output']['time_elapsed']
		total_ops = s['output']['total_ops']
		
		ss = s['args'].split(' ')
		args = {}
		for i in range(0, len(ss), 2):
			args[ss[i][2:]] = ss[i+1]
		
		test = args['reads'] + '-' + args['inserts'] + '-' + args['upserts']
		if test not in d:
			d[test] = {}
		if table not in d[test]:
			d[test][table] = {}
		d[test][table][int(args['num-threads'])] = {'throughput': throughput, 'runtime': runtime, 'total_ops': total_ops}
	
	colors = {'FOLKLORE': 'blue', 'HOPSCOTCH': 'grey', 'LIBCUCKOO': 'red', 'NDHASH': 'green', 'DHASH': 'lightgreen', 'PARALLELDP': 'pink'}
	test_dict = {'0-100-0': 'All inserts', '100-0-0': 'All reads', '90-10-0': '90% Read, 10% Insert'}
	
	for test in d:
		print test
		ax = plt.subplot(111)
		for table in d[test]:
			dd = d[test][table]
			x = dd.keys()
			x.sort()
			print x
			y = []
			for t in x:
				y.append(int(dd[t]['throughput']['value']))
			ax.plot(x, y,'-o', color=colors[table], linewidth=2, label=table)
			
		plt.title('Throughput vs. Number of cores (' + test_dict[test] + ')')
		plt.xlabel('Number of cores')
		plt.ylabel('Operations per second')
		chartBox = ax.get_position()
		ax.set_position([chartBox.x0, chartBox.y0, chartBox.width*0.95, chartBox.height])
		ax.legend(bbox_to_anchor=(0.4, 0.92), shadow=True, ncol=1)
		plt.axis([0, 150, 0, 3*10**8])
		plt.show()
	
		
	
	
	
