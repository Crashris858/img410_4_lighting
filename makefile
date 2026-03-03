all: raycast

raycast: raycast.cpp v3math.c 
	g++ -Wall raycast.cpp v3math.c  -o raycast

clean:
	rm -f raycast

run: raycast
	./raycast 500 500 input.scene output.ppm 