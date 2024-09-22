markov: markov.o
	gcc -o $@ $<

clean:
	rm markov *.o

.PHONY: clean

%.o:%.c
	gcc -c -o $@ $<


