all:
	gcc ./src/api/*.c -o ./src/api/apipack
        ./src/api/apipack
	gcc ./src*.c ./matte/src/*.c
