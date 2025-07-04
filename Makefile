CC := gcc
CFLAGS := -I./include -Wall -Wextra
LDFLAGS := -lncurses
DEBUG_FLAGS := -g

SRC_DIR := src
INCLUDE_DIR := include
OBJ_DIR := obj
BIN := void

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: CFLAGS += -O3
all: $(BIN)

debug: CFLAGS += $(DEBUG_FLAGS) -Og
debug: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all debug clean
