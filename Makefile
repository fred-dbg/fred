# Path to fred packages.
PYTHONPATH=.
# Path to fredtest directory
FREDTEST=test
RECORD_REPLAY=record-replay
# Path to dmtcp_checkpoint (change this only if it's not already in your path)
DMTCPPATH=

all: ${FREDTEST} ${RECORD_REPLAY}
	cd ${FREDTEST} && $(MAKE) $(MAKEFLAGS) all
	cd ${RECORD_REPLAY} && $(MAKE) $(MAKEFLAGS) all

clean:
	cd ${FREDTEST} && $(MAKE) $(MAKEFLAGS) clean

check: all fredtest.py
	if test -n "${DMTCPPATH}"; then PATH=${DMTCPPATH}:$$PATH; fi; \
	  PYTHONPATH=$(PYTHONPATH) ./fredtest.py

# 'make check-TESTS' where TESTS is comma-separated list of tests.
check-%: all fredtest.py
	if test -n "${DMTCPPATH}"; then PATH=${DMTCPPATH}:$$PATH; fi; \
	  PYTHONPATH=$(PYTHONPATH) ./fredtest.py -t '$*'

tidy:
	rm -rf ckpt_*_files
	rm -f ckpt_* dmtcp_restart_script* \
	  dmtcp-shared-memory.* dmtcp-test-typescript.tmp core*
