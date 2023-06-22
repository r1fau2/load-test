#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "sockets.hpp"

EventSelector::~EventSelector()
{
    if(fd_array)
        delete[] fd_array;
}

void EventSelector::Add(FdHandler *h)
{
    int i;
    int fd = h->GetFd();
    if(!fd_array) {
        fd_array_len = fd > 15 ? fd + 1 : 16;
        fd_array = new FdHandler*[fd_array_len];
        for(i = 0; i < fd_array_len; i++)
            fd_array[i] = 0;
        max_fd = -1;
    }
    if(fd_array_len <= fd) {
        FdHandler **tmp = new FdHandler*[fd+1];
        for(i = 0; i <= fd ; i++)
            tmp[i] = i < fd_array_len ? fd_array[i] : 0;
        fd_array_len = fd + 1;
        delete[] fd_array;
        fd_array = tmp;
    }
    if(fd > max_fd)
        max_fd = fd;
    fd_array[fd] = h;
}

bool EventSelector::Remove(FdHandler *h)
{
    int fd = h->GetFd();
    if(fd >= fd_array_len || fd_array[fd] != h)
        return false;
    fd_array[fd] = 0;
    if(fd == max_fd) {
        while(max_fd >= 0 && !fd_array[max_fd])
            max_fd--;
    }
    return true;
}

void EventSelector::Run()
{
    struct timeval tvptr;
    tvptr.tv_sec = wait_sec_to_exit;
    tvptr.tv_usec = 0;
    quit_flag = false;

    do {
        int i;
        fd_set rds;
        FD_ZERO(&rds);
        for(i = 0; i <= max_fd; i++)
          if(fd_array[i])
			FD_SET(i, &rds);
        
        int res = select(max_fd+1, &rds, 0, 0, &tvptr);	// set timeout
       //int res = select(max_fd+1, &rds, 0, 0, 0);
        if(res < 0) {
            if(errno == EINTR)
                continue;
            else
                break;
        }
        if(res == 0)					// timeout quit
			quit_flag = true;
        if(res > 0) {
            for(i = 0; i <= max_fd; i++) {
                if(!fd_array[i])
                    continue;
                if(FD_ISSET(i, &rds))
					fd_array[i]->Handle();  
            }
        }
    } while(!quit_flag);
}

FdHandler::~FdHandler()
{
    if(own_fd)
        close(fd);
    printf("close %d\n", GetFd());     
}
