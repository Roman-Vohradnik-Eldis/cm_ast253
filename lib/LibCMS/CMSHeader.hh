#ifndef _CMS_HEADER_HH
#define _CMS_HEADER_HH

#include "cms_header.h"

#include <algorithm>
#include <deque>
#include <exception>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#define CRoundBuffer CMSRoundBuffer
#define CRoundBlockBuffer CMSRoundBlockBuffer
#define CTCPClient CMSClient
#define CConfig2 CMSConfig
#define CIP CMSIP
#define CTCPServer CMSServer
#define CTCPServer2 CMSServer2
#define CMutex CMSMutex
#define CMutexRecursive CMSMutexRecursive
#define CCondition CMSCondition
#define CUDP CMSUDP

#define TRANSFER_MODE_ELDIS CMS_TCP_MODE_ELDIS
#define TRANSFER_MODE_RAW CMS_TCP_MODE_RAW
#define TRANSFER_MODE_TEXT CMS_TCP_MODE_TEXT

#define ROUND_BUFFER_READ_BLOCK CMS_ROUND_BUFFER_READ_BLOCK
#define ROUND_BUFFER_WRITE_GROW CMS_ROUND_BUFFER_WRITE_GROW

#endif /* _CMS_HEADER_HH */
