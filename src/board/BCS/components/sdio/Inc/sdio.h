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


typedef enum {
	SD_STATE_UNMOUNTED,
	SD_STATE_MOUNTED_UNOPEN,
	SD_STATE_OPEN_WRITING,
} sd_state_t;

typedef enum {
	SD_OK = 0,
	SD_ERROR_MOUNT,
	SD_ERROR_DIR,
	SD_ERROR_OPEN,
	SD_ERROR_WRITE,
	SD_ERR0R_LOW_MEMORY,
	SD_ERROR_SAVE,
	SD_UNKNOWN
} sd_error_t;

sd_error_t sd_init(void);



#endif /* COMPONENTS_SDIO_INC_SDIO_H_ */
