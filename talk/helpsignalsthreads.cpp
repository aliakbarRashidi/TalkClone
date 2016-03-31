#include "helpsignalsthreads.h"

#define SUCCESS 0
#define ERR_SOCKET 3

/*
 *=====================================
 * SIGNAL HANDLING
 *=====================================
 */

void int_signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM || signum == SIGHUP) {
        endOfLoop = true; //this is the main loop controller
                          //by turning this to true we have a succesfully
                          //shutdown
    }
}

void setSigMask(int sigAction)
{
    //If we block all signals the inherited thread will block them also
    sigset_t set;
    sigfillset(&set);

    int res = pthread_sigmask(sigAction, &set, nullptr);
    if ( res != 0 )
        throw std::system_error(errno, std::system_category(),
        "It wasn't possible to block signals receiving");
}

/*===============================================================
 * Threads's domain
 *===============================================================
 */
void requestCancellation(std::thread& oneThread)
{
    pthread_cancel(oneThread.native_handle());
    oneThread.join();
}


