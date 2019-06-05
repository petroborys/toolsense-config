#include "NetworkInterface.h"
#include "http_request.h"
#include "json.h"
#include "map"
#include "nvstore.h"

/**
    Example:

#include "mbed.h"
#include "http_config.h"

#define SN "sn"
#define ENABLE_TEMP_SENSOR "bl"
#define MEASUREMENTS_FREQUENCY "frequency"

TS_config conf_ts;

int main()
{
	conf_ts.sn = 554466;
	conf_ts.server_url = "http://192.168.1.9";

	conf_ts.init_conf_pair(SN, 8);
	conf_ts.init_conf_pair("bl", 9);
	conf_ts.init_conf_pair("frequency", 10);

	conf_ts.init();

	if (conf_ts.connect() == 0)
	{
		int rc = conf_ts.get_conf();
		conf_ts.send_verification(rc);
		conf_ts.disconnect();
	}

	int sn = conf_ts.get_value(SN);
	bool enable_t = conf_ts.get_value(ENABLE_TEMP_SENSOR);
	int frequency_t = conf_ts.get_value(MEASUREMENTS_FREQUENCY);

	printf("Value %s: %d\r\n", SN, sn);
	printf("Value %s: %d\r\n", ENABLE_TEMP_SENSOR, enable_t);
	printf("Value %s: %d\r\n", MEASUREMENTS_FREQUENCY, frequency_t);
}

*/

#define     TS_HTTP_ERROR_OK                   0        /* no error */
#define 	TS_HTTP_ERROR_REQUEST_FAIL		   4001     /* http request failed */
#define 	TS_HTTP_ERROR_UNSET_DATA_NAME      4002     /*  */
#define 	TS_HTTP_ERROR_WRONG_DATA_TYPE      4003     /*  */
#define 	TS_HTTP_ERROR_WRONG_LENGHT_DATA    4004     /*  */
#define 	TS_HTTP_ERROR_NO_DATA      		   4005     /*  */
#define 	TS_HTTP_ERROR_WRONG_KEY			   4006     /*  */
#define 	TS_HTTP_ERROR_TOO_MUCH_KEYS        4007     /*  */
#define 	TS_HTTP_ERROR_NO_SET_DATA          4008     /*  */



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
	int save_conf_to_flesh();
	int read_conf_from_flesh();
public:
	TS_config_map_TypeDef conf_map; //Public for dubug only
	int sn;
	char* server_url;
	int init();
	int reset_nvstore();
	nsapi_error_t connect();
	nsapi_error_t disconnect();
	int get_conf();
	int init_conf_pair(char* alias, uint16_t key);
	uint32_t get_value(char* alias);
	int send_verification(int code);
};

}
