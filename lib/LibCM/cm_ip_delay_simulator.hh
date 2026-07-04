#ifndef CM_IP_DELAY_SIMULATOR_HH_
#define CM_IP_DELAY_SIMULATOR_HH_

#include "cm_ip.hh"
#include "cm_debug.hh"
#include <deque>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>

struct DelayedFrame
{
    int sockfd;
    sockaddr_in dest;
    std::string message;
    int64_t time_added;
};

class DelaySimulator
{
    std::deque<DelayedFrame> queue;
    int64_t add_delay;
    int64_t debug_probability;

public:
    void set_delay(int64_t delay) { add_delay = delay; }
    void set_debug_probability(int64_t p) { debug_probability = p; }

    int64_t get_next_event_time(int64_t time_now) const
    {
        if (queue.empty())
            return time_now + 10*1000*1000;
        else
        {
            cms_ns_if_print("cmip", 3, "DelaySimulator: Wakeup after %.3fms",
                            (queue.front().time_added + add_delay - time_now)/1000.0);
            return queue.front().time_added + add_delay;
        }
    }

    void process_events()
    {
        int64_t time_now = get_current_time_usec();
        send_some(time_now);
    }
    
    // Tohle se chova jako funkce `sendto`.
    //
    // Jenom pozor: Kdyz vrati EAGAIN, EWOULDBLOCK, tak je to jenom
    // simulovane. Tj. Pokud bychom pouzivali edge-triggered epoll,
    // a cekali, az budeme moci zapisovat, tak budeme cekat vecne.
    //
    ssize_t sendto(int sockfd, char const *data, size_t len, sockaddr_in dest)
    {
        cms_ns_if_print("cmip", 2, "sendto() Debug: Queuing (queue_size=%zu): %s",
                        queue.size(),
                        replace_nonprintable(WeakString(data, data + len)).c_str());
        int64_t time_now = get_current_time_usec();
        send_some(time_now);

        DelayedFrame f{
            sockfd,
            dest,
            std::string(data, data + len),
            time_now,
        };

        if (debug_probability > 0 && rand() % debug_probability == 0)
        {
            switch (rand() % 5)
            {
            case 0:
                cms_ns_if_print("cmip", 2, "sendto() Debug: Reordering datagram: %s", 
                                replace_nonprintable(WeakString(data, data + len)).c_str());
                if (queue.size() >= 2)
                    queue.insert(queue.end() - 1, f);
                return len;
            case 1:
                cms_ns_if_print("cmip", 2, "sendto() Debug: Returning EWOULDBLOCK: %s", 
                                replace_nonprintable(WeakString(data, data + len)).c_str());
                errno = EWOULDBLOCK;
                return -1;
            case 2:
                cms_ns_if_print("cmip", 2, "sendto() Debug: Returning EAGAIN: %s", 
                                replace_nonprintable(WeakString(data, data + len)).c_str());
                errno = EAGAIN;
                return -1;
            case 3:
                cms_ns_if_print("cmip", 2, "sendto() Debug: Duplicating datagram: %s", 
                                replace_nonprintable(WeakString(data, data + len)).c_str());
                queue.push_back(f);
                queue.push_back(f);
                return len;
            default:
                cms_ns_if_print("cmip", 2, "sendto() Debug: Discarding datagram: %s", 
                                replace_nonprintable(WeakString(data, data + len)).c_str());
                return len;
            }
        }
        else
        {
            queue.push_back(std::move(f));
            return len;
        }
    }

private:
    void send_some(int64_t time_now)
    {
        for (; !queue.empty(); queue.pop_front())
        {
            DelayedFrame &f = queue.front();
            int64_t send_time = f.time_added + add_delay;
            if (send_time < time_now)
            {
                ssize_t n = ::sendto(f.sockfd,
                                     f.message.c_str(),
                                     f.message.size(),
                                     0,
                                     (sockaddr const *)&f.dest,
                                     sizeof f.dest);
                if (n < 0)
                {
                    auto err = errno;
                    cms_ns_if_print("cmip", 1, "sendto() Debug: Failed (delayed %.3f msec): %s: %s",
                                    (time_now - f.time_added)/1000.0,
                                    replace_nonprintable(f.message).c_str(), strerror(err));
                    break;
                }
                assert(n == f.message.size());
                cms_ns_if_print("cmip", 2, "sendto() Debug: Send (delay %.1f ms, req_delay %.1f ms): %s",
                                (time_now - f.time_added)/1000.0,
                                add_delay/1000.0,
                                replace_nonprintable(f.message).c_str());
            }
            else
            {
                // cms_ns_if_print("cmip", 2, "sendto() Waiting: Delayed %.3f msec: %s",
                //                 (send_time - time_now)/1000.0,
                //                 f.message.c_str());
                break;
            }
        }
    }
};


#endif // CM_IP_DELAY_SIMULATOR_HH_
