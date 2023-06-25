#include <iostream>

#include "chat.hpp"

const char name[max_out_line_length/8] = "bob";
const char pswd[max_out_line_length/8] = "pswd-b";
const char expr[max_out_line_length] = "232.45 / .56 +34  *( .456 /.45) ---2";

int main(int argc, char *argv[])
{
	Init *init = Init::ReadStdIn(argc, argv);
	
    EventSelector *selector = new EventSelector;
    Master *master = new Master(init, name, pswd, expr, selector);
    
    master->Connect();
    selector->Run();
    master->OutStatistic();
 
    return 0;
}
