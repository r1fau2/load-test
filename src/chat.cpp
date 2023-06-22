#include <stdio.h>  // for sprintf
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <netdb.h>	// for struct hostent

#include <sys/socket.h>
//#include <sys/select.h> // for FD_SETSIZE
//#include <netinet/in.h>
//#include <sys/time.h> // for gettimeofday()

#include "chat.hpp"

ChatSession::ChatSession(Master *a_master, int fd)
    : FdHandler(fd, true), the_master(a_master),
    state(fsm_in), count(0)
{
}

ChatSession::~ChatSession()
{
    //if(name)
    //    delete[] name;
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
    StateStep(buffer);
}

void ChatSession::StateStep(const char *str)
{
	count++;
	//printf("\n------------------\nsd = %d\tcount = %d\n", GetFd(), count);
	//printf("%s\n", str);
	if (count > the_master->GetMaxSend()) {
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
	//printf("%s", wmsg);
	delete[] wmsg;
}
/*

int bytesRead = 0, bytesWritten = 0;
    struct timeval startses, endses;
    gettimeofday(&startses, NULL);

char buffer[256];
    while (true) {
		bzero(buffer, 256);
		n = read(sd, buffer, 256);
		if (n < 0) 
			return 1;
		bytesRead +=n;
		printf("%s", buffer);
    
		bzero(buffer,256);
		fgets(buffer,256,stdin);
		if (strstr(buffer, "quit"))
			break;
		n = write(sd,buffer,strlen(buffer));
		if (n < 0)
			return 1;
		bytesWritten += n;	
	}
gettimeofday(&endses, NULL);
// close(sd);
*/

//////////////////////////////////////////////////////////////////////

Master::Master(struct hostent *serv, int prt, int maxcnt, int maxsend,
	char *nm, char *ps, char *ex, EventSelector *sel)
    : max_connect(maxcnt), max_send_pc(maxsend),
    name(nm), pswd(ps), expr(ex), 
    the_selector(sel), first(0), first_stat(0)
{
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)serv->h_addr, (char *)&serv_addr.sin_addr.s_addr,
		serv->h_length);
    serv_addr.sin_port = htons(prt);
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
		//PrintList();
        if((*p)->s == s) {
            item *tmp = *p;
            *p = tmp->next;
            delete tmp->s;
            delete tmp;
            return;
        }
    }
}

/*
void Master::PrintList()
{
	int i;
	item *p;
	printf(".......\n");
	for(i = 1, p = first; p; i++, p = p->next)
		printf("list [%d] = %p\n", i, p);
	printf(".......\n");	
}
*/

int Master::Connect() // in loop create and Add sessions
{
	int sd, n;
      
	/*for (int i = 0; i < FD_SETSIZE -3; i++) {	
     	printf("FD_SETSIZE = %d\n", FD_SETSIZE);
     	printf("max_connect = %d\n", max_connect);
     	printf("max_send_pc = %d\n", max_send_pc);
    */ 	
    
    for (int i = 0; i < max_connect; i++) {	
		sd = socket(AF_INET, SOCK_STREAM, 0);
		printf("sd = %d\n", sd);
		if(sd == -1)
			return 1;
		int opt = 1;
		setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		if (connect(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
			return 1;
     
		item *p = new item;
		p->next = first;
		p->s = new ChatSession(this, sd);
		first = p;
		//PrintList();

		the_selector->Add(p->s);
	}
	printf("--------\n");
	return 0;
}

void Master::OutStatistic()
{

}
