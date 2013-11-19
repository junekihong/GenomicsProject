TARGETS=mdpc mdpl mdpw mdps

all: $(TARGETS)
	cp src/client/mdpc .
	cp src/coord/mdpl .
	cp src/worker/mdpw .
	cp src/storage/mdps .

%:
	cd src; make $*

.PHONY: clean
clean:
	rm $(TARGETS)
	cd src; make clean
