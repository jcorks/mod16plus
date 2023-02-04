all:
	cd ../../../ascii/ && make
	cp ../../../ascii/rom.mod16 ./ascii-renderer
	mod16plus package ./
	
clean: 
	cd ../../../ascii/ && make clean
	rm -f ./ascii-renderer 
	rm -f ./rom.mod16
	
