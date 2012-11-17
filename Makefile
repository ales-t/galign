BOOST_PATH = /usr
OBJDIR = bin
SRCDIR = src

CXX_FLAGS = -O3 -I$(BOOST_PATH)/include/
LD_FLAGS = -static -L$(BOOST_PATH)/lib/ -lboost_program_options -lboost_iostreams -lz

all: $(OBJDIR)/wordalign

clean:
	rm -rf $(OBJDIR)

$(OBJDIR)/wordalign: $(SRCDIR)/wordalign.cpp $(OBJDIR)/Corpus.o $(OBJDIR)/Model1.o $(OBJDIR)/Options.o $(OBJDIR)/Writer.o $(OBJDIR)/Utils.o
	$(CXX) $(CXX_FLAGS) $^ $(LD_FLAGS) -o $@ 

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXX_FLAGS) -c -o $@ $<
