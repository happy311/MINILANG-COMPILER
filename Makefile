CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SOURCES  = $(wildcard src/*.cpp)

minicc: $(SOURCES) $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o minicc

run-examples: minicc
	@for f in examples/*.ml; do echo "--- $$f"; ./minicc $$f; done

clean:
	rm -f minicc
