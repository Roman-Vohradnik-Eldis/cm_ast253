#ifndef _CMS_HEADER_H
#define _CMS_HEADER_H

#ifdef __i386__
#define _FILE_OFFSET_BITS 64
#endif

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>

#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef G_OS_UNIX
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sched.h>
#include <signal.h>
#include <syslog.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/vfs.h>
#include <sys/wait.h>
#include <termios.h>
#endif

#ifdef G_OS_WIN32
#define WINVER 0x0501
#include <windows.h>
#include <ws2tcpip.h>
#define SHUT_RDWR SD_BOTH
#define WEXITSTATUS(x) (x)
#define sleep(m) g_usleep ((m) * 1000000)
#define fsync(x)
#endif


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MSGMAX 0xffff
#define BUFMAX 0xff00

#define CMS_CLIENT_MAX_BUFFER_SIZE 0x1000000
#define CMS_IP_MAX_BUFFER_SIZE 0x1000000
#define CMS_UDP_MAX_BUFFER_SIZE 0x1000000

#define CMS_FLAG_SEND 0x01
#define CMS_FLAG_RECV 0x02
#define CMS_FLAG_BOTH 0x03

enum CMS_CLIENT_MODE
{
	CMS_CLIENT_MODE_NONE,
	CMS_CLIENT_MODE_SERVER,
	CMS_CLIENT_MODE_SERVER2,
	CMS_CLIENT_MODE_IP,
};

enum CMS_TCP_MODE
{
	CMS_TCP_MODE_ELDIS,
	CMS_TCP_MODE_RAW,
	CMS_TCP_MODE_TEXT
};

enum CMS_PING_TYPE
{
	CMS_PING_TYPE_ICMP,
	CMS_PING_TYPE_DATA,
	CMS_PING_TYPE_OLD,
};

#define ELDIS_CMS_VERSION "3.6.23"

#endif /* _CMS_HEADER_H */
