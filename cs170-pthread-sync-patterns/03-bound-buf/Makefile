CC = gcc
UTILS_MEM_DIR  = ../../utilities/utilities-mem/
UTILS_PTHD_DIR = ../../utilities/utilities-pthread/
CFLAGS = -I$(UTILS_MEM_DIR)                            \
         -I$(UTILS_PTHD_DIR)                           \
         -std=gnu90 -pthread -Wpedantic -Wall -Wextra

EXE = bound-buf-mutex    \
      bound-buf-condvar1 \
      bound-buf-condvar2 \
      bound-buf-sema

SHARED_OBJ = ctimer.o                              \
             $(UTILS_MEM_DIR)utilities-mem.o       \
             $(UTILS_PTHD_DIR)utilities-pthread.o

NSHARED_OBJ = bound-buf-mutex.o    \
              bound-buf-condvar1.o \
              bound-buf-condvar2.o \
              bound-buf-sema.o

all                   : $(EXE)
bound-buf-mutex : bound-buf-mutex.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
bound-buf-condvar1 : bound-buf-condvar1.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
bound-buf-condvar2 : bound-buf-condvar2.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^
bound-buf-sema : bound-buf-sema.o $(SHARED_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

bound-buf-mutex.o                    : ctimer.h                             \
                                       $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
bound-buf-condvar1.o                 : ctimer.h                             \
                                       $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
bound-buf-condvar2.o                 : ctimer.h                             \
                                       $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
bound-buf-sema.o                     : ctimer.h                             \
                                       $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
ctimer.o                             : ctimer.h
$(UTILS_MEM_DIR)utilities-mem.o      : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_PTHD_DIR)utilities-pthread.o : $(UTILS_PTHD_DIR)utilities-pthread.h

.PHONY : clean clean-all

clean :
	rm $(SHARED_OBJ) $(NSHARED_OBJ)
clean-all :
	rm -f $(EXE) $(SHARED_OBJ) $(NSHARED_OBJ)
