SUBDIRS := hw1 hw2 hw3 hw4

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)

clean:
	for DIR in $(SUBDIRS) ; do\
		$(MAKE) -C  $$DIR  clean ; \
	done
.PHONY: clean
