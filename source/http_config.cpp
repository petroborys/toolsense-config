#include "http_config.h"

#define DEBUG_HTML

nsapi_error_t TS_config::connect()
{

	network = NetworkInterface::get_default_instance();
	nsapi_error_t rc = network->connect();

#ifdef DEBUG_HTML
    if (rc != NSAPI_ERROR_OK) printf("Cannot connect to the network (error code %d)\r\n", rc);
    else printf("Connect Ok: %s\r\n", network->get_ip_address());
#endif

    return rc;
}

nsapi_error_t TS_config::disconnect()
{
	nsapi_error_t rc = network->disconnect();

#ifdef DEBUG_HTML
	if (rc != NSAPI_ERROR_OK)printf("Cannot disconnect (error code %d)\r\n", rc);
	else printf("Disconnect Ok\r\n");
#endif

	return rc;
}

int TS_config::validation_config_data(json_value* value_res)
{
	int length = value_res->u.array.length;
	char* name_value;

    for (int i = 0; i<length; i++) {
    	name_value = value_res->u.object.values[i].name;
    	if (conf_map.data.count(name_value) == 0) return TS_HTTP_ERROR_UNSET_DATA_NAME;

    	if ((value_res->u.object.values[i].value->type != json_integer) && (value_res->u.object.values[i].value->type != json_boolean))
    		return TS_HTTP_ERROR_WRONG_DATA_TYPE;
    }

	return TS_HTTP_ERROR_OK;
}


int TS_config::parcer(HttpResponse* get_res)
{
	json_value* value_res;
	char* name_value;
	uint32_t value = 0;
	int rc;

	value_res = json_parse(get_res->get_body_as_string().c_str(), get_res->get_body_length());

    if (value_res == NULL) {
            return TS_HTTP_ERROR_NO_DATA;
    }

    int length = value_res->u.array.length;

	if (conf_map.nv_key.size() != length) {
#ifdef DEBUG_HTML
		printf("Wrong number of parameters\r\n");
#endif
		return TS_HTTP_ERROR_WRONG_LENGHT_DATA;
	}

	rc = validation_config_data(value_res);

	if (rc != TS_HTTP_ERROR_OK) {
#ifdef DEBUG_HTML
		printf("Wrong name or type of parameter\r\n");
#endif
		return rc;
	}

    for (int i = 0; i<length; i++) {

    	name_value = value_res->u.object.values[i].name;
#ifdef DEBUG_HTML
    	printf("Name: %s\r\n", name_value);
#endif

		switch (value_res->u.object.values[i].value->type) {
				case json_integer:
						value = (uint32_t)value_res->u.object.values[i].value->u.integer;
#ifdef DEBUG_HTML
						printf("int: %d\r\n", value);
#endif
						break;
				case json_boolean:
						value = (uint32_t)value_res->u.object.values[i].value->u.boolean;
#ifdef DEBUG_HTML
						printf("bool: %d\r\n", value);
#endif
						break;
				default:
#ifdef DEBUG_HTML
						printf("Parse error: wrong type\r\n");
#endif
						return TS_HTTP_ERROR_WRONG_DATA_TYPE;
		}

		conf_map.data[name_value] = value;
    }

	return TS_HTTP_ERROR_OK;
}


int TS_config::get_conf()
{
	HttpRequest* get_req;
	HttpResponse* get_res;

	int rc;

    char url_str[256];
    sprintf(url_str, "%s/%s/%d", server_url, "get_config", sn);

    get_req = new HttpRequest(network, HTTP_GET, url_str);

    get_res = get_req->send();

    if (!get_res) {
#ifdef DEBUG_HTML
        printf("HttpRequest failed (error code %d)\r\n", get_req->get_error());
#endif
        delete get_req;
        return TS_HTTP_ERROR_REQUEST_FAIL;
    }

#ifdef DEBUG_HTML
    printf("\r\nBody (%d bytes):\r\n\r\n%s\r\n", get_res->get_body_length(), get_res->get_body_as_string().c_str());
#endif

    rc = parcer(get_res);
//    delete get_req;
    if (rc != TS_HTTP_ERROR_OK) return rc;

    rc = save_conf_to_flesh();
    return rc;
}

int TS_config::send_verification(int code)
{
	HttpRequest* get_req;
	HttpResponse* get_res;

    char url_str[256];
    sprintf(url_str, "%s/%s/%d", server_url, "config_verification", code);

    get_req = new HttpRequest(network, HTTP_GET, url_str);

    get_res = get_req->send();

    if (!get_res) {
#ifdef DEBUG_HTML
        printf("HttpRequest failed (error code %d)\r\n", get_req->get_error());
#endif
        delete get_req;
        return TS_HTTP_ERROR_REQUEST_FAIL;
    }

    delete get_req;
	return TS_HTTP_ERROR_OK;
}

int TS_config::init_conf_pair(char* alias, uint16_t key)
{
	if (key == 0) {
#ifdef DEBUG_HTML
		printf("Wrong key (func set_key): %s - %d\r\n", alias, key);
#endif
		return TS_HTTP_ERROR_WRONG_KEY;
	}

	conf_map.nv_key[alias] = key;
	conf_map.data[alias] = 0;
#ifdef DEBUG_HTML
	printf("set_key: %s - %d\r\n", alias, key);
#endif

	return TS_HTTP_ERROR_OK;
}


uint32_t TS_config::get_value(char* alias)
{
	uint32_t res = 0;
	res = conf_map.data[alias];
#ifdef DEBUG_HTML
	printf("get_value: %s - %d\r\n", alias, res);
#endif
	return res;
}

int TS_config::max_keys_correct()
{
	size_t map_size = conf_map.nv_key.size();
	size_t max_keys = nvstore.get_max_keys();
	size_t max_possible_keys = nvstore.get_max_possible_keys();

	if (map_size > max_keys) {
		if (map_size < max_possible_keys) {
			nvstore.set_max_keys(map_size);
#ifdef DEBUG_HTML
			printf("NVStore new max number of keys is %d\r\n", nvstore.get_max_keys());
#endif
		}
		else {
#ifdef DEBUG_HTML
			printf("NVStore wrong max number of keys is %d\r\n", map_size);
#endif
			return TS_HTTP_ERROR_TOO_MUCH_KEYS;
		}
	}

	return TS_HTTP_ERROR_OK;
}


int TS_config::init()
{
	int rc;
	rc = nvstore.init();

	if (rc != NVSTORE_SUCCESS) {
#ifdef DEBUG_HTML
		printf("NVStore return code is %d \r\n", rc);
#endif
		return rc;
	}

	rc = max_keys_correct();
	if (rc !=TS_HTTP_ERROR_OK) return rc;

#ifdef DEBUG_HTML
	printf("Init NVStore.\r\n");
    // Show NVStore size, maximum number of keys and area addresses and sizes
    printf("NVStore size is %d.\r\n", nvstore.size());
    printf("NVStore max number of keys is %d (out of %d possible ones in this flash configuration).\r\n",
            nvstore.get_max_keys(), nvstore.get_max_possible_keys());
    printf("NVStore areas:\r\n");
    for (uint8_t area = 0; area < NVSTORE_NUM_AREAS; area++) {
        uint32_t area_address;
        size_t area_size;
        nvstore.get_area_params(area, area_address, area_size);
        printf("Area %d: address 0x%08lx, size %d (0x%x).\r\n", area, area_address, area_size, area_size);
    }
#endif

    rc = read_conf_from_flesh();
    return rc;
}


int TS_config::reset_nvstore()
{
	int rc = nvstore.reset();
	return rc;
}


int TS_config::save_conf_to_flesh()
{
	uint16_t key;
	uint32_t data;

	int rc = max_keys_correct();
	if (rc !=0) return rc;

#ifdef DEBUG_HTML
	printf("\r\nIterator map:\r\n");
#endif
	for (map <string,uint32_t> ::iterator it=conf_map.data.begin(); it!=conf_map.data.end(); ++it) {
		key = conf_map.nv_key[it->first];
		data = it->second;


		rc = nvstore.set(key, sizeof(data), &data);
		if (rc != NVSTORE_SUCCESS) {
#ifdef DEBUG_HTML
			printf("NVStore return code is %d \r\n", rc);
#endif
			return rc;
		}
#ifdef DEBUG_HTML
		printf("Set key %d to value %ld. \r\n", key, data);
#endif
	}

	return TS_HTTP_ERROR_OK;
}


int TS_config::read_conf_from_flesh()
{
	uint16_t key;
	uint32_t data;
	uint16_t actual_len_bytes = 0;
	int rc;
#ifdef DEBUG_HTML
	printf("Read map:\r\n");
#endif
	if (conf_map.nv_key.size() == 0) {
#ifdef DEBUG_HTML
		printf("The array of keys is empty\r\n");
#endif
		return TS_HTTP_ERROR_NO_SET_DATA;
	}
	for (map <string,uint16_t> ::iterator it=conf_map.nv_key.begin(); it!=conf_map.nv_key.end(); ++it) {
		key = it->second;

		rc = nvstore.get(key, sizeof(data), &data, actual_len_bytes);
		if (rc != NVSTORE_SUCCESS) {
#ifdef DEBUG_HTML
			printf("NVStore return code is %d \r\n", rc);
#endif
			return rc;
		}

		conf_map.data[it->first] = data;
#ifdef DEBUG_HTML
		printf("Get key %d. Value is %ld \r\n", key, data);
#endif
	}

	return TS_HTTP_ERROR_OK;
}
