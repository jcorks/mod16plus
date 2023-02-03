all:
	cd ./widgets-src && make
	cp ./widgets-src/rom.ses ./widgets
	cd ./widgets-src/textarea-src && make 
	cp ./widgets-src/textarea-src/rom.ses ./textarea
	cd ./widgets-src/rectangle-renderer-src && make 
	cp ./widgets-src/rectangle-renderer-src/rom.ses ./rectangle-renderer
	cd ../debug/ && make 
	cp ../debug/rom.ses ./console
	ses package ./
	
clean:
	cd ./widgets-src && make clean
	cd ./widgets-src/textarea-src && make clean
	cd ./widgets-src/rectangle-renderer-src && make clean
	cd ../debug/ && make 
	rm -f ./rom.ses
	rm -f ./console
	rm -f ./rectangle-renderer
	rm -f ./textarea
	rm -f ./widgets
