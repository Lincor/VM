all:
	make -C src
clean:
	rm `find -name *.o` vm
