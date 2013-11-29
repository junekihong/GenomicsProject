TARGETS=mdpc mdpl mdpw mdps

all: $(TARGETS)

.PHONY: $(TARGETS) common
common:
	cd common; make

mdpc: common
	cd client; make $*

mdpl: common
	cd coord; make $*

mdpw: common
	cd worker; make $*

mdps: common
	cd storage; make $*

.PHONY: clean
clean:
	cd common; make clean
	cd client; make clean
	cd coord; make clean
	cd worker; make clean
	cd storage; make clean
