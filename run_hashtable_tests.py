
import os
import subprocess
import multiprocessing
import math
from time import gmtime, strftime

timestamp=""

#Tests to run
# random and random2 for values between 72 and 144
# paralleldp, ndquad, hopscotch (hash table and fasta test)
# count cache misses for all of them (including empty and random) on 72 cores (both insert and read)
# test on a single socket 

def runTest(table, thread, total_ops=100, initial_capacity=23, prefill=25, reads=0, inserts=0, upserts=0):
	thread = str(thread)
	total_ops = str(total_ops)
	initial_capacity = str(initial_capacity)
	prefill = str(prefill)
	reads = str(reads)
	inserts = str(inserts)
	upserts = str(upserts)
	
	program = "./builds/" + table + "/tests/universal-benchmark/universal_benchmark"
	output_file = "results-4/"+timestamp+"/"+table+"_"+thread+"_threads_"+reads+"_"+inserts+"_"+upserts
	params = program + " --reads " + reads + " --inserts " + inserts + " --upserts " + upserts + " --initial-capacity " + initial_capacity + " --prefill " + prefill + " --total-ops " + total_ops + " --num-threads " + thread + " > " + output_file
	
	print(params)
	
	os.system(params)


if __name__ == '__main__':
	timestamp = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
	if(not os.path.isdir("results-4")):
		os.mkdir("results-4")
	os.mkdir("results-4/"+timestamp)
	
	n_threads = multiprocessing.cpu_count()
	n_points = 3
	tables = ["folklore", "libcuckoo", "ndhash", "random", "paralleldp", "ndquad", "hopscotch"]
	threads = [int(float(i)/n_points*(n_threads-1))+1 for i in range(n_points+1)]
	print threads
	
	for table in tables:
		for thread in threads:
			runTest(table, thread, total_ops=100, initial_capacity=24, prefill=75, reads=100)
			runTest(table, thread, total_ops=50, initial_capacity=24, prefill=25, inserts=100)
			runTest(table, thread, total_ops=100, initial_capacity=24, prefill=50, reads = 90, inserts=10)
			
#		os.chdir("builds/" + table)
#		for filename in os.listdir("."):
#			print filename
	
#		os.chdir("../..")
