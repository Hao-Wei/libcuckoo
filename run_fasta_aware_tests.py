
import os
import subprocess
import multiprocessing
import math
from time import gmtime, strftime

timestamp=""
# datafile = "tests/fasta-kmer-benchmark/data/Meleagris_gallopavo.UMD2.dna.chromosome.17.fa"
datafile = "tests/fasta-kmer-benchmark/data/Homo_sapiens.GRCh38.dna.chromosome.22.fa"

def runTest(table, thread):
	thread = str(thread)
	
	program = "./fasta_builds/" + table + "/tests/fasta-kmer-benchmark/kmer_benchmark"
	output_file = "fasta_results/"+timestamp+"/"+table+"_"+thread+"_threads.txt"
	params = program + " " + str(thread) + " " + datafile + " > " + output_file
	
	print(params)
	
	os.system(params)


if __name__ == '__main__':
	timestamp = strftime("%Y-%m-%d_%H:%M:%S", gmtime())
	if(not os.path.isdir("fasta_results")):
		os.mkdir("fasta_results")
	os.mkdir("fasta_results/"+timestamp)
	
	n_threads = multiprocessing.cpu_count()/2
	n_points = 5
	# tables = ["folklore", "hopscotch", "libcuckoo", "ndhash", "paralleldp"]
	tables = ["folklore", "hopscotch", "libcuckoo", "ndhash"]
	threads = [int(float(i)/n_points*(n_threads-1))+1 for i in range(n_points+1)]
	threads.append(multiprocessing.cpu_count())
	print threads
	
	for table in tables:
		for thread in threads:
			runTest(table, thread)
			
#		os.chdir("builds/" + table)
#		for filename in os.listdir("."):
#			print filename
	
#		os.chdir("../..")
