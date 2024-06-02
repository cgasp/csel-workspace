#include <string>

<<<<<<< HEAD
//#include <vector>
#include <set>
=======
#include <vector>
>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a

class HostCounter
{
    public:
        HostCounter();

        // Announce a host to the HostCounter.
        // if the host is new, it will be added to the list, otherwise we ignore it.
        void notifyHost(std::string hostname);

        // return the number of unique hosts found so far
        int getNbOfHosts();

    private:
        // check if host is already in the list
        bool isNewHost(std::string hostname);

<<<<<<< HEAD
        //std::vector< std::string > myHosts;
        std::set< std::string > myHosts;
=======
        std::vector< std::string > myHosts;
>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a
};
