//
//  REPL.cpp
//  REPL
//
//  Created by BlueCocoa on 16/3/3.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#include "REPL.h"

#include <editline/readline.h>
#include <iostream>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <sys/select.h>
#include <unistd.h>

int REPL::key3(unsigned char * input) {
    if (input[2] == 128 && input[1] == 156 && input[0] == 239) {
        // OS X - Xcode - arrow up
        return KEY_ARROW_UP;
    } else if (input[2] == 65 && input[1] == 91 && input[0] == 27) {
        // OS X - Terminal - arrow up
        return KEY_ARROW_UP;
    } else if (input[2] == 129 && input[1] == 156 && input[0] == 239) {
        // OS X - Xcode - arrow down
        return KEY_ARROW_DOWN;
    } else if (input[2] == 66 && input[1] == 91 && input[0] == 27) {
        // OS X - Terminal - arrow down
        return KEY_ARROW_DOWN;
    } else if (input[2] == 130 && input[1] == 156 && input[0] == 239) {
        // OS X - Xcode - arrow left
        return KEY_ARROW_LEFT;
    } else if (input[2] == 68 && input[1] == 91 && input[0] == 27) {
        // OS X - Terminal - arrow left
        return KEY_ARROW_LEFT;
    } else if (input[2] == 131 && input[1] == 156 && input[0] == 239) {
        // OS X - Xcode - arrow right
        return KEY_ARROW_RIGHT;
    } else if (input[2] == 67 && input[1] == 91 && input[0] == 27) {
        // OS X - Terminal - arrow right
        return KEY_ARROW_RIGHT;
    }
    return KEY_UNKNOWN;
}

REPL& REPL::add_history(bool cache) {
    if (!cache) {
        // increase the number of history
        this->_history_count++;
        
        // set current history index
        this->_current_history_index = this->_history_count;
        
        ::add_history(strdup(this->terminal_buffer.buffer.c_str()));
        
        DEBUG_PRINT("Add %s to history", this->terminal_buffer.buffer.c_str());
    } else {
        this->terminal_buffer.cached = strdup(this->terminal_buffer.buffer.c_str());
        DEBUG_PRINT("Line cached!");
    }
    return *this;
}

REPL& REPL::remove_history() {
    if (this->terminal_buffer.cached) {
        free((void *)this->terminal_buffer.cached);
        this->terminal_buffer.cached = NULL;
        this->_current_history_index--;
        this->direction = 0;
        DEBUG_PRINT("Remove cached line");
    }
    return *this;
}

REPL::REPL () {
    // using history feature
    using_history();
    
    // store old settings
    tcgetattr(0, &this->stored_settings);
    
    struct termios new_settings = this->stored_settings;
    
    // set to non-canonical mode
    new_settings.c_lflag &= ~(ICANON | IEXTEN);
    
    // timer
    new_settings.c_cc[VTIME] = 0;
    
    // one char
    new_settings.c_cc[VMIN] = 1;
    
    // apply new settings
    if (tcsetattr(0, TCSANOW, &new_settings) != 0) {
        perror("[ERROR] tcsetattr:");
        exit(-1);
    }
    
    DEBUG_PRINT("new terminal settings applied");
}

REPL::~REPL () {
    // restore terminal settings
    if (tcsetattr(0, TCSANOW, &this->stored_settings) != 0) {
        perror("[ERROR] tcsetattr:");
    }
}

std::string REPL::prompt() {
    return this->_prompt;
}

REPL& REPL::set_prompt(const std::string &prompt) {
    this->_prompt = prompt;
    this->terminal_buffer.set_prompt(prompt);
    return *this;
}

REPL& REPL::readline(const std::function<bool(REPL& self, std::string line)>& callback) {
    this->_readline = callback;
    return *this;
}

REPL& REPL::start() {
    this->_continue = true;
    this->terminal_thread = std::thread([&](){
        // print prompt
        this->terminal_buffer.print(REPL::stringbuffer::PR_PROMPT_ONLY);
        
        // set STDIN_FILENO to terminal_fd
        fd_set terminal_fd;
        FD_ZERO(&terminal_fd);
        FD_SET(STDIN_FILENO, &terminal_fd);
        if (!FD_ISSET(STDIN_FILENO, &terminal_fd)) {
            perror("[ERROR] Cannot select stdin");
            exit(-1);
        }
        
        while (this->_continue) {
            // timeout for select
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            // capture terminal_fd
            fd_set terminal_read = terminal_fd;
            
            // wait for reading
            int terminal_ret = select(FD_SETSIZE, &terminal_read, NULL, NULL, &timeout);
            
            if (terminal_ret == 0) {
                // timeout
                continue;
            } else if (terminal_ret == -1) {
                // error
                perror("[ERROR] select:");
            } else {
                // read
                if (FD_ISSET(0, &terminal_read)) {
                    int count;
                    
                    // read through ioctl
                    ioctl(0, FIONREAD, &count);
                    
                    // there are some characters available to read
                    if (count != 0) {
                        // malloc memory
                        unsigned char * input = (unsigned char *)malloc(sizeof(unsigned char) * count + 1);
                        
                        // set input to zero
                        bzero(input, sizeof(unsigned char) * count + 1);
                        
                        // read from stdin
                        count = (int)read(STDIN_FILENO, input, count);
                        
                        // did read 1 byte
                        if (count == 1)
                        {
                            // user entered return key
                            if (input[count - 1] == '\n')
                            {
                                // add input buffer to history
                                this->add_history();
                                
                                // callback
                                this->_continue = this->_readline(*this, this->terminal_buffer.buffer);
                                
                                // print prompt if allowed
                                if (this->_continue) {
                                    this->terminal_buffer.clear().print(REPL::stringbuffer::PR_PROMPT_ONLY);
                                    this->_current_history_index = this->_history_count;
                                }
                            } // user entered return key
                            else
                            { // current key is not return
                                // current key is backspace
                                if (input[count - 1] == KEY_BACKSPACE)
                                {
                                    // if there are one or more characters in input buffer
                                    if (this->terminal_buffer.buffer.length() > 0)
                                    {
                                        this->remove_history();
                                        this->terminal_buffer.erase(this->terminal_buffer.buffer.length() - 1);
                                        this->terminal_buffer.print(REPL::stringbuffer::PR_WITH_CLEAN, 1);
                                    }
                                } // current key is backspace
                                else
                                { // current key is not backspace
                                    this->remove_history();
                                    this->terminal_buffer.push_back(input[count - 1]).print(REPL::stringbuffer::PR_WITH_CLEAN);
                                }
                            } // current key is not return
                        } // did read 1 byte
                        else if (count == 3)
                        { // did read 3 byte
                            // switch keys
                            switch (key3(input)) {
                                case KEY_ARROW_UP: {
                                    if (this->terminal_buffer.cached == NULL) {
                                        DEBUG_PRINT("Caching current line");
                                        this->add_history(true);
                                        this->terminal_buffer.print(REPL::stringbuffer::PR_NOTHING_BUT_CLEAN, this->terminal_buffer.buffer.length());
                                    }
                                    else if (this->_current_history_index == this->_history_count + 1)
                                    {
                                        if (this->direction == 1)
                                        {
                                            if (this->terminal_buffer.cached == NULL) {
                                                this->_current_history_index -= 2;
                                            } else {
                                                this->_current_history_index -= 1;
                                            }
                                        }
                                        else
                                        {
                                            // if index points to the newest one, decrese by one
                                            this->_current_history_index = this->_history_count;
                                            this->terminal_buffer.print(REPL::stringbuffer::PR_NOTHING_BUT_CLEAN, this->terminal_buffer.buffer.length());
                                            this->_current_history_index = this->_history_count;
                                        }
                                    }
                                    
                                    DEBUG_PRINT("Getting previous history at index %d", this->_current_history_index);
                                    
                                    // get hist entry
                                    HIST_ENTRY * entry = history_get(this->_current_history_index);
                                    if (entry) {
                                        this->terminal_buffer.replace_buffer(entry->line).print(REPL::stringbuffer::PR_WITH_CLEAN, 4);
                                        this->_current_history_index--;
                                        if (this->_current_history_index == 0) {
                                            this->_current_history_index = 1;
                                        }
                                        this->direction = -1;
                                    } else {
                                        this->terminal_buffer.print(REPL::stringbuffer::PR_NOTHING_BUT_CLEAN, 4);
                                        this->direction = 0;
                                    }
                                    
                                    DEBUG_PRINT("direction is %d, index is %d", this->direction, this->_current_history_index);
                                    break;
                                }
                                case KEY_ARROW_DOWN: {
                                    if (this->_current_history_index == 1 && this->terminal_buffer.cached)
                                    {
                                        // if index points to the first one, increase by one
                                        this->_current_history_index = 2;
                                    }
                                    else if (this->direction == -1)
                                    {
                                        this->_current_history_index += 2;
                                    }
                                    
                                    DEBUG_PRINT("Getting next history at index %d, count is %d", this->_current_history_index, this->_history_count);
                                    
                                    if (this->_current_history_index >= this->_history_count + 1) {
                                        if (this->terminal_buffer.cached) {
                                            this->terminal_buffer.replace_buffer(this->terminal_buffer.cached);
                                        }
                                        this->terminal_buffer.print(REPL::stringbuffer::PR_WITH_CLEAN, 4);
                                        this->_current_history_index = this->_history_count + 1;
                                    } else {
                                        // get hist entry
                                        HIST_ENTRY * entry = history_get(this->_current_history_index);
                                        if (entry) {
                                            this->terminal_buffer.replace_buffer(entry->line).print(REPL::stringbuffer::PR_WITH_CLEAN, 4);
                                            this->direction = 1;
                                        } else {
                                            this->terminal_buffer.print(REPL::stringbuffer::PR_NOTHING_BUT_CLEAN, 4);
                                            this->direction = 0;
                                        }
                                        this->_current_history_index++;
                                    }
                                    
                                    
                                    DEBUG_PRINT("direction is %d, index is %d", this->direction, this->_current_history_index);
                                    
                                    break;
                                }
                                case KEY_ARROW_LEFT: {
                                    this->terminal_buffer.left().print(REPL::stringbuffer::PR_WITH_CLEAN, 4);
                                    break;
                                }
                                case KEY_ARROW_RIGHT: {
                                    this->terminal_buffer.right().print(REPL::stringbuffer::PR_WITH_CLEAN, 4);
                                    break;
                                }
                                default:
                                    break;
                            }
                        } // did read 3 byte
                        else
                        { // did read some byte
                            this->remove_history();
                            this->terminal_buffer.append((char *)input).print(REPL::stringbuffer::PR_WITH_CLEAN);
                        } // did read some byte
                    }
                } // read
                else {
                    DEBUG_PRINT("FD_ISSET failed");
                }
            }
        }
        this->_condition->notify_one();
    });
    this->terminal_thread.detach();
    return *this;
}

REPL& REPL::condition(const std::condition_variable& condition) {
    this->_condition = const_cast<std::condition_variable *>(&condition);
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::set_prompt(const std::string &prompt) {
    this->prompt = prompt;
    this->cursor = this->prompt.length();
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::print(print_t pr, ssize_t clean) {
    switch (pr) {
        case PR_PROMPT_ONLY:
            std::cout<<this->prompt<<std::flush;
            break;
        case PR_WITH_CLEAN: {
            clean += this->prompt.length() + this->last_buffer_length + this->buffer.length() + 1;
            long backspace = this->prompt.length() + this->buffer.length() - this->cursor;
            
            for (int i = 0; i < clean; i++) {
                std::cout<<'\b';
            }
            for (int i = 0; i < clean; i++) {
                std::cout<<' ';
            }
            for (int i = 0; i < clean; i++) {
                std::cout<<'\b';
            }
            
            std::cout<<this->prompt<<this->buffer<<std::flush;
            
            for (long i = 0; i < backspace; i++) {
                std::cout<<'\b';
            }
            break;
        }
        case PR_NOTHING_BUT_CLEAN: {
            for (int i = 0; i < clean; i++) {
                std::cout<<'\b';
            }
            for (int i = 0; i < clean; i++) {
                std::cout<<' ';
            }
            for (int i = 0; i < clean; i++) {
                std::cout<<'\b';
            }
            break;
        }
        default:
            break;
    }
    std::cout<<std::flush;
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::left() {
    if (this->cursor > this->prompt.length()) this->cursor--;
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::right() {
    if (this->cursor < this->prompt.length() + this->buffer.length()) this->cursor++;
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::append(const std::string& append, bool current_position) {
    if (current_position) {
        this->buffer.insert(this->cursor - this->prompt.length(), append);
        this->cursor += append.length();
    } else {
        this->buffer.append(append);
        this->cursor = this->prompt.length() + this->buffer.length();
    }
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::push_back(char c) {
    char * append = (char *)malloc(sizeof(char) * 2);
    append[0] = c;
    append[1] = '\0';
    this->buffer.insert(this->cursor - this->prompt.length(), append);
    this->cursor++;
    free(append);
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::clear() {
    this->last_buffer_length = -1;
    this->buffer.clear();
    this->cursor = this->prompt.length();
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::erase(ssize_t pos) {
    if (pos <= this->cursor - this->prompt.length()) {
        this->cursor--;
    }
    this->buffer.erase(pos);
    return *this;
}

REPL::stringbuffer& REPL::stringbuffer::replace_buffer(const std::string &buffer) {
    this->last_buffer_length = this->buffer.length();
    this->buffer = buffer;
    this->cursor = this->prompt.length() + this->buffer.length();
    return *this;
}
