
#SYSTEMC = /home/pufferfish/systemc230 
SYSTEMC = lib
LDFLAGS = -Llib_os -lsystemc -lpthread
#LDFLAGS = -L$(SYSTEMC)/liblinux64 -lsystemc -fpermissive -Wno-deprecated
CXXFLAGS = -Wall -ggdb -I$(SYSTEMC)/include -std=c++11 

TARGET = PCIeSystem.out
SOURCES = $(shell echo *.cpp)

HEADERS = $(shell echo *.h)	#bind all headerfiles to the HEADERS variable, must be in the same folder
#OBJECTS = $(SOURCES:.c=.o)	#All objects are named the same as their cpp files, 

all:
	g++ $(CXXFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)
# 	./a.out

#counterrors:
#	{ g++ 2>&1 | tee /proc/self/fd/3 | grep Error | wc -l } 3>&1 $(CXXFLAGS) $(SOURCES) $(LDFLAGS) -o $(TARGET)

clean:
	-rm -f *.o
	-rm -f *.out


##g++ -Wno-deprecated -I/home/pufferfish/systemc230/include -fpermissive -Wall counter_tb.cpp -L/home/pufferfish/systemc230/liblinux64 -lsystemc -o counter.out
