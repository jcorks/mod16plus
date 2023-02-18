all:
	cd ./widgets-src && make
	cp ./widgets-src/rom.mod16 ./widgets
	cd ./widgets-src/textarea-src && make 
	cp ./widgets-src/textarea-src/rom.mod16 ./textarea
	cd ./widgets-src/rectangle-renderer-src && make 
	cp ./widgets-src/rectangle-renderer-src/rom.mod16 ./rectangle-renderer
	cd ../debug/ && make 
	cp ../debug/rom.mod16 ./console
	mod16plus package ./
	
clean:
	cd ./widgets-src && make clean
	cd ./widgets-src/textarea-src && make clean
	cd ./widgets-src/rectangle-renderer-src && make clean
	cd ../debug/ && make 
	rm -f ./rom.mod16
	rm -f ./console
	rm -f ./rectangle-renderer
	rm -f ./textarea
	rm -f ./widgets
