# Path to fred packages.
PYTHONPATH=.
# Path to fredtest directory
FREDTEST=test

all: ${FREDTEST}
	cd ${FREDTEST} && $(MAKE) $(MAKEFLAGS) all

clean:
	cd ${FREDTEST} && $(MAKE) $(MAKEFLAGS) clean

check: all fredtest.py
	PYTHONPATH=$(PYTHONPATH) ./fredtest.py

tidy:
	rm -rf ckpt_*_files
	rm -f ckpt_* dmtcp_restart_script* \
	  dmtcp-shared-memory.* dmtcp-test-typescript.tmp core*
