BOOST_PATH = /home/ales/boost/

CXX_FLAGS = -O3 -I$(BOOST_PATH)/include/ -L$(BOOST_PATH)/lib/

all: wordalign

%: %.cpp
	$(CXX) $(CXX_FLAGS) -lboost_program_options -o $@ $<
