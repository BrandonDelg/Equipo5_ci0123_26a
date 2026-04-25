XC = c++
STD = -std=c++20
WFLAGS = -Wall -Wextra

LIBS = -lssl -lcrypto

SRC = src
BUILD = build
BIN = bin

DIRS = $(shell find -L $(SRC) -type d)
FSOURCE = $(wildcard $(DIRS:%=%/*.cpp))
FOBJECT = $(FSOURCE:$(SRC)/%.cpp=$(BUILD)/%.o)

MAINS = main main_casos_de_prueba

main_BIN = program
main_casos_de_prueba_BIN = program_test

COMMON_OBJ = $(filter-out $(addprefix $(BUILD)/,$(addsuffix .o,$(MAINS))), $(FOBJECT))

.PHONY: all clean run debug lint $(MAINS)

all: main

$(MAINS): %:
	$(MAKE) $(BIN)/$($@_BIN)

$(BIN)/program: $(COMMON_OBJ) $(BUILD)/main.o | $(BIN)
	$(XC) $(STD) $(WFLAGS) -g $^ -o $@ $(LIBS)

$(BIN)/program_test: $(COMMON_OBJ) $(BUILD)/main_casos_de_prueba.o | $(BIN)
	$(XC) $(STD) $(WFLAGS) -g $^ -o $@ $(LIBS)

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	$(XC) $(STD) $(WFLAGS) -c -g $< -o $@

$(BIN):
	mkdir -p $@

$(BUILD):
	mkdir -p $@

run: main
	./$(BIN)/program $(ARGS)

run-test: main_casos_de_prueba
	./$(BIN)/program_test $(ARGS)

debug: main
	gdb ./$(BIN)/program

clean:
	rm -rf $(BIN) $(BUILD)

CPPLINT_FLAGS=--filter=-readability/casting,-build/header_guard,-build/include_subdir,-runtime/int

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