all:
	make -C util
	make -C ComplexityInference

clean:
	cd ComplexityInference && make clean && cd ..
	cd util && make clean && cd ..