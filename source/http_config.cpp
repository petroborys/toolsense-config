#include "http_config.h"

#define DEBUG_HTML

int TS_config::connect()
{
#ifdef DEBUG_HTML
	printf("Starting connection...\r\n");
#endif

	network = NetworkInterface::get_default_instance();

    int rc = network->connect();

    if (rc != 0) {
        printf("Cannot connect to the network (error code %d)\r\n", rc);
        return rc;
    } else {
#ifdef DEBUG_HTML
        printf("Connect Ok: %s\r\n", network->get_ip_address());
#endif
    	return 0;
    }
}

int TS_config::disconnect()
{
	int rc = network->disconnect();

	if (rc != 0) {
		printf("Cannot disconnect (error code %d)\r\n", rc);
		return rc;
	}

#ifdef DEBUG_HTML
		printf("Disconnect Ok\r\n");
#endif

	return 0;
}

int TS_config::parcer(json_char* json_res)
{
	json_value* value_res;
	char* name_value;

	uint32_t value = 0;

	value_res = json_parse(get_res->get_body_as_string().c_str(), get_res->get_body_length());

    if (value_res == NULL) {
            return 1;
    }

    int length = value_res->u.array.length;

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
						printf("Parse error: incorrect type\r\n");
						return 1;
		}

		conf_map.date[name_value] = value;
    }

	return 0;
}


int TS_config::get_conf()
{
    char url_str[256];
    sprintf(url_str, "%s/%s/%d", server_url, "get_config", sn);

    get_req = new HttpRequest(network, HTTP_GET, url_str);

    get_res = get_req->send();

    if (!get_res) {
        printf("HttpRequest failed (error code %d)\r\n", get_req->get_error());
        return 1;
    }
#ifdef DEBUG_HTML
    printf("\r\nBody (%d bytes):\r\n\r\n%s\r\n", get_res->get_body_length(), get_res->get_body_as_string().c_str());
#endif

    if (parcer((json_char*)get_res->get_body()) != 0) return 1;

    if (save_conf_to_flesh() != 0) return 1;

	return 0;
}

int TS_config::set_key(char* alias, uint16_t key)
{
	if (key == 0) {
		printf("Incorrect key (func set_key): %s - %d\r\n", alias, key);
		return -1;
	}

	conf_map.nv_key[alias] = key;
#ifdef DEBUG_HTML
	printf("set_key: %s - %d\r\n", alias, key);
#endif
	return 0;
}


uint32_t TS_config::get_value(char* alias)
{
	uint32_t res = 0;
	res = conf_map.date[alias];
#ifdef DEBUG_HTML
	printf("get_value: %s - %d\r\n", alias, res);
#endif
	return res;
}

int TS_config::max_keys_correct() {
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
			printf("NVStore incorrect max number of keys is %d\r\n", map_size);
			return 1;
		}
	}

	return 0;
}


int TS_config::init() {

	int rc;
	rc = nvstore.init();

	if (rc != NVSTORE_SUCCESS) {
		printf("NVStore return code is %d \r\n", rc);
		return rc;
	}

	if (max_keys_correct() !=0) {
		return 1;
	}

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

    if (read_conf_from_flesh() == 0) return 0;
    else return 1;
}

int TS_config::reset_nvstore() {

	int rc;
	rc = nvstore.reset();
	return rc;
}

int TS_config::save_conf_to_flesh() {
	uint16_t key;
	uint32_t data;
	int rc;
	if (max_keys_correct() !=0) {
		return 1;
	}
#ifdef DEBUG_HTML
	printf("\r\nIterator map:\r\n");
#endif
	for (map <string,uint32_t> ::iterator it=conf_map.date.begin(); it!=conf_map.date.end(); ++it) {
		key = conf_map.nv_key[it->first];
		data = it->second;

		if (key != 0) {
			rc = nvstore.set(key, sizeof(data), &data);
			if (rc != NVSTORE_SUCCESS) {
				printf("NVStore return code is %d \r\n", rc);
				return rc;
			}
#ifdef DEBUG_HTML
			printf("Set key %d to value %ld. \r\n", key, data);
#endif
		}
	}

	return 0;
}

int TS_config::read_conf_from_flesh() {
	uint16_t key;
	uint32_t data;
	uint16_t actual_len_bytes = 0;
	int rc;
#ifdef DEBUG_HTML
	printf("\r\nRead map:\r\n");
#endif
	for (map <string,uint16_t> ::iterator it=conf_map.nv_key.begin(); it!=conf_map.nv_key.end(); ++it) {
		key = it->second;

		if (key != 0) {
		    rc = nvstore.get(key, sizeof(data), &data, actual_len_bytes);
			if (rc != NVSTORE_SUCCESS) {
				printf("NVStore return code is %d \r\n", rc);
				return rc;
			}

		    conf_map.date[it->first] = data;
#ifdef DEBUG_HTML
		    printf("Get key %d. Value is %ld \r\n", key, data);
#endif
		}
	}

	return 0;
}
