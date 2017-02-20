CXXFLAGS = -Wall -Wextra -pedantic -std=c++14 -O2 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector
OBJ = 
EXECS = disk

all: $(EXECS)

$(OBJ): %.o : %.cc %.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(EXECS): % : %.cc $(OBJ)
	$(CXX) -o $@ $(CXXFLAGS) $^

clean:
	-rm -f $(OBJ) $(EXECS)