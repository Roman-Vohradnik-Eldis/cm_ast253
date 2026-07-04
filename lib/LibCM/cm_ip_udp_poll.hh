#ifndef CM_IP_UDP_POLL_HH_
#define CM_IP_UDP_POLL_HH_

#include <unistd.h>
#include <sys/poll.h>

class Poll
{
    int pipe_fd[2];

public:
    Poll()
    {
        if (pipe(pipe_fd) != 0)
        {
            throw std::runtime_error(format("Failed to pipe(): %s", strerror(errno)));
        }
    }

    ~Poll()
    {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    // interrupt() se muze volat z libovolneho threadu. Vzbudi thread,
    // ktery je zablokovany na wait_for_event().
    void interrupt()
    {
        // Asi nevadi, ze je blokujici
        write(pipe_fd[1], "x", 1);
    }
    
    void wait_for_event(int sockfd, bool need_write, int64_t timeout_usec)
    {
        pollfd pfd[2];
	
        // Add socket
        pfd[0].fd = sockfd;
        pfd[0].events = POLLIN; // need_read
        if (need_write)
            pfd[0].events |= POLLOUT; // new_write	    
        pfd[0].revents = 0;

        // Add pipe
        pfd[1].fd = pipe_fd[0];
        pfd[1].events = POLLIN;
        pfd[1].revents = 0;

        int timeout_msec = timeout_usec / 1000;
        if (timeout_msec == 0)
            timeout_msec = 1;

        // cms_ns_if_print("cmip", 5, "poll: sleeping for %d msec, wait_for_write=%d", timeout_msec, need_write);
        int status = poll(pfd, 2, timeout_msec);

        if (status < 0)
        {
            cms_ns_if_print("cmip",5, "poll: error");
        }
        else if (status == 0)
        {
            cms_ns_if_print("cmip",5, "poll: timeout");
        }
        else
        {
            // status > 0: pocet socketu, ktere jsou pripraveny
            if (pfd[0].revents & POLLIN)
            {
                // cms_ns_if_print("cmip",5, "poll: have data to read");
            }
            if (pfd[1].revents & POLLIN)
            {
                // cms_ns_if_print("cmip",5, "poll: interrupted");
                char buffer[16];
                if (read(pipe_fd[0], buffer, sizeof buffer) < 0)
                {
                    cms_ns_if_print("cmip",1, "read from pipe failed");
                }
            }
        }
    }

private:
    Poll(Poll const &);
    Poll &operator=(Poll);
};

#endif // CM_IP_UDP_POLL_HH_
