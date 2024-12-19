all:
	cd ../cartridges/debug && make
	cp ../cartridges/debug/rom.mod16 ./vertices/debug
	cd ./vertices/ && mod16plus package ./
	cp ../cartridges/debug/rom.mod16 ./sprites/debug 
	cd ./sprites/ && mod16plus package ./
	cp ../cartridges/debug/rom.mod16 ./debug/debug 
	cd ./debug/ && mod16plus package ./
