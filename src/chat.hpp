#ifndef CHAT_HPP_SENTRY
#define CHAT_HPP_SENTRY

#include <netinet/in.h>	// for struct sockaddr_in 

#include "sockets.hpp"

enum {
    max_out_line_length = 256,
    max_in_line_length = 2 * max_out_line_length + 64,
    max_send = 1024
};

enum fsm_states {fsm_in, fsm_pasw, fsm_work};

class Master;

class ChatSession : FdHandler {
    friend class Master;

    char buffer[max_in_line_length+1];
    int count; // send count  
    enum fsm_states state;

    Master *the_master;

    ChatSession(Master *a_master, int fd);
    ~ChatSession();

    virtual void Handle();
    void Send(const char *msg);
    void StateStep(const char *str);
};

class Master {
    struct sockaddr_in serv_addr;
    char* name;
    char* pswd;
    char* expr;
       
    EventSelector *the_selector;
    
    struct item {
        ChatSession *s;
        item *next;
    };
    item *first;
    
    int max_sess;
    struct item_stat {
		};
	item_stat *first_stat;
	    
public:
    Master(struct hostent *serv, int prt, char *nm, char *ps,
		char *ex, EventSelector *sel);
    ~Master();

    char *GetName() {return name;}
    char *GetPswd() {return pswd;}
    char *GetExpr() {return expr;}
    int Connect(); // in loop create and Add sessions
    void RemoveSession(ChatSession *s);
    void OutStatistic();
};

#endif
