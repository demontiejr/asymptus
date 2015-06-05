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
