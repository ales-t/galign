BOOST_PATH = /home/ales/boost
OBJDIR = bin
SRCDIR = src

CXX_FLAGS = -O3 -I$(BOOST_PATH)/include/

all: $(OBJDIR)/wordalign

clean:
	rm -rf $(OBJDIR)

$(OBJDIR)/wordalign: $(SRCDIR)/wordalign.cpp $(OBJDIR)/Corpus.o $(OBJDIR)/Model1.o $(OBJDIR)/Options.o $(OBJDIR)/Writer.o $(OBJDIR)/Utils.o
	$(CXX) $(CXX_FLAGS) $^ -lz $(BOOST_PATH)/lib/libboost_program_options.a $(BOOST_PATH)/lib/libboost_iostreams.a -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXX_FLAGS) -c -o $@ $<
