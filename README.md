# REPL
A lightweight asynchronous REPL library with history feature support! 

### Compile
```$ make && sudo make install```

To enable debug infomation, 
```$ CPPFLAGS='-DDEBUG=1' make && sudo make install```

### Dependencies
- edit
- pthread
- termcap

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
