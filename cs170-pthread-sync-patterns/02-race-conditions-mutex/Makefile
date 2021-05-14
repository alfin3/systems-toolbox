CC = gcc
UTILS_MEM_DIR  = ../../utilities/utilities-mem/
UTILS_PTHD_DIR = ../../utilities/utilities-pthread/
CFLAGS = -I$(UTILS_MEM_DIR)                            \
         -I$(UTILS_PTHD_DIR)                           \
         -std=gnu90 -pthread -Wpedantic -Wall -Wextra

EXE = race-cond-mutex \
      race-cond1      \
      race-cond2

SHARED_OBJ = $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_PTHD_DIR)utilities-pthread.o

NSHARED_OBJ = race-cond-mutex.o \
              race-cond1.o      \
              race-cond2.o

all                   : $(EXE)
race-cond-mutex : race-cond-mutex.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
race-cond1      : race-cond1.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
race-cond2      : race-cond2.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

race-cond-mutex.o                    : $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
race-cond1.o                         : $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
race-cond2.o                         : $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
$(UTILS_MEM_DIR)utilities-mem.o      : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_PTHD_DIR)utilities-pthread.o : $(UTILS_PTHD_DIR)utilities-pthread.h

.PHONY : clean clean-all

clean :
	rm $(SHARED_OBJ) $(NSHARED_OBJ)
clean-all :
	rm -f $(EXE) $(SHARED_OBJ) $(NSHARED_OBJ)
