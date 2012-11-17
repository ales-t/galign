BOOST_PATH = /home/ales/boost/

CXX_FLAGS = -O3 -I$(BOOST_PATH)/include/ -L$(BOOST_PATH)/lib/

.SECONDARY:

all: build/wordalign

build:
	mkdir -p build

clean:
	rm -rf build

build/wordalign: src/wordalign.cpp build/Corpus.o build/Model1.o build/Options.o
	$(CXX) $(CXX_FLAGS) $^ -lz $(BOOST_PATH)/lib/libboost_program_options.a $(BOOST_PATH)/lib/libboost_iostreams.a -o $@

build/%.o: src/%.cpp build
	$(CXX) $(CXX_FLAGS) -c -o $@ $<
