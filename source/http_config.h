#include "NetworkInterface.h"
#include "http_request.h"
#include "json.h"
#include "map"
#include "nvstore.h"

typedef struct {
	map <string,uint16_t>  nv_key;
	map <string,uint32_t> data;
} TS_config_map_TypeDef;

namespace mbed {

class TS_config {

private:
	NetworkInterface* network;
//	TS_config_map_TypeDef conf_map;
	int parcer(HttpResponse* get_res);
	NVStore &nvstore = NVStore::get_instance();
	int max_keys_correct();
	int validation_config_data(json_value* value_res);

public:
	TS_config_map_TypeDef conf_map; //Public for dubug only
	int sn;
	char* server_url;
	int init();
	int reset_nvstore();
	int connect();
	int disconnect();
	int get_conf();
	int set_key(char* alias, uint16_t key);
	uint32_t get_value(char* alias);
	int save_conf_to_flesh();
	int read_conf_from_flesh();
};

}
