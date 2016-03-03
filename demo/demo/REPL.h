//
//  REPL.hpp
//  REPL
//
//  Created by BlueCocoa on 16/3/3.
//  Copyright © 2016 BlueCocoa. All rights reserved.
//

#ifndef REPL_H
#define REPL_H

#include <string>
#include <termios.h>
#include <thread>
#include <functional>
#include <condition_variable>

#ifdef DEBUG
#define DEBUG_PRINT(...) {\
    fprintf(stderr,"[REPL_DEBUG] %s: Line %d: ", __FILE__, __LINE__);\
    fprintf(stderr, __VA_ARGS__);\
    fprintf(stderr,"\n");\
}
#else
#define DEBUG_PRINT(...) (static_cast<void *>(0))
#endif

class REPL {
public:
    /**
     *  @brief Constructor
     */
    REPL ();
    
    /**
     *  @brief Destructor
     */
    ~REPL ();
    
    /**
     *  @brief Get current prompt
     *
     *  @return Current prompt
     */
    std::string prompt();
    
    /**
     *  @brief Set prompt
     *
     *  @param prompt prompt for terminal
     *
     *  @return *this
     */
    REPL& set_prompt(const std::string &prompt);
    
    /**
     *  @brief Readline
     *
     *  @param callback user input callback
     *
     *  @discussion Called when when user entered return key.
     *              Return false to stop
     *
     *  @return *this
     */
    REPL& readline(const std::function<bool(REPL& self, std::string line)>& callback);
    
    /**
     *  @brief Start REPL
     *
     *  @return *this
     */
    REPL& start();
    
    /**
     *  @brief Notify
     *
     *  @param condition will call notify_one() when readline callback returns false
     *
     *  @return *this
     */
    REPL& condition(const std::condition_variable& condition);
private:
    /// Buffer for input
    class stringbuffer {
        friend class REPL;
        std::string prompt = "";
        std::string buffer = "";
        unsigned long long cursor = 0;
    private:
        /**
         *  @brief Set prompt
         *
         *  @return *this
         */
        stringbuffer& set_prompt(const std::string &prompt);
        
        /**
         *  @brief Replace all the buffer
         *
         *  @param buffer new buffer
         *
         *  @return *this
         */
        stringbuffer& replace_buffer(const std::string &buffer);
        
        enum print_t {
            PR_PROMPT_ONLY,
            PR_WITH_CLEAN
        };
        /**
         *  @brief Print current buffer to terminal
         *
         *  @param clean 0 for printing at current line without cleaning dirty characters
         *               -1 for printing at a new line
         *               -2 for printing prompt only without cleaning
         *               >0 for printing at current line with cleaning # dirty characters
         *
         *  @return *this
         */
        stringbuffer& print(print_t pr, ssize_t clean = 0);
        
        /**
         *  @brief Cursor goes left
         *
         *  @return *this
         */
        stringbuffer& left();
        
        /**
         *  @brief Cursor goes right
         *
         *  @return *this
         */
        stringbuffer& right();
        
        /**
         *  @brief Append buffer to current buffer
         *
         *  @param append           buffer to append
         *  @param current_position true for appending at cursor position
         *                          false for appending at the end
         *
         *  @return *this
         */
        stringbuffer& append(const std::string& append, bool current_position = true);
        
        /**
         *  @brief Push a character to cursor position
         *
         *  @param c character to be pushed
         *
         *  @return *this
         */
        stringbuffer& push_back(char c);
        
        /**
         *  @brief Clear all buffer
         *
         *  @return *this
         */
        stringbuffer& clear();
        
        /**
         *  @brief Erase a character at given position
         *
         *  @return *this
         */
        stringbuffer& erase(ssize_t pos);
    };
    
    /**
     *  @brief Terminal buffer
     */
    stringbuffer terminal_buffer;
    
    enum key_t : int {
        KEY_BACKSPACE = 127,
        KEY_RETURN = '\n',
        KEY_ARROW_UP,
        KEY_ARROW_DOWN,
        KEY_ARROW_LEFT,
        KEY_ARROW_RIGHT,
        KEY_UNKNOWN
    };
    
    /**
     *  @brief Store terminal settings when REPL gets initialised
     */
    struct termios stored_settings;
    
    /**
     *  @brief Stored prompt
     */
    std::string _prompt;
    
    /**
     *  @brief Buffer for user input
     */
    //    std::string _buffer;
    
    /**
     *  @brief Callback when user entered return key
     */
    std::function<bool(REPL& self, std::string line)> _readline;
    
    /**
     *  @brief The count of history
     */
    int _history_count = 0;
    
    /**
     *  @brief The index of current HIST_ENTRY
     */
    int _current_history_index = 0;
    
    /**
     *  @brief Work thread
     */
    std::thread terminal_thread;
    
    /**
     *  @brief Should continue
     */
    bool _continue = true;
    
    /**
     *  @brief Uses condition
     */
    std::condition_variable * _condition;
    
    /**
     *  @brief Return the corresponding key for given input
     *
     *  @param input ioctl reads
     *
     *  @return The corresponding key
     */
    int key3(unsigned char * input);
};

#endif /* REPL_H */
