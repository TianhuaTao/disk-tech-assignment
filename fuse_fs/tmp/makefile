helloworld: helloworld.o helloworld_cxx.a
	gcc helloworld.o helloworld_cxx.a  `pkg-config fuse3 --cflags --libs` -lstdc++ -o helloworld

helloworld_cxx.a: helloworld_cxx.o
	ar rvs helloworld_cxx.a helloworld_cxx.o

helloworld_cxx.o: helloworld_cxx.cpp
	g++ -c helloworld_cxx.cpp

helloworld.o: helloworld.c
	gcc  -c helloworld.c  `pkg-config fuse3 --cflags --libs` 

.PHONY:
clean:
	rm -rf helloworld *.o *.a
