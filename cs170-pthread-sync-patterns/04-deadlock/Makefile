CC = gcc
UTILS_MEM_DIR  = ../../utilities/utilities-mem/
UTILS_PTHD_DIR = ../../utilities/utilities-pthread/
CFLAGS = -I$(UTILS_MEM_DIR)                               \
         -I$(UTILS_PTHD_DIR)                              \
         -std=gnu90 -pthread -Wpedantic -Wall -Wextra -O1

EXE = deadlock1 deadlock2 deadlock-free1 deadlock-free2

SHARED_OBJ = driver.o                              \
             $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_PTHD_DIR)utilities-pthread.o

NSHARED_OBJ = deadlock1.o      \
              deadlock2.o      \
              deadlock-free1.o \
              deadlock-free2.o

all       : $(EXE)
deadlock1 : deadlock1.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
deadlock2 : deadlock2.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
deadlock-free1 : deadlock-free1.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
deadlock-free2 : deadlock-free2.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

deadlock1.o                          : deadlock.h                            \
                                       $(UTILS_MEM_DIR)utilities-mem.h       \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
deadlock2.o                          : deadlock.h                            \
                                       $(UTILS_MEM_DIR)utilities-mem.h       \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
deadlock-free1.o                     : deadlock.h                            \
                                       $(UTILS_MEM_DIR)utilities-mem.h       \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
deadlock-free2.o                     : deadlock.h                            \
                                       $(UTILS_MEM_DIR)utilities-mem.h       \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
driver.o                             : deadlock.h                            \
                                       $(UTILS_MEM_DIR)utilities-mem.h       \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
$(UTILS_MEM_DIR)utilities-mem.o      : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_PTHD_DIR)utilities-pthread.o : $(UTILS_PTHD_DIR)utilities-pthread.h

.PHONY : clean clean-all

clean :
	rm $(SHARED_OBJ) $(NSHARED_OBJ)
clean-all :
	rm -f $(EXE) $(SHARED_OBJ) $(NSHARED_OBJ)
