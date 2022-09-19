all:
	cd ./src/matte/src/rom && make
	cd ./src/matte/src/rom && ./makerom 
	cd ./src/api/ && gcc *.c -o apipack
	cd ./src/shaders && gcc *.c -o shaderpack
	cd ./src/api/ && ./apipack 
	cd ./src/shaders && ./shaderpack
	gcc -O2 ./src/*.c ./src/develop/*.c ./src/matte/src/*.c ./src/matte/src/rom/native.c -o ses -lm -lSDL2 -lGLESv2
