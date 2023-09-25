# Generic Makefile with automatic dependencies management

# Choose your compiler g++/clang++
CXX        = gcc
CXXFLAGS   = -Wall -Wextra
DEPFLAGS   = -MMD
LDFLAGS    = 
# Replace your_files.cpp with the list of cpp files that need to be compiled
# Ex: file1.cpp file2.cpp file3.cpp
SRCS       = cache_sim.c
OBJS       = $(SRCS:.c=.o)
# Replace your_app with the name you want to give to your executable file
TARGET     = cache_sim 
DEPS	   = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) -lm

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c  $<

clean:
	rm -f *.o *.d

mrproper: clean
	rm -f $(TARGET)

exe: $(TARGET)
	./$(TARGET)

-include $(DEPS)
