#module load gcc-6.1.0
linker: main.cpp
	g++61 -g -o linker main.cpp
clean:
	rm -f linker *~
