CPP  = g++.exe
DLLWRAP=dllwrap.exe
LIBS = -lversion --no-export-all-symbols --add-stdcall-alias
INCS =
OBJ  = dinput8.o
BIN  = dinput8.dll
CXXFLAGS = $(INCS) -DBUILDING_DLL=1

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	del $(OBJ) $(BIN)



$(BIN): $(OBJ)
	$(DLLWRAP) --driver-name c++ $(OBJ) $(LIBS) -o $(BIN)

$(OBJ): dllmain.cpp
	$(CPP) -c dllmain.cpp -o $(OBJ) $(CXXFLAGS)