#ifndef _ESP_ASYNC_WEB_SERVER_H_
#define _ESP_ASYNC_WEB_SERVER_H_
#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

/* Stand-in for ESPAsyncWebServer, enough of it to drive the console's handlers on the host.
   Unlike the IDF's C API, the server and the socket are objects the backend keeps in its
   own anonymous namespace - so a test cannot reach them by name. Everything observable is
   therefore recorded here, in one place both sides can see, the way the LittleFS stub keeps
   its filesystem. */
class AsyncWebServerRequest;
class AsyncWebSocket;
class AsyncWebSocketClient;

using ArRequestHandlerFunction = std::function<void(AsyncWebServerRequest*)>;
/* The library hands a POST body to this one in chunks, and only then calls the request
   handler - which is the order the update path depends on, so a stub that could not
   reproduce it would be testing something else. */
using ArBodyHandlerFunction = std::function<void(AsyncWebServerRequest*, uint8_t*, size_t, size_t, size_t)>;
using ArUploadHandlerFunction = std::function<void(AsyncWebServerRequest*, const std::string&, size_t, uint8_t*, size_t, bool)>;

enum AwsEventType { WS_EVT_CONNECT, WS_EVT_DISCONNECT, WS_EVT_PONG, WS_EVT_ERROR, WS_EVT_DATA };
enum AwsFrameOpcode { WS_CONTINUATION = 0, WS_TEXT = 1, WS_BINARY = 2 };
struct AwsFrameInfo { bool final; uint64_t index; uint64_t len; uint8_t opcode; };

using AwsEventHandler = std::function<void(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType,
                                           void*, uint8_t*, size_t)>;

enum { HTTP_GET = 1, HTTP_POST = 2 };

struct WebStubStateType {
    std::map<std::string, ArRequestHandlerFunction> Routes;
    std::map<std::string, ArBodyHandlerFunction> BodyRoutes;
    AwsEventHandler SocketHandler;
    /* What left as a text frame and what left as a binary one, kept apart because the
       console sends answers as the first and the panel as the second. */
    std::vector<std::string> Text;
    std::vector<std::string> Binary;
    /* Set by the test, read by the backend through count(): whether anybody is watching is
       the socket's answer on the target, so the test has to stand in for it. */
    size_t ClientCount{0u};
    bool Listening{false};
};

inline WebStubStateType& webStubState() { static WebStubStateType State; return State; }

class AsyncWebServerResponse {
  public:
    /* Kept, unlike the first version of this stub: the update route is the one place where
       the same shape of answer means two different things, and 200 against 400 is what says
       which. */
    int Code{0};
    std::string ContentType;
    std::string Body;
    std::map<std::string, std::string> Headers;

    virtual ~AsyncWebServerResponse() { }
    void addHeader(const char* Key, const char* Value) { Headers[Key] = Value; }
};

class AsyncResponseStream : public AsyncWebServerResponse {
  public:
    size_t write(uint8_t Character) { Body.push_back(static_cast<char>(Character)); return 1u; }
    size_t write(const uint8_t* Data, size_t Length) { Body.append(reinterpret_cast<const char*>(Data), Length); return Length; }
};

enum class AsyncAuthType { AUTH_NONE, AUTH_BASIC, AUTH_DIGEST };

class AsyncWebServerRequest {
  public:
    /* What the backend asks about a credential, and what it does when there is none. The
       comparison itself is the library's on the target, so the stub only has to say which
       answer a case wants and record that the 401 went out. */
    bool Authorised{true};
    bool AuthenticationRequested{false};

    bool authenticate(const char*, const char*) { return Authorised; }
    void requestAuthentication(AsyncAuthType, const char*) { AuthenticationRequested = true; }

    /* What the handler produced, for the test to look at. Owned here and freed with the
       request, which is what the server does on the target. */
    AsyncWebServerResponse* Response{nullptr};

    ~AsyncWebServerRequest() { delete Response; }

    AsyncWebServerResponse* beginResponse(int Code, const char* Type, const uint8_t* Content, size_t Length) {
        AsyncWebServerResponse* Made = new AsyncWebServerResponse();
        Made->Code = Code;
        Made->ContentType = Type;
        Made->Body.assign(reinterpret_cast<const char*>(Content), Length);
        return Made;
    }

    AsyncResponseStream* beginResponseStream(const char* Type) {
        AsyncResponseStream* Made = new AsyncResponseStream();
        Made->ContentType = Type;
        return Made;
    }

    void send(AsyncWebServerResponse* Sent) { delete Response; Response = Sent; }
};

class AsyncWebSocket {
  public:
    explicit AsyncWebSocket(const char*) { }

    void onEvent(AwsEventHandler Handler) { webStubState().SocketHandler = Handler; }
    void textAll(const char* Message, size_t Length) { webStubState().Text.emplace_back(Message, Length); }
    void binaryAll(const char* Message, size_t Length) { webStubState().Binary.emplace_back(Message, Length); }
    size_t count() const { return webStubState().ClientCount; }
};

class AsyncWebServer {
  public:
    explicit AsyncWebServer(uint16_t) { }

    void on(const char* Uri, int, ArRequestHandlerFunction Handler) { webStubState().Routes[Uri] = Handler; }
    void on(const char* Uri, int, ArRequestHandlerFunction Handler, ArUploadHandlerFunction,
            ArBodyHandlerFunction Body) {
        webStubState().Routes[Uri] = Handler;
        webStubState().BodyRoutes[Uri] = Body;
    }
    void addHandler(AsyncWebSocket*) { }
    void begin() { webStubState().Listening = true; }
};
#endif
