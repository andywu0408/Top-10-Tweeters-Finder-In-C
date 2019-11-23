maxTweeter.out: *.o
	gcc -Wall -g -o maxTweeter.out *.o

*.o: *.c 
	gcc -Wall -g -c *.c

clean:
	rm -f maxTweeter.out *.o 

