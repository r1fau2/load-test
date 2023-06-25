#ifndef CHAT_HPP_SENTRY
#define CHAT_HPP_SENTRY

#include <netdb.h>		// struct hostent
#include <netinet/in.h>	// struct sockaddr_in 

#include "sockets.hpp"

enum {
    max_out_line_length = 256,
    max_in_line_length = 2 * max_out_line_length + 64
};

enum fsm_states {fsm_in, fsm_pasw, fsm_work};

class Master;

class Init {
	friend class Master;
	
	struct hostent *server;
	int port;
	int num_connect;
	int num_send;
		
    Init(struct hostent *svr, int prt, int cnt, int snd);
public:	
	static Init *ReadStdIn(int argc, char *argv[]);
};

class ChatSession : FdHandler {
    friend class Master;

    char buffer[max_in_line_length+1];
    int count; // send count  
    enum fsm_states state;

    Master *the_master;

    ChatSession(Master *a_master, int fd);

    virtual void Handle();
    void Send(const char *msg);
    void StateStep(const char *str);
};

class Master {
    struct sockaddr_in serv_addr;
    int num_connect;
    int num_send;
    const char* name;
    const char* pswd;
    const char* expr;
   	struct timeval start;
   	struct timeval end;
           
    EventSelector *the_selector;
    
    struct item {
        ChatSession *s;
        item *next;
    };
    item *first;
	    
public:
    unsigned long int bytesSend;
    long int bytesReceive;
    Master(Init *init, const char *nm, const char *ps, const char *ex,
		EventSelector *sel);
    ~Master();

    int GetNumSend() {return num_send;}
    const char *GetName() {return name;}
    const char *GetPswd() {return pswd;}
    const char *GetExpr() {return expr;}
    void Connect();
    void RemoveSession(ChatSession *s);
    void OutStatistic();
};

#endif
