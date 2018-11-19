# Student's Makefile for the CS:APP Performance Lab
CC = gcc
CFLAGS = -Wall -g3 -O2 -m64
LIBS = -lm
TURNIN = submission.tgz

OBJS = driver.o kernels.o fcyc.o clock.o

all: driver

driver: $(OBJS) fcyc.h clock.h defs.h config.h
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o driver

handin:
	cp kernels.c $(HANDINDIR)/$(TEAM)-$(VERSION)-kernels.c

clean: 
	-rm -f $(OBJS) driver core *~ *.o


submit:
	@rm -f $(TURNIN)
	@cp kernels.c $(USER)-kernels.c
	@tar czf $(TURNIN) $(USER)-kernels.c
	@~jloew/CSE202/submitPerfLab.pl
	@rm -f $(TURNIN) $(USER)-kernels.c
