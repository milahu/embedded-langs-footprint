SVN := svn co -q
GIT_CLONE := git clone -q --depth 1
CXXFLAGS := -Os -s -std=c++17
# For debugging:
# CXXFLAGS := -O0 -g -std=c++17

executables := \
	target/tinyscheme \
	target/lua \
	target/chibi-scheme \
	target/squirrel \
	target/arkscript \
	target/gravity \
	target/janet \
	target/chaiscript \
	target/angelscript \
	target/python

results := results.adoc

executable_runners := $(patsubst %,run[%], $(executables))
$(executable_runners):: run[%] : %
	@echo | tee -a $(results)
	@$^ | tee -a $(results)

.PHONE: clean-results
clean-results:
	$(RM) $(results)

.PHONY: run
run: clean-results $(executable_runners)

angelscript-code := target/angelscript-code
$(angelscript-code): | target
	$(SVN) "https://svn.code.sf.net/p/angelscript/code/trunk" $@

angelscript-lib :=$(angelscript-code)/sdk/angelscript/lib/libangelscript.a
$(angelscript-lib): $(angelscript-code)
	$(MAKE) -C $(angelscript-code)/sdk/angelscript/projects/gnuc static

target/angelscript: src/angelscript.cc src/mem.cc $(angelscript-lib)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		$(angelscript-code)/sdk/add_on/scriptarray/*.cpp \
		$(angelscript-code)/sdk/add_on/scriptstdstring/*.cpp \
		-I$(angelscript-code)/sdk/angelscript/include \
		-I$(angelscript-code)/sdk/add_on \
		-pthread

chaiscript-code := target/chaiscript-code
$(chaiscript-code): | target
	$(GIT_CLONE) "https://github.com/ChaiScript/ChaiScript" $@

target/chaiscript: src/chaiscript.cc src/mem.cc | $(chaiscript-code)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(chaiscript-code)/include \
		-pthread -ldl

# HEAD is broken now because of 578abafb0f19436ab46f45f85a1286cd31ad89d3
arkscript-code := target/arkscript-code
$(arkscript-code): | target
	$(GIT_CLONE) "https://github.com/SuperFola/Ark" $@
	cd $@ && git fetch --all --tags && git checkout tags/v3.0.12

arkscript-lib := $(arkscript-code)/build/libArkReactor.a
$(arkscript-lib): $(arkscript-code)
	cd $(arkscript-code) && \
	git submodule update --init --recursive && \
	cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release && \
	cmake --build build

target/arkscript: src/arkscript.cc src/mem.cc $(arkscript-lib)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(arkscript-code)/include \
		-I$(arkscript-code)/submodules/String/include \
		-I$(arkscript-code)/thirdparty \
		-ldl

chibi-scheme-code := target/chibi-scheme-code
$(chibi-scheme-code): | target
	$(GIT_CLONE) "https://github.com/ashinn/chibi-scheme" $@

chibi-scheme-lib := $(chibi-scheme-code)/libchibi-scheme.a
$(chibi-scheme-lib): $(chibi-scheme-code)
	$(MAKE) -C $(chibi-scheme-code) libchibi-scheme.a

target/chibi-scheme: src/chibi-scheme.cc src/mem.cc $(chibi-scheme-lib)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(chibi-scheme-code)/include \
		-ldl

gravity-code := target/gravity-code
$(gravity-code): | target
	$(GIT_CLONE) "https://github.com/marcobambini/gravity" $@

gravity-lib := $(gravity-code)/libgravityapi_s.a
$(gravity-lib): $(gravity-code)
	cd $(gravity-code) && \
	cmake . && \
	$(MAKE)

target/gravity: src/gravity.cc src/mem.cc $(gravity-lib)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(gravity-code)/src/shared \
		-I$(gravity-code)/src/utils \
		-I$(gravity-code)/src/compiler \
		-I$(gravity-code)/src/runtime

janet-code := target/janet-code
$(janet-code): | target
	$(GIT_CLONE) "https://github.com/janet-lang/janet" $@

janet-lib := $(janet-code)/build/libjanet.a
$(janet-lib): $(janet-code)
	$(MAKE) -C $(janet-code)

target/janet: src/janet.cc src/mem.cc $(janet-lib)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(janet-code)/build \
		-ldl -pthread

squirrel-code := target/squirrel-code
$(squirrel-code): | target
	$(GIT_CLONE) "https://github.com/albertodemichelis/squirrel" $@

squirrel-lib := $(squirrel-code)/lib/libsquirrel.a
$(squirrel-lib): $(squirrel-code)
	$(MAKE) -C $(squirrel-code)

target/squirrel: src/squirrel.cc src/mem.cc $(squirrel-lib) $(squirrel-code)/lib/libsqstdlib.a
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(squirrel-code)/include

tinyscheme-code := target/tinyscheme-code
$(tinyscheme-code): | target
	$(SVN) "https://svn.code.sf.net/p/tinyscheme/code/trunk" $@

tinyscheme-lib := $(tinyscheme-code)/libtinyscheme.a
$(tinyscheme-lib): $(tinyscheme-code)
	DL_FLAGS=-DSTANDALONE=0 \
	$(MAKE) -C $(tinyscheme-code) libtinyscheme.a

target/tinyscheme: src/tinyscheme.cc src/mem.cc $(tinyscheme-lib)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(tinyscheme-code) \
		-DSTANDALONE=0 -ldl

lua-code := target/lua-code
$(lua-code): | target
	$(GIT_CLONE) "https://github.com/lua/lua" $@

lua-lib := $(lua-code)/liblua.a
$(lua-lib): $(lua-code)
	$(MAKE) -C $(lua-code) a

target/lua: src/lua.cc src/mem.cc $(lua-lib)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(lua-code) -ldl

python-code := target/python-code
$(python-code): | target
	$(GIT_CLONE) "https://github.com/python/cpython.git" $@

python-lib := $(python-code)/libpython.a
$(python-lib): | $(python-code)
	cd $(python-code) && \
	./configure && \
	$(MAKE) -j VERSION="" libpython.a

python-std := target/python-std.zip
python-std-path := $(PWD)/$(python-std)

python-std-modules := _collections_abc.py \
_sitebuiltins.py \
abc.py \
codecs.py \
genericpath.py \
io.py \
os.py \
posixpath.py \
site.py \
stat.py \
encodings/aliases.py \
encodings/__init__.py \
encodings/utf_8.py

$(python-std): | $(python-code)
	cd $(python-code)/Lib && \
	zip $(python-std-path) $(python-std-modules)

python-std-elf := target/python-std.o
$(python-std-elf): $(python-std)
	$(LD) -r -b binary -o $@ $^

target/python: src/python.cc src/mem.cc $(python-lib) $(python-std-elf)
	$(CXX) $(CXXFLAGS) $^ -o $@ \
		-I$(python-code) -I$(python-code)/Include \
		-ldl -pthread -lutil -fPIC

target:
	mkdir -p target

.PHONY: clean
clean:
	$(RM) target
