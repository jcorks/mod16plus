all:
	cd ../ascii && make
	cp ../ascii/rom.ses ./ascii-renderer
	ses package ./

clean:
	cd ../ascii && make clean
	rm -f ./ascii-renderer
	rm -f ./rom.ses

