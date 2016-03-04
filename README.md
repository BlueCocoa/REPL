# REPL
A lightweight asynchronous REPL library with history feature support! 

Tested on OS X / Raspbian Linux / iOS. 

### Compile
```$ make && sudo make install```

To enable debug infomation, 
```$ CPPFLAGS='-DDEBUG=1' make && sudo make install```

### Dependencies
- [edit](https://sourceforge.net/projects/libedit/)
- pthread
- [termcap](http://ftp.gnu.org/gnu/termcap/)

### Usage

		REPL ouolang;
		ouolang
		.set_prompt("> ") // You can set a prompt for your language
		.readline([&](REPL& self, std::string line) -> bool {
		    // You can read user input from this callback
		    std::cout<<"did read: "<<line<<std::endl;
		    
		    // return false to stop
		    return true;
		})
		.start(); // start REPL
		
		// main thread goes to sleep
		std::mutex mutex;
		std::condition_variable cond;
		std::thread t = std::thread([&] {
		    std::unique_lock<std::mutex> lock(mutex);
		    cond.wait(lock);
		    std::cout<<"REPL notified"<<std::endl;
		});
		
		// You can set condition varible to REPL object, 
		// and it'll call notify_one() when retrives false in readline callback
		ouolang.condition(cond);
		t.join();

### Screenshots
#### Raspbian Linux
![Raspbian Linux](https://raw.githubusercontent.com/BlueCocoa/REPL/master/screenshot-Raspbian%20Linux.png)

#### iOS
![iOS](https://raw.githubusercontent.com/BlueCocoa/REPL/master/screenshot-iOS.png)

#### OS X
See screenrecord.mp4
