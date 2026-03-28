CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -O2 -D_XOPEN_SOURCE_EXTENDED -D_DEFAULT_SOURCE
LDFLAGS := -lncursesw
INCLUDE := -Iinclude

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := build

TARGET  := $(BIN_DIR)/skanna
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/$(SRC_DIR)/%.c.o, $(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	@echo "[LINKING] $@"
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "[DONE]"

$(OBJ_DIR)/$(SRC_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "[COMPILING] $<"
	@$(CC) $(CFLAGS) $(INCLUDE) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)

run: all
	./$(TARGET)

clean:
	@echo "[CLEANING]"
	rm -rf $(OBJ_DIR) $(BIN_DIR)
