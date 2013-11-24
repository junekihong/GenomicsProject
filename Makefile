TARGETS=mdpc mdpl mdpw mdps

all: $(TARGETS)
	cp src/client/mdpc .
	cp src/coord/mdpl .
	cp src/worker/mdpw .
	cp src/storage/mdps .

.PHONY: $(TARGETS)
mdpc:
	cd src; make mdpc
mdpl:
	cd src; make mdpl
mdpw:
	cd src; make mdpw
mdps:
	cd src; make mdps

.PHONY: clean
clean:
	rm $(TARGETS)
	cd src; make clean
