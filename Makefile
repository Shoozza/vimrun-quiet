CC=g++
CFLAGS=-O2 -c -Wall
LDFLAGS=-static -static-libgcc -static-libstdc++ -Wl,--subsystem,windows
SOURCES=vimrun.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=vimrun.exe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
