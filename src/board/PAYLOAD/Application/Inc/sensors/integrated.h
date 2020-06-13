#ifndef SRC_INTEGERATED_TEMPERATURE_H_
#define SRC_INTEGERATED_TEMPERATURE_H_


#include <mavlink_main.h>


int its_pld_inttemp_init(void);

int its_pld_inttemp_read(mavlink_own_temp_t * msg);


#endif /* SRC_INTEGERATED_TEMPERATURE_H_ */
