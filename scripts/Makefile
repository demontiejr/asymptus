CC = g++

RM = rm -f

ASS = asymptus.cpp
CPA = cpa.cc


all: asymptus cpa

asymptus: $(ASS)
	@echo " --- Compiling Asymptus --- "
	@$(CC)  $< -o asymptus

cpa: $(CPA)
	@echo " --- Compiling CPA --- "
	@$(CC)  $< -o cpa


clean:
	$(RM) asymptus cpa
	clear

install:
	@echo " --- Installing Asymptus files --- "
	@sudo mkdir -p /usr/bin/asymptusProfiler/
	@sudo mv asymptus cpa /usr/bin/asymptusProfiler/
	@sudo cp  /usr/bin/asymptusProfiler/asymptus /usr/bin/
	@sudo cp  gen_csv.py parse_complexity.py  parse_file.py /usr/bin/asymptusProfiler/

uninstall:
	@echo " --- Removing Asymptus files --- "
	@sudo rm -f /usr/bin/asymptusProfiler/*
	@sudo rmdir /usr/bin/asymptusProfiler/
	@sudo rm /usr/bin/asymptus
