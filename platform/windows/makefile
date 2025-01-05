INCLUDE_PATHS := .\include
INCLUDE_PATHS += .\public\include

INCLUDE_PATHS := $(foreach item,$(INCLUDE_PATHS),-I$(item))

C_FILES := $(shell powershell Get-ChildItem -Recurse -Filter "*.c" -Path . -Name)
C_FILES_MOD := $(filter src\\%.c,$(C_FILES))
C_FILES_CORE := $(filter core\\%.c,$(C_FILES))
OBJ_FILES_MOD := $(patsubst src\\%.c,out\objs\src\\%.o,$(C_FILES_MOD))
OBJ_FILES_CORE := $(patsubst core\\%.c,out\objs\core\\%.o,$(C_FILES_CORE))

clean:
	@if not exist out\ (mkdir .\out)
	@if exist out\Windows\ (rmdir /s/q .\out\Windows)
	@if exist out\objs\ (rmdir /s/q .\out\objs)
	@mkdir .\out\Windows
	@mkdir .\out\objs\core
	@mkdir .\out\objs\src

copy:

out\objs\src\\%.o : src\\%.c
	@gcc -fexec-charset=GBK -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

out\objs\core\\%.o : core\\%.c
	@gcc -fexec-charset=GBK -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

build: clean $(OBJ_FILES_MOD) $(OBJ_FILES_CORE) copy
	@ar rcs out/Windows/libCrystalCore.a $(OBJ_FILES_CORE)
	@gcc -fexec-charset=GBK -shared -o out/Windows/CrystalCore.so $(OBJ_FILES_MOD) $(INCLUDE_PATHS) -lwinmm

C_FILES_TEST := $(filter test\\%.c,$(C_FILES))

TEST_LIBRARY_PATHS := .\\out\\Windows

TEST_LIBRARIES := CrystalCore

TEST_LIBRARY_PATHS := $(foreach item,$(TEST_LIBRARY_PATHS),-L$(item))
TEST_LIBRARIES := $(foreach item,$(TEST_LIBRARIES),-l$(item))

cleantest:
	@if exist out\\Windows\\Test1.exe (rm out\\Windows\\Test1.exe)

test:
	@if not exist out\\ (echo you have to "mingw32-make build" first) \
	else (gcc -fexec-charset=GBK -o out\\Windows\\Test1.exe $(INCLUDE_PATHS) $(C_FILES_TEST) $(TEST_LIBRARY_PATHS) $(TEST_LIBRARIES))

.PHONY: build clean copy test