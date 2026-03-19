XC=c++
STD=-std=c++20
WFLAGS=-Wall -Wextra
APPNAME=program

# Configuración de cpplint
CPPLINT_FLAGS=--filter=-readability/casting,-build/header_guard,-build/include_subdir,-runtime/int

# Directorios
SRC=src
BUILD=build
BIN=bin

# Archivos
DIRS=$(shell find -L $(SRC) -type d)
FSOURCE=$(wildcard $(DIRS:%=%/*.cpp))
FHEADERS=$(wildcard $(DIRS:%=%/*.hpp))
FOBJECT=$(FSOURCE:$(SRC)/%.cpp=$(BUILD)/%.o)

.PHONY: all bin build run clean show debug lint

all: $(BIN)/$(APPNAME)

$(BIN)/$(APPNAME): $(FOBJECT) | $(BIN)
	$(XC) $(STD) $(WFLAGS) -g $^ -o $@

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	$(XC) $(STD) $(WFLAGS) -c -g $^ -o $@

$(BIN):
	mkdir -p $@

$(BUILD):
	mkdir -p $@

run:
	./$(BIN)/$(APPNAME)

clean:
	rm -rf $(BIN) $(BUILD)

show:
	echo $(FOBJECT)

debug: $(BIN)/$(APPNAME)
	gdb ./$(BIN)/$(APPNAME)

lint:
	@echo "Ejecutando cpplint..."
	@find $(SRC) -name "*.cpp" -o -name "*.hpp" | xargs cpplint $(CPPLINT_FLAGS)
	@echo "Análisis completado"


lint-file:
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make lint-file FILE=path/to/file.cpp"; \
		exit 1; \
	fi
	@echo "Analizando $(FILE)..."
	@cpplint $(CPPLINT_FLAGS) $(FILE) || true
	@echo "Análisis completado"
