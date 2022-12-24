CC := clang

ifeq ($(DEBUG),1)
	CFLAGS := -Wall -O0 -g -DDEBUG
else
	CFLAGS := -Wall -O1 -DNDEBUG 
endif

LDFLAGS := -lrt

all: isort sum

isort.o: isort.c 
	$(CC) $(CFLAGS) -c isort.c 

qsort.o: qsort.c 
	$(CC) $(CFLAGS) -c qsort.c 

isort: isort.o qsort.o
	$(CC) -o isort isort.o qsort.o $(LDFLAGS)

sum.o: sum.c 
	$(CC) $(CFLAGS) -c sum.c 

sum: sum.o
	$(CC) -o sum sum.o $(LDFLAGS)


sum2.o: sum.c 
	$(CC) $(CFLAGS) -c sum2.c 

sum2: sum2.o
	$(CC) -o sum2 sum2.o $(LDFLAGS)

clean:
	rm -f isort sum sum2 *.o *.gcda *.gcno *.gcov perf.data */perf.data cachegrind.out.*
