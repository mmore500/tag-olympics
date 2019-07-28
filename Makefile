# Project-specific settings
PROJECT := tag-olympics
EMP_DIR := ../Empirical/source

# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++17 -I$(EMP_DIR)/

# Native compiler information
CXX_nat := g++
CFLAGS_nat := -O3 -DNDEBUG $(CFLAGS_all)
CFLAGS_nat_debug := -g -v $(CFLAGS_all)

# Emscripten compiler information
CXX_web := emcc
OFLAGS_web_all := -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']" -s TOTAL_MEMORY=67108864 --js-library $(EMP_DIR)/web/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1 #--embed-file configs
OFLAGS_web := -Oz -DNDEBUG
OFLAGS_web_debug := -g4 -v -Oz -Wno-dollar-in-identifier-extension

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) $(OFLAGS_web_all)
CFLAGS_web_debug := $(CFLAGS_all) $(OFLAGS_web_debug) $(OFLAGS_web_all)


default: native
native: low-$(PROJECT) mid-$(PROJECT)
web: $(PROJECT).js
all: $(PROJECT) $(PROJECT).js

low: low-$(PROJECT)
mid: mid-$(PROJECT)

debug:	CFLAGS_nat := $(CFLAGS_nat_debug)
debug:	$(PROJECT)

debug-web:	CFLAGS_web := $(CFLAGS_web_debug)
debug-web:	$(PROJECT).js

web-debug:	debug-web

low-$(PROJECT):	source/native/low-$(PROJECT).cc
	$(CXX_nat) $(CFLAGS_nat) source/native/low-$(PROJECT).cc -o low-$(PROJECT)
	@echo To build the web version use: make web

mid-$(PROJECT):	source/native/low-$(PROJECT).cc
	$(CXX_nat) $(CFLAGS_nat) source/native/mid-$(PROJECT).cc -o mid-$(PROJECT)
	@echo To build the web version use: make web

$(PROJECT).js: source/web/$(PROJECT)-web.cc
	$(CXX_web) $(CFLAGS_web) source/web/$(PROJECT)-web.cc -o web/$(PROJECT).js

.PHONY: clean test serve

serve:
	python3 -m http.server

clean:
	rm -f low-$(PROJECT) mid-$(PROJECT) web/$(PROJECT).js web/*.js.map web/*.js.map *~ source/*.o web/*.wasm web/*.wast

test: debug
	./low-tag-olympics | grep -q 'no run type provided' && echo 'matched!' || exit 1
	./mid-tag-olympics | grep -q 'no run type provided' && echo 'matched!' || exit 1

# Debugging information
print-%: ; @echo '$(subst ','\'',$*=$($*))'
