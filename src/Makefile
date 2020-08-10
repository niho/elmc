
PLAT= guess

CC= gcc -std=gnu99
CFLAGS= -Wall -Wextra $(SYSCFLAGS)
LDFLAGS= $(SYSLDFLAGS)
LIBS= -lm $(SYSLIBS)

DBGCFLAGS= -DDEBUG -g
RELCFLAGS= -O3

SYSCFLAGS=
SYSLDFLAGS=
SYSLIBS=

AR= ar rcu
RANLIB= ranlib
MKDIR= mkdir -p
RM= rm -f
UNAME= uname
MAKE= make

### END OF USER SETTINGS ###

PLATS= guess generic linux macosx posix

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += $(DBGCFLAGS)
else
    CFLAGS += $(RELCFLAGS)
endif

ELM_A=  libelm.a
BASE_O= east.o eparse.o estack.o ezio.o mpc.o

ELM_T=  elm
ELM_O=  elm.o

ELMC_T= elmc
ELMC_O= elmc.o

TEST_T= test
TEST_O= test.o

ALL_O= $(BASE_O) $(ELM_O) $(ELMC_O) $(TEST_O)
ALL_T= $(ELM_A) $(ELM_T) $(ELMC_T) $(TEST_T)
ALL_A= $(ELM_A)

# Targets start here:

default: $(PLAT)

all: $(ALL_T)

o: $(ALL_O)

a: $(ALL_A)

$(ELM_A): $(BASE_O)
	$(AR) $@ $^
	$(RANLIB) $@

$(ELM_T): $(ELM_O) $(ELM_A)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(ELMC_T): $(ELMC_O) $(ELM_A)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(TEST_T): $(TEST_O) $(ELM_A)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

depend:
	@$(CC) $(CFLAGS) $(INCFLAGS) -MM e*.c

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.s.o: %s
	$(MKDIR) $(dir $@)
	$(AS) $(ASFLAGS) -c -o $@ $<

clean:
	$(RM) $(ALL_T) $(ALL_O)

ALL= all

guess:
	@$(MAKE) `$(UNAME)`

generic: $(ALL)

Linux linux:
	$(MAKE) $(ALL) SYSCFLAGS="-DELM_USE_LINUX"

Darwin macos macosx:
	$(MAKE) $(ALL) SYSCFLAGS="-DELM_USE_MACOSX"

posix:
	$(MAKE) $(ALL) SYSCFLAGS="-DELM_USE_POSIX"


.PHONY : default all $(PLATS) clean o a depend

