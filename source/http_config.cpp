#include "http_config.h"

#define DEBUG_HTML

int TS_config::connect()
{
#ifdef DEBUG_HTML
	printf("Starting connection...\r\n");
#endif

	network = NetworkInterface::get_default_instance();

    int status_err = network->connect();

    if (status_err != 0) {
#ifdef DEBUG_HTML
        printf("Cannot connect to the network (error code %d)\r\n", status_err);
#endif
        return status_err;
    } else {
#ifdef DEBUG_HTML
        printf("Connect Ok: %s\r\n", network->get_ip_address());
#endif
    	return 0;
    }
}

int TS_config::parcer(json_char* json_res)
{
	json_value* value_res;
	char* name_value;

	value_res = json_parse(get_res->get_body_as_string().c_str(), get_res->get_body_length());

    if (value_res == NULL) {
            return 1;
    }

    int length = value_res->u.array.length;

    for (int i = 0; i<length; i++) {

    	name_value = value_res->u.object.values[i].name;
    	printf("Name: %s\r\n", name_value);

		switch (value_res->u.object.values[i].value->type) {
				case json_none:
						printf("none\r\n");
						break;
				case json_integer:
						printf("int: %10" PRId64 "\r\n", value_res->u.object.values[i].value->u.integer);
						break;
				case json_double:
						printf("double: %f\r\n", value_res->u.object.values[i].value->u.dbl);
						break;
				case json_string:
						printf("string: %s\r\n", value_res->u.object.values[i].value->u.string.ptr);
						break;
				case json_boolean:
						printf("bool: %d\r\n", value_res->u.object.values[i].value->u.boolean);
						break;
		}
    }



//    printf("length %d\r\n", length);

	return 0;
}


int TS_config::get_config()
{
	int id = 56789;
    char url_str[256];
    sprintf(url_str, "%s/%d", "http://192.168.1.9/get_config", id);

    get_req = new HttpRequest(network, HTTP_GET, url_str);

    get_res = get_req->send();

    if (!get_res) {
#ifdef DEBUG_HTML
        printf("HttpRequest failed (error code %d)\r\n", get_req->get_error());
#endif
        return 1;
    }
#ifdef DEBUG_HTML
    printf("\r\nBody (%d bytes):\r\n\r\n%s\r\n", get_res->get_body_length(), get_res->get_body_as_string().c_str());
#endif

    json_char* json_res;
    json_res = (json_char*)get_res->get_body();

    parcer(json_res);

	return 0;
}
