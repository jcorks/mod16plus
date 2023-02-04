all:
	cd ./rectangle-renderer-src/ && make
	cp ./rectangle-renderer-src/rom.mod16 ./rectangle-renderer
	cd ./textarea-src/ && make 
	cp ./textarea-src/rom.mod16 ./textarea
	cd ../../ascii/ && make
	cp ../../ascii/rom.mod16 ./ascii-renderer
	mod16plus package ./
	
clean:
	cd ./rectangle-renderer-src/ && make clean
	rm -f ./rectangle-renderer
	cd ./textarea-src/ && make clean
	rm -f ./textarea
	cd ../../ascii/ && make clean
	rm -f ./ascii-renderer
	rm -f rom.mod16

