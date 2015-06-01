LEVEL=../../../../..

all:
	make -C DepGraph LEVEL=$(LEVEL)
	make -C ComplexityInference LEVEL=$(LEVEL)
	make -C RangeAnalysis LEVEL=$(LEVEL)

clean:
	make -C ComplexityInference clean LEVEL=$(LEVEL)
	make -C DepGraph clean LEVEL=$(LEVEL)
	make -C RangeAnalysis clean LEVEL=$(LEVEL)
