#ifndef _ESP_HTTP_SERVER_H_
#define _ESP_HTTP_SERVER_H_
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include <sys/types.h>
typedef struct httpd_t* httpd_handle_t;
typedef enum { HTTP_GET = 1, HTTP_POST = 2 } httpd_method_t;
typedef struct httpd_req { int method; void* user_ctx; size_t content_len; } httpd_req_t;
typedef esp_err_t (*httpd_handler_t)(httpd_req_t*);
typedef void (*httpd_close_func_t)(httpd_handle_t, int);
typedef struct { const char* uri; httpd_method_t method; httpd_handler_t handler; void* user_ctx;
                 bool is_websocket; bool handle_ws_control_frames; const char* supported_subprotocol; } httpd_uri_t;
typedef struct { uint16_t server_port; uint16_t max_open_sockets; uint16_t max_uri_handlers;
                 httpd_close_func_t close_fn; int core_id; } httpd_config_t;
/* The 8 is the framework's own default for max_uri_handlers, kept here because it is the
   number the backend raises the count against. */
#define HTTPD_DEFAULT_CONFIG() httpd_config_t{80, 7, 8, nullptr, 0}
typedef enum { HTTPD_WS_TYPE_CONTINUE=0, HTTPD_WS_TYPE_TEXT=1, HTTPD_WS_TYPE_BINARY=2, HTTPD_WS_TYPE_CLOSE=8 } httpd_ws_type_t;
typedef struct { bool final; bool fragmented; httpd_ws_type_t type; uint8_t* payload; size_t len; } httpd_ws_frame_t;
esp_err_t httpd_start(httpd_handle_t*, const httpd_config_t*);
esp_err_t httpd_register_uri_handler(httpd_handle_t, const httpd_uri_t*);
esp_err_t httpd_resp_set_type(httpd_req_t*, const char*);
esp_err_t httpd_resp_set_hdr(httpd_req_t*, const char*, const char*);
esp_err_t httpd_resp_set_status(httpd_req_t*, const char*);
esp_err_t httpd_resp_send(httpd_req_t*, const char*, ssize_t);
/* The framework's own sentinel for "send until the terminator", which the update route
   answers with rather than counting its own JSON. */
#define HTTPD_RESP_USE_STRLEN (-1)
/* Answers a piece of the request body, or 0 for a connection that went away and a negative
   value for an error - which is the distinction the update route acts on. */
int httpd_req_recv(httpd_req_t*, char*, size_t);
/* One request header by name, ESP_OK when it was there. What the authorisation gate reads,
   and the only header this backend asks for. */
esp_err_t httpd_req_get_hdr_value_str(httpd_req_t*, const char*, char*, size_t);
esp_err_t httpd_resp_send_chunk(httpd_req_t*, const char*, ssize_t);
int httpd_req_to_sockfd(httpd_req_t*);
esp_err_t httpd_ws_recv_frame(httpd_req_t*, httpd_ws_frame_t*, size_t);
esp_err_t httpd_ws_send_frame_async(httpd_handle_t, int, httpd_ws_frame_t*);
#endif
