maxTweeter.out: *.o
	gcc -g -o maxTweeter.out *.o

*.o: *.c 
	gcc -g -c *.c

clean:
	rm -f maxTweeter.out *.o 

