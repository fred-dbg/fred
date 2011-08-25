# Path to fred packages.
PYTHONPATH=..

all: test-programs
	cd test-programs && $(MAKE) $(MAKEFLAGS) all

clean:
	cd test-programs && $(MAKE) $(MAKEFLAGS) clean

check: all fredtest.py
	PYTHONPATH=$(PYTHONPATH) ./fredtest.py

check-pdb: all fredtest.py
	PYTHONPATH=$(PYTHONPATH) pdb ./fredtest.py
