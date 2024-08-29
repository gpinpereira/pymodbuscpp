/**
 * @file net_.h
 */

#ifndef _NET_ //###############################################################
#define _NET_

#include "exception_.h"
#include "string_.h"

#include <string>

namespace CUTIL {

cStringList cGetInterfaces();
bool cIsInterfaceUpAndRunning(std::string interface, bool loopback_dev=false);
bool cIsInterfaceWireless(std::string interface, std::string *protocol=nullptr);
std::string cGetIPAddress(std::string interface);
std::string cGetHostIpAddress(std::string domainName);

}

#endif // _NET_ ###############################################################
