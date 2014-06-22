all:
	make -C src
clean:
	rm -f `find -name *.o` vm
