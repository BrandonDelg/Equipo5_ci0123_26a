XC = c++
STD = -std=c++20
WFLAGS = -Wall -Wextra
APPNAME = program

SRC = src
BUILD = build
BIN = bin

DIRS = $(shell find -L $(SRC) -type d)
FSOURCE = $(wildcard $(DIRS:%=%/*.cpp))
FOBJECT = $(FSOURCE:$(SRC)/%.cpp=$(BUILD)/%.o)

MAINS = simulacion

COMMON_OBJ = $(filter-out $(addprefix $(BUILD)/,$(addsuffix .o,$(MAINS))), $(FOBJECT))

.PHONY: all clean run run-pruebas debug lint $(MAINS)

all: simulacion

$(MAINS): %: $(BIN)/%
	@echo "Ejecutable listo: $@"

$(BIN)/%: $(COMMON_OBJ) $(BUILD)/%.o | $(BIN)
	$(XC) $(STD) $(WFLAGS) -g $^ -o $@ -lpthread

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	$(XC) $(STD) $(WFLAGS) -c -g $< -o $@

$(BIN):
	mkdir -p $@

$(BUILD):
	mkdir -p $@

run: all
	./$(BIN)/simulacion

run-pruebas: simulacion
	@for f in $$(ls casos_de_prueba/*.txt | sort); do \
		echo ""; \
		echo "======================================"; \
		echo "CASO: $$f"; \
		echo "======================================"; \
		./$(BIN)/simulacion < $$f; \
	done

debug: simulacion
	gdb ./$(BIN)/simulacion

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
