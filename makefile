all:
	cd ./src/matte/src/rom && make
	cd ./src/matte/src/rom && ./makerom 
	gcc ./src/api/*.c -o ./src/api/apipack
	./src/api/apipack ./src/api/api.mt
	gcc ./src/*.c ./src/develop/*.c ./src/matte/src/*.c ./src/matte/src/rom/native.c -o ses -lm
