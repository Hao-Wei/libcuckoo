
import os
import multiprocessing
import math
from time import gmtime, strftime

timestamp=""

def runTest(table, thread, total_ops=100, initial_capacity=23, prefill=25, reads=0, inserts=0, upserts=0):
	thread = str(thread)
	total_ops = str(total_ops)
	initial_capacity = str(initial_capacity)
	prefill = str(prefill)
	reads = str(reads)
	inserts = str(inserts)
	upserts = str(upserts)
	
	program = "./builds/" + table + "/tests/universal-benchmark/universal_benchmark"
	params = "--reads " + reads + " --inserts " + inserts + " --upserts " + upserts + " --initial-capacity " + initial_capacity + " --prefill " + prefill + " --total-ops " + total_ops + " --num-threads " + thread
	output_file = "results/"+timestamp+"/"+table+"_"+thread+"_threads_"+reads+"_"+inserts+"_"+upserts
	
	return program + " " + params +" > " + output_file


if __name__ == '__main__':
	timestamp = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
	if(not os.path.isdir("results")):
		os.mkdir("results")
	os.mkdir("results/"+timestamp)
	
	n_threads = multiprocessing.cpu_count()
	n_points = 2
	tables = ["folklore", "hopscotch", "libcuckoo", "ndhash"]
	threads = [int(float(i)/n_points*(n_threads-1))+1 for i in range(n_points+1)]
	threads.append(multiprocessing.cpu_count()*2)

	for table in tables:
		for thread in threads:
			os.system(runTest(table, thread, total_ops=100, initial_capacity=23, prefill=25, reads=100))
			
#		os.chdir("builds/" + table)
#		for filename in os.listdir("."):
#			print filename
	
#		os.chdir("../..")
