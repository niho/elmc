TARGET= elmc
SRC_DIR= ./src
BUILD_DIR= ./build

CC= gcc -std=gnu99
CFLAGS= -Wall -Wextra
DBGCFLAGS= -DDEBUG -g
RELCFLAGS= -O3
AS= gcc
ASFLAGS=
DBGASFLAGS= -Wa,g
LDFLAGS=

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += $(DBGCFLAGS)
	ASFLAGS += $(DBGASFLAGS)
else
    CFLAGS += $(RELCFLAGS)
endif

SRC := $(shell find $(SRC_DIR) -name *.c -or -name *.s)
OBJ := $(SRC:%=$(BUILD_DIR)/%.o)
DEP := $(OBJ:.o=.d)

INC := $(shell find $(SRC_DIR) -type d)
INCFLAGS := $(addprefix -I,$(INC)) -MMD -MP

$(BUILD_DIR)/$(TARGET): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.s.o: %s
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c -o $@ $<

clean:
	rm -r $(BUILD_DIR)

-include $(DEP)

.PHONY : clean

