#include <stdio.h>
#include <netdb.h>	// for struct hostent
#include <stdlib.h>

#include "chat.hpp"

char name[max_out_line_length/8] = "bob";
char pswd[max_out_line_length/8] = "pswd-b";
char expr[max_out_line_length] = "232.45 / .56 +34  *( .456 /.45) ---2";

int main(int argc, char *argv[])
{
	if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       return 1;
    }
	struct hostent *server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        return 1;
    }
	int port = atoi(argv[2]);
    
    EventSelector *selector = new EventSelector;
    Master *master = new Master(server, port, name, pswd, expr, selector);
    
    int res = master->Connect();
    if(res) {				// res != 0 - error
        perror("connect");
        return 1;
    }
    selector->Run();
    master->OutStatistic();
    return 0;
}
