#include "HostCounter.h"

#include <algorithm> // for std::find

HostCounter::HostCounter()
{
}

bool HostCounter::isNewHost(std::string hostname)
{
<<<<<<< HEAD
    //return std::find(myHosts.begin(), myHosts.end(), hostname) == myHosts.end();
    return myHosts.find(hostname) == myHosts.end();
=======
    return std::find(myHosts.begin(), myHosts.end(), hostname) == myHosts.end();
>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a
}

void HostCounter::notifyHost(std::string hostname)
{
    // add the host in the list if not already in
    if(isNewHost(hostname))
    {
<<<<<<< HEAD
        // myHosts.push_back(hostname);
        myHosts.insert(hostname);
=======
        myHosts.push_back(hostname);
>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a
    }
}

int HostCounter::getNbOfHosts()
{
    return myHosts.size();
}
