

#define INA219_COUNT BATTERY_PACK_COUNT * 2


extern float ina219_voltage[INA219_COUNT];
extern float ina219_current[INA219_COUNT];


void data_ina219_init();

void data_ina219_read();

void data_ina219_send();

