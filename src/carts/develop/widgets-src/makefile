all:
	cd ./rectangle-renderer-src/ && make
	cp ./rectangle-renderer-src/rom.ses ./rectangle-renderer
	cd ./textarea-src/ && make 
	cp ./textarea-src/rom.ses ./textarea
	cd ../../ascii/ && make
	cp ../../ascii/rom.ses ./ascii-renderer
	ses package ./
	
clean:
	cd ./rectangle-renderer-src/ && make clean
	rm -f ./rectangle-renderer
	cd ./textarea-src/ && make clean
	rm -f ./textarea
	cd ../../ascii/ && make clean
	rm -f ./ascii-renderer
	rm -f rom.ses

