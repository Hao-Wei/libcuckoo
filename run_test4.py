
import os
import subprocess
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
	output_file = "results/test4/"+timestamp+"/"+table+"_"+thread+"_threads_"+reads+"_"+inserts+"_"+upserts
	params = program + " --reads " + reads + " --inserts " + inserts + " --upserts " + upserts + " --initial-capacity " + initial_capacity + " --prefill " + prefill + " --total-ops " + total_ops + " --num-threads " + thread + " > " + output_file
	
	print(params)
	
	os.system(params)


if __name__ == '__main__':
	timestamp = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
	if(not os.path.isdir("results")):
		os.mkdir("results")
	if(not os.path.isdir("results/test4")):
		os.mkdir("results/test4")
	os.mkdir("results/test4/"+timestamp)
	
	n_threads = multiprocessing.cpu_count()/2
	n_points = 10
	tables = ["empty", "folklore", "libcuckoo", "ndhash", "random", "paralleldp", "ndquad", "hopscotch"]
	threads = []
	threads.append(multiprocessing.cpu_count())
	print threads
	
	for table in tables:
		for thread in threads:
			runTest(table, thread, total_ops=100, initial_capacity=25, prefill=75, reads=100)
			runTest(table, thread, total_ops=50, initial_capacity=25, prefill=25, inserts=100)
			
#		os.chdir("builds/" + table)
#		for filename in os.listdir("."):
#			print filename
	
#		os.chdir("../..")
