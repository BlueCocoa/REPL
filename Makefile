CXX = g++
AR = ar
RANLIB = ranlib

PREFIX ?= /usr/local
SOURCE = REPL.cpp
HEADER = REPL.h
OBJECT = REPL.o
TARGET = librepl.a

LDFLAGS += -L/usr/local/lib -ledit -lpthread -ltermcap
CPPFLAGS += -I/usr/local/include -std=c++14 -fPIC -c -o $(OBJECT)
RANLIBFLAGS += $(TARGET)

$(TARGET) : 
	$(CXX) $(CPPFLAGS) $(SOURCE)
	case `uname` in Darwin*) $(AR) crv $(TARGET) $(OBJECT) ;; *) $(AR) cr $(TARGET) && $(AR) crv $(TARGET) $(OBJECT) ;; esac
	$(RANLIB) $(RANLIBFLAGS)

demo : $(TARGET)
	$(CXX) $(LDFLAGS) $(OBJECT) -L./ -lrepl -std=c++14 -fPIC main.cpp -o demo
    
install :
	install -m 775 $(TARGET) $(PREFIX)/lib
	install -m 644 $(HEADER) $(PREFIX)/include

uninstall :
	rm -f $(PREFIX)/lib/$(TARGET)
	rm -f $(PREFIX)/include/$(HEADER)

clean :
	-rm -f $(OBJECT)
	-rm -f $(TARGET)
