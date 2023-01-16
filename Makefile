#DEBUG=1

INCLUDE_DIR=../include
SYNCLINK_H=$(INCLUDE_DIR)/synclink.h

CFLAGS := -Wall -Wstrict-prototypes -fomit-frame-pointer -I $(INCLUDE_DIR)
ifdef DEBUG
CFLAGS += -g -O 
else
CFLAGS += -O2
endif

TOOLS=hdlc 
all : $(TOOLS)

hdlc: hdlc.cpp $(SYNCLINK_H)
	g++ $(CFLAGS) -o $@ $< -lpthread 


CLEAN_TARGETS = *~ core $(TOOLS) *.o
clean:
	@for f in $(CLEAN_TARGETS) ; do \
		find . -name "$$f" | xargs rm -f ; \
	done;

