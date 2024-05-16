INCLUDE_DIRS = -I $(UTILS_DIR) -I $(LIBS_DIR)

CFLAGS = -Wall -std=c11 $(INCLUDE_DIRS)
LDFLAGS = -lmosquitto -lcurl

SRC_DIR := src
OBJ_DIR := obj
UTILS_DIR := utils
LIBS_DIR := libs

SRCS := $(wildcard $(SRC_DIR)/*.c)
UTILS := $(wildcard $(UTILS_DIR)/*.c)
LIBS := $(wildcard $(LIBS_DIR)/*.c)

OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
UTIL_OBJS := $(patsubst $(UTILS_DIR)/%.c,$(OBJ_DIR)/%.o,$(UTILS))
LIB_OBJS := $(patsubst $(LIBS_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIBS))

EXECUTABLE := mqtt_sub

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(UTIL_OBJS) $(LIB_OBJS) $(OBJS) 
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(UTILS_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(LIBS_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@




clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)

