all:
	rm -R builds
	mkdir builds
	cd builds
	
	mkdir libcuckoo
	cd libcuckoo
	cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=LIBCUCKOO ../..
	make all
	cd ..
	
	mkdir dhash
	cd dhash
	cmake -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_UNIVERSAL_BENCHMARK=1 -DUNIVERSAL_TABLE=DHASH ../..
	make all
	cd ..
	
	cd ..
