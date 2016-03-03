CXX = g++
AR = ar
RANLIB = ranlib

PREFIX ?= /usr/local
SOURCE = REPL.cpp
HEADER = REPL.h
OBJECT = REPL.o
TARGET = librepl.a

LDFLAGS += -lreadline
CPPFLAGS += -std=c++14 -fPIC -c -o $(OBJECT)
ARFLAGS += -crv $(TARGET) $(OBJECT)
RANLIBFLAGS += $(TARGET)

$(TARGET) : 
	$(CXX) $(CPPFLAGS) $(SOURCE)
	$(AR) $(ARFLAGS)
	$(RANLIB) $(RANLIBFLAGS)

demo : $(TARGET)
	$(CXX) -std=c++14 -fPIC main.cpp $(LDFLAGS) -L./ -lrepl  -o main
    
install :
	install -m 775 $(TARGET) $(PREFIX)/lib
	install -m 644 $(HEADER) $(PREFIX)/include

uninstall :
	rm -f $(PREFIX)/lib/$(TARGET)
	rm -f $(PREFIX)/include/$(HEADER)

clean :
	-rm -f $(OBJECT)
	-rm -f $(TARGET)
