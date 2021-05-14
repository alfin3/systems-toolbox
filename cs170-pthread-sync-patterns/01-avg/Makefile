CC = gcc
UTILS_MEM_DIR  = ../../utilities/utilities-mem/
UTILS_PTHD_DIR = ../../utilities/utilities-pthread/
CFLAGS = -I$(UTILS_MEM_DIR)         \
         -I$(UTILS_PTHD_DIR)        \
         -pthread -Wall -Wextra -O2

OBJ = avg.o                                \
      $(UTILS_MEM_DIR)utilities-mem.o      \
      $(UTILS_PTHD_DIR)utilities-pthread.o

avg : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

avg.o                                : $(UTILS_MEM_DIR)utilities-mem.h      \
                                       $(UTILS_PTHD_DIR)utilities-pthread.h
$(UTILS_MEM_DIR)utilities-mem.o      : $(UTILS_MEM_DIR)utilities-mem.h
$(UTILS_PTHD_DIR)utilities-pthread.o : $(UTILS_PTHD_DIR)utilities-pthread.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all :
	rm -f avg $(OBJ)
