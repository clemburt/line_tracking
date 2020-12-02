CXX = g++
RM = rm

CFLAGS = `pkg-config --cflags opencv` -pthread -lrt `xeno-config --cflags --alcheny --skin=posix`
LIBS = `pkg-config --libs opencv` -pthread -lrt `xeno-config --ldflags --alcheny --skin=posix`
LDLIBS = -lwiringPi

TARGET = tracking

OBJS = tracking.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDLIBS) -o $@ $^ $(CFLAGS) $(LIBS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $<

clean:
	$(RM) -f *.o $(TARGET)
