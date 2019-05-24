#include "NetworkInterface.h"
#include "http_request.h"
#include "json.h"

namespace mbed {

class TS_config {

private:
	NetworkInterface* network;
	HttpRequest* get_req;
	HttpResponse* get_res;

public:
	int connect();
	int parcer(json_char* json_res);
	int get_config();
};

}
