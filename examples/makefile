all:
	cd ../src/carts/debug && make
	cp ../src/carts/debug/rom.mod16 ./vertices/debug
	cd ./vertices/ && mod16plus package ./
	cp ../src/carts/debug/rom.mod16 ./sprites/debug 
	cd ./sprites/ && mod16plus package ./
