all: raycast

raycast: raycast.cpp v3math.cpp 
	g++ -Wall raycast.cpp v3math.cpp  -o raycast

clean:
	rm -f raycast

run: raycast
	./raycast 500 500 input.scene output.ppm 
