# if your Boost is located elsewhere, set it here or use make BOOST_PATH=/your/boost
BOOST_PATH ?= /usr/
OBJDIR = bin
SRCDIR = src

CXX_FLAGS = -O3 -I$(BOOST_PATH)/include/ -g -Wall -fopenmp

# linking as much as possible statically (cannot link TBB like this without manual compilation of the library)
LD_FLAGS = -Wl,-Bstatic $(BOOST_PATH)/lib/libboost_program_options.a $(BOOST_PATH)/lib/libboost_iostreams.a -lz -Wl,-Bdynamic -ltbb 

all: $(OBJDIR)/wordalign

clean:
	rm -rf $(OBJDIR)

# the final binary
$(OBJDIR)/wordalign: $(SRCDIR)/wordalign.cpp $(OBJDIR)/Corpus.o $(OBJDIR)/Model1.o $(OBJDIR)/Options.o $(OBJDIR)/Writer.o $(OBJDIR)/Utils.o $(OBJDIR)/HMM.o
	$(CXX) $(CXX_FLAGS) $^ $(LD_FLAGS) -o $@ 

# intermediate object files
# XXX dependencies are not always correcly handled,
# use make clean if build fails after source code changes
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.hpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXX_FLAGS) -c -o $@ $<
