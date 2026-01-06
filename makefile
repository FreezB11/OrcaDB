CC = gcc
FLAGS = -O3 -g -march=native -D_GNU_SOURCE
FLAGS += -lpthread -flto
FLAGS += -Wall -Wextra -Wunused-parameter

SRC := $(shell find ./src -name "*.c")
OBJ_DIR = obj
OBJS := $(SRC:.c=.o)

OUTPUT := orca

OBJS_C = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(SRC_DIR)/*.c))

.PHONY: all directories $(OUTPUT) echo
all: directories $(OUTPUT) echo

$(OUTPUT): $(OBJS)

echo:
# 	echo "hello"

# rule to compile .c to .o
%.o: %.c
# 	echo hi
	mkdir -p $(OBJ_DIR)/$(dir $@)
	$(CC) $(FLAGS) -c $< -o $(OBJ_DIR)/$@

directories:
	mkdir -p $(OBJ_DIR)

docker:
	docker build -t kvcache .
	docker run --rm kvcache
	docker build -f Dockerfile.test -t test-test .
	docker build -f Dockerfile.prod -t test-prod .

clean:
# 	rm -f $(OBJS)
	rm -r ./obj