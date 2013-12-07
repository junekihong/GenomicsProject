TARGETS=mdpc mdpl mdpw mdps matrix_test

all: $(TARGETS)

.PHONY: $(TARGETS)
mdpc:
	cd src; make mdpc
	cp src/client/mdpc .
mdpl:
	cd src; make mdpl
	cp src/coord/mdpl .
mdpw:
	cd src; make mdpw
	cp src/worker/mdpw .
mdps:
	cd src; make mdps
	cp src/storage/mdps .
matrix_test:
	cd src; make tests
	cp src/tests/matrix_test .

.PHONY: clean
clean:
	rm -f $(TARGETS)
	cd src; make clean
