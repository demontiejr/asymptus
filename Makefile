all:
	make -C util
	make -C ComplexityInference
	make -C RangeAnalysis

clean:
	cd ComplexityInference && make clean && cd ..
	cd util && make clean && cd ..
	make -C RangeAnalysis clean
