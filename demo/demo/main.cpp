//
//  main.cpp
//  demo
//
//  Created by BlueCocoa on 16/3/3.
//  Copyright © 2016年 BlueCocoa. All rights reserved.
//

#include <iostream>
#include <thread>
#include <functional>
#include <condition_variable>
#include "REPL.h"

using namespace std;

const char * const prompt = "> ";

int main(int argc, const char * argv[]) {
    REPL ouolang;
    ouolang
    .set_prompt(prompt)
    .readline([&](REPL& self, std::string line) -> bool {
        std::cout<<"did read: "<<line<<std::endl;
        return true;
    })
    .start();

    std::mutex mutex;
    std::condition_variable cond;
    std::thread t = std::thread([&] {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock);
        std::cout<<"REPL notified"<<std::endl;
    });
    ouolang.condition(cond);
    t.join();
    return 0;
}
