// The simulator's web host, over a real socket on loopback.

#include "check.h"
#include "cases.h"

#include "sim/WebHost.h"

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

/*
 * A connection that is opened and says nothing - what a browser does when it opens a socket
 * ahead of a request - must cost the tick a moment and not stop it. It used to stop it until
 * the browser closed the socket, and the window and every other client with it.
 *
 * Closed from a second thread after a few seconds, so that a host which does wait for it
 * ends this case late and failed rather than never. Needs the port, so a simulator left
 * open next to the tests skips the case rather than failing it.
 */
void TestWebHostOutlastsAnIdleConnection() {
#ifndef _WIN32
  WebHost& host = WebHost::GetInstance();

  if (host.Begin() != kEOk) {
    std::cerr << "skipped: port " << WEB_HOST_PORT << " is taken, the web host case needs it\n";
    return;
  }

  int const idle = socket(AF_INET, SOCK_STREAM, 0);
  if (idle < 0) {
    Expect(false, "the case must get a socket of its own");
    host.End();
    return;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(WEB_HOST_PORT);
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  Expect(connect(idle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0,
         "the idle connection must reach the host");

  std::promise<void> task_done;
  std::thread closer([idle, done = task_done.get_future()]() {
    if (done.wait_for(std::chrono::seconds(3)) == std::future_status::timeout) {
      shutdown(idle, SHUT_RDWR);
    }
  });

  auto const start = std::chrono::steady_clock::now();
  host.Task();
  auto const elapsed = std::chrono::steady_clock::now() - start;

  task_done.set_value();
  closer.join();
  close(idle);
  host.End();

  Expect(elapsed < std::chrono::seconds(1),
         "an idle connection must not hold the tick until it closes");
#endif
}
