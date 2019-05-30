#include "NetworkInterface.h"
#include "http_request.h"
#include "json.h"
#include "map"
#include "nvstore.h"

typedef struct {
	map <string,uint16_t>  nv_key;
	map <string,uint32_t> date;
} TS_config_map_TypeDef;

namespace mbed {

class TS_config {

private:
	NetworkInterface* network;
	HttpRequest* get_req;
	HttpResponse* get_res;
//	TS_config_map_TypeDef conf_map;
	int parcer(json_char* json_res);
	NVStore &nvstore = NVStore::get_instance();
	int max_keys_correct();

public:
	TS_config_map_TypeDef conf_map;
	int sn;
	char* server_url;
	int init();
	int reset_nvstore();
	int connect();
	int get_conf();
	int set_key(char* alias, uint16_t key);
	uint32_t get_value(char* alias);
	int save_conf_to_flesh();
	int read_conf_from_flesh();
};

}
