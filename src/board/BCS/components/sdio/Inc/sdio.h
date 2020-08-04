/*
 * sdio.h
 *
 *  Created on: Jul 28, 2020
 *      Author: sereshotes
 */

#ifndef COMPONENTS_SDIO_INC_SDIO_H_
#define COMPONENTS_SDIO_INC_SDIO_H_

#define SD_BAN(F) \
	F(MAVLINK_MSG_ID_TIMESTAMP)


int sd_init(void);



#endif /* COMPONENTS_SDIO_INC_SDIO_H_ */
