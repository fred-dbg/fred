# Path to fred packages.
PYTHONPATH=.
# Path to fredtest directory
FREDTEST=fredtest

all: ${FREDTEST}/test-programs
	cd ${FREDTEST}/test-programs && $(MAKE) $(MAKEFLAGS) all

clean:
	cd ${FREDTEST}/test-programs && $(MAKE) $(MAKEFLAGS) clean

check: all fredtest.py
	PYTHONPATH=$(PYTHONPATH) ./fredtest.py
