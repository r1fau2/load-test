#include <stdio.h>
#include <netdb.h>	// for struct hostent
#include <stdlib.h>

#include "chat.hpp"

char name[max_out_line_length/8] = "bob";
char pswd[max_out_line_length/8] = "pswd-b";
char expr[max_out_line_length] = "232.45 / .56 +34  *( .456 /.45) ---2";

int main(int argc, char *argv[])
{
	if (argc < 5) {
       fprintf(stderr,"usage %s ip-addr port max_connect max_send_per_connect\n", argv[0]);
       return 1;
    }
	struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return 1;
    }
	int port = atoi(argv[2]);
	int max_connect = atoi(argv[3]);
	int max_send_pc = atoi(argv[4]);
    
    EventSelector *selector = new EventSelector;
    Master *master = new Master(server, port, max_connect, max_send_pc,
		name, pswd, expr, selector);
    
    int res = master->Connect();
    if(res) {				// res != 0 - error
        perror("connect");
        return 1;
    }
    selector->Run();
    master->OutStatistic();
    return 0;
}
