all:
	cd ./src/matte/src/rom && make
	cd ./src/matte/src/rom && ./makerom 
	cd ./src/api/ && gcc *.c -o apipack
	cd ./src/api/ && ./apipack 
	gcc -g -fsanitize=address -fsanitize=undefined ./src/*.c ./src/develop/*.c ./src/matte/src/*.c ./src/matte/src/rom/native.c -o ses -lm
