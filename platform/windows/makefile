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
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

out\objs\core\\%.o : core\\%.c
	@gcc -fvisibility=hidden -fPIC -c $^ -o $@ $(INCLUDE_PATHS)

build: clean $(OBJ_FILES_MOD) $(OBJ_FILES_CORE) copy
	@ar rcs out/Windows/libCrystalCore.a $(OBJ_FILES_CORE)
	@gcc -shared -o out/Windows/CrystalCore.so $(OBJ_FILES_MOD) $(INCLUDE_PATHS) -lwinmm

debug:
	@echo $(C_FILES)
	@echo $(C_FILES_MOD)
	@echo $(C_FILES_CORE)

.PHONY: build clean copy debug