INCLUDE_PATHS := ./include
INCLUDE_PATHS += ./public/include

INCLUDE_PATHS := $(foreach item,$(INCLUDE_PATHS),-I$(item))

C_FILES := $(shell find . -name "*.c")
C_FILES_MOD := $(filter ./src/%.c,$(C_FILES))
C_FILES_CORE := $(filter ./core/%.c,$(C_FILES))
OBJ_FILES_MOD := $(patsubst ./src/%.c,./out/objs/src/%.o,$(C_FILES_MOD))
OBJ_FILES_CORE := $(patsubst ./core/%.c,./out/objs/core/%.o,$(C_FILES_CORE))

clean:
	@if [ ! -d "./out/" ];then mkdir ./out;fi
	@if [ -d "./out/Linux/" ];then rm -r ./out/Linux/;fi
	@if [ -d "./out/objs/" ];then rm -r ./out/objs/;fi
	@mkdir -p ./out/Linux
	@mkdir -p ./out/objs/core
	@mkdir -p ./out/objs/src

copy:

./out/objs/src/%.o: ./src/%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

./out/objs/core/%.o: ./core/%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

build: clean $(OBJ_FILES_MOD) $(OBJ_FILES_CORE) copy
	@ar rcs out/Linux/libCrystalCore.a $(OBJ_FILES_CORE)
	@gcc -shared -o out/Linux/CrystalCore.so $(OBJ_FILES_MOD) $(INCLUDE_PATHS)

C_FILES_TEST := $(filter ./test/%.c,$(C_FILES))

TEST_LIBRARY_PATHS := ./out/Linux

TEST_LIBRARIES := CrystalCore

TEST_LIBRARY_PATHS := $(foreach item,$(TEST_LIBRARY_PATHS),-L$(item))
TEST_LIBRARIES := $(foreach item,$(TEST_LIBRARIES),-l$(item))

cleantest:
	@if [ -d "./out/Linux/Test1" ]; then rm ./out/Linux/Test1;fi

test: cleantest
	@if [ ! -d "./out/" ];then echo you have to "make build" first; \
	else gcc -o ./out/Linux/Test1 $(INCLUDE_PATHS) $(C_FILES_TEST) $(TEST_LIBRARY_PATHS) $(TEST_LIBRARIES); \
	fi

.PHONY: build clean copy