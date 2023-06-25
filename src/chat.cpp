#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>	// gettimeofday

#include "chat.hpp"

using namespace std;

Init::Init(struct hostent *svr, int prt, int nmcnt, int nmsend)
	: server(svr), port(prt), num_connect(nmcnt),
	num_send(nmsend)
{
}

Init *Init::ReadStdIn(int argc, char *argv[])
{
	if (argc < 5) {
		cerr << "usage: " << argv[0] << " ip-addr port number_of_connections"
			<< " number_of_send_per_connection\n";
		exit(1);
    }
    else if (atoi(argv[3]) <= 0 || atoi(argv[4]) <= 0) {
		cerr << "number_of_connections and number_of_send_per_connection must be > 0\n";
		exit(1);
    }
    struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        cerr << "ERROR, no such host\n";
        exit(1);
    }
	int port = atoi(argv[2]);
	int num_connect = atoi(argv[3]);
	int num_send = atoi(argv[4]);
	
	return new Init(server, port, num_connect, num_send);
}

ChatSession::ChatSession(Master *a_master, int fd)
    : FdHandler(fd, true), the_master(a_master),
    state(fsm_in), count(0)
{
}

void ChatSession::Send(const char *msg)
{
    write(GetFd(), msg, strlen(msg));
}

void ChatSession::Handle()
{
    int rc = read(GetFd(), buffer, sizeof(buffer));
    if(rc < 1) {				// EOF if rc == 0
        the_master->RemoveSession(this);
        return;
    }
    the_master->bytesReceive += rc;
    StateStep(buffer);
}

void ChatSession::StateStep(const char *str)
{
	count++;
	if (count > the_master->GetNumSend()) {
		the_master->RemoveSession(this);
		return;
	}
	
	char *wmsg = new char[max_out_line_length];

	switch(state) {
	case fsm_in:
        if (strstr(str, "\nlogin: ") == str)
        {
			state = fsm_in;
			sprintf(wmsg, "%s\n", the_master->GetName());
		}
		else {
			the_master->RemoveSession(this);
			return;	
		}
		state = fsm_pasw;
		break;
	case fsm_pasw:
	if (strstr(str, "\nРassword: ") == str)
        {
			state = fsm_in;
			sprintf(wmsg, "%s\n", the_master->GetPswd());
		}
		else {
			the_master->RemoveSession(this);
			return;
		}
		state = fsm_work;
		break;
	case fsm_work:
		if (strstr(str, "expr")) {		
			sprintf(wmsg, "%s\n", the_master->GetExpr());
		}
		else {
			the_master->RemoveSession(this);
			return;	
		}
	}
	Send(wmsg);
	the_master->bytesSend += strlen(wmsg);
	delete[] wmsg;
}

//////////////////////////////////////////////////////////////////////

Master::Master(Init *init, const char *nm, const char *ps, const char *ex,
	EventSelector *sel)
    : num_connect(init->num_connect), num_send(init->num_send),
    name(nm), pswd(ps), expr(ex), 
    the_selector(sel), first(0)
{
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	bcopy((char *)init->server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr,
		init->server->h_length);
    serv_addr.sin_port = htons(init->port);
}

Master::~Master()
{
    while(first) {
        item *tmp = first;
        first = first->next;
        the_selector->Remove(tmp->s);
        delete tmp->s;
        delete tmp;
    }
}

void Master::RemoveSession(ChatSession *s)
{
    the_selector->Remove(s);
    item **p;
    for(p = &first; *p; p = &((*p)->next)) {
        if((*p)->s == s) {
            item *tmp = *p;
            *p = tmp->next;
            delete tmp->s;
            delete tmp;
            return;
        }
    }
}

void Master::Connect()
{
	int sd, n;
    for (int i = 0; i < num_connect; i++) {	
		sd = socket(AF_INET, SOCK_STREAM, 0);
		if(sd == -1) {
			cerr << "ERROR, socket not created\n";
			exit(1);
		}	
		int opt = 1;
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		
		if (connect(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			cerr << "ERROR, connection not created\n";
			exit(1);
		}
		item *p = new item;
		p->next = first;
		p->s = new ChatSession(this, sd);
		first = p;

		the_selector->Add(p->s);
	}
	
	cout << "\n************ StartSession ************\n\n"
		<< "open " << num_connect << " connection\n"
		<< "StartRun (" << num_send  << " sends per conection)...\n";
	gettimeofday(&start, NULL);
}

void Master::OutStatistic()
{
	gettimeofday(&end, NULL);
    long seconds = (end.tv_sec - start.tv_sec);
    double micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
	long minutes = (micros/1000000 - 1)/60;
	cout << "close " << num_connect << " connection"
		<< "\n\n************ EndSession **************"
		<< "\n\nBytes sent: " << bytesSend 
		<< ", bytes received: " << bytesReceive
		<< ".\nElapsed time: ";
	if (minutes)
		cout << minutes << " mins ";
	cout << std::setprecision(3) << ((micros/1000000 - wait_sec_to_exit) - 60 * minutes)
		<< " secs " << "run, " << wait_sec_to_exit << " sec wait.\n";
	if (bytesSend && bytesReceive)	
		cout << "Average time between sends: "
			<< (micros/1000000 - 1)/num_connect/num_send << " secs.\n\n";
}
