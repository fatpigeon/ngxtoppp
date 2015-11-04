ngxtoppp.o : src/ngxtop++.cpp src/ngxtop++.h
	g++ -I /usr/include -std=c++11 -c -o build/ngxtoppp.o src/ngxtop++.cpp -g 
lines.o : src/lines.cpp src/lines.h
	g++ -I /usr/include -std=c++11 -c -o build/lines.o src/lines.cpp -g 
test.o : tests/test.cpp
	g++ -I /usr/include -std=c++11 -c -o build/test.o tests/test.cpp -g 
objects = build/lines.o build/ngxtoppp.o build/test.o
test : $(objects)
	g++ -lboost_regex -lboost_iostreams  -o build/test  $(objects)
all : ngxtoppp.o lines.o test.o test
clean :
	rm -f build/*.o
