all: raycast

raycast: raycast.cpp v3math.cpp 
	g++ -Wall raycast.cpp v3math.cpp  -o raycast

clean:
	rm -f raycast

run: raycast
	./raycast 500 500 input.scene output.ppm 

# all: raycast

# raycast: raycast.o v3math.o
# 	g++ -Wall raycast.o v3math.o -o raycast

# raycast.o: raycast.cpp
# 	g++ -Wall -c raycast.cpp

# v3math.o: v3math.c
# 	g++ -Wall -c v3math.cpp

# clean:
# 	rm -f raycast *.o

# run: raycast
# 	./raycast 500 500 input.scene output.ppm