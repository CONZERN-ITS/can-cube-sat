/*
 * me2o2.h
 *
 *  Created on: 31 мая 2020 г.
 *      Author: snork
 */

#ifndef INC_ME2O2_H_
#define INC_ME2O2_H_

#include <mavlink/its/mavlink.h>


int me2o2_init(void);

int me2o2_read(mavlink_pld_me2o2_data_t * msg);


#endif /* INC_ME2O2_H_ */
