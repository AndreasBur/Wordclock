/******************************************************************************************************************************************************
 *  FILE DESCRIPTION
 *  -------------------------------------------------------------------------------------------------------------------------------------------------*/
/**     \file       webhost_test.cpp
 *      \brief      The simulator's web host, over a real socket on loopback
******************************************************************************************************************************************************/


#include "check.h"
#include "cases.h"

#include "sim/WebHost.h"

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#ifndef _WIN32
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
#endif

/* A connection that is opened and says nothing - what a browser does when it opens a socket
   ahead of a request - must cost the tick a moment and not stop it. It used to stop it until
   the browser closed the socket, and the window and every other client with it.

   Closed from a second thread after a few seconds, so that a host which does wait for it
   ends this case late and failed rather than never. Needs the port, so a simulator left
   open next to the tests skips the case rather than failing it. */
void testWebHostOutlastsAnIdleConnection()
{
#ifndef _WIN32
    WebHost& Host = WebHost::getInstance();

    if(Host.begin() != E_OK) {
        std::cerr << "skipped: port " << WEB_HOST_PORT << " is taken, the web host case needs it\n";
        return;
    }

    const int Idle = socket(AF_INET, SOCK_STREAM, 0);
    if(Idle < 0) {
        expect(false, "the case must get a socket of its own");
        Host.end();
        return;
    }

    sockaddr_in Address{};
    Address.sin_family = AF_INET;
    Address.sin_port = htons(WEB_HOST_PORT);
    Address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    expect(connect(Idle, reinterpret_cast<sockaddr*>(&Address), sizeof(Address)) == 0,
           "the idle connection must reach the host");

    std::promise<void> TaskDone;
    std::thread Closer([Idle, Done = TaskDone.get_future()]() {
        if(Done.wait_for(std::chrono::seconds(3)) == std::future_status::timeout) { shutdown(Idle, SHUT_RDWR); }
    });

    const auto Start = std::chrono::steady_clock::now();
    Host.task();
    const auto Elapsed = std::chrono::steady_clock::now() - Start;

    TaskDone.set_value();
    Closer.join();
    close(Idle);
    Host.end();

    expect(Elapsed < std::chrono::seconds(1), "an idle connection must not hold the tick until it closes");
#endif
}
