TARGETS=mdpc mdpl mdpw mdps

.PHONY: $(TARGETS)
mdpc:
	cd client; make $*
mdpl:
	cd coord; make $*
mdpw:
	cd worker; make $*
mdps:
	cd storage; make $*

.PHONY: clean
clean:
	cd common; make clean
	cd client; make clean
	cd coord; make clean
	cd worker; make clean
	cd storage; make clean

