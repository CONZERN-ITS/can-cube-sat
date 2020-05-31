#ifndef INC_MICS6814_H_
#define INC_MICS6814_H_

#include <mavlink/its/mavlink.h>


//! Концентрация NH3 из сопротивления NH3 сенсора MICS-6814
float mics6814_rescale_nh3(float nh3_r);

//! Концентрация NO2 из сопротивления OX сенсора MICS-6814
float mics6814_rescale_no2(float ox_r);

//! Конценрация CO из сопротивления RED сенсора MICS-6814
float mics6814_rescale_co(float red_r);


//! Собираем все данные с mics6814
int mics6814_read(mavlink_pld_mics_6814_data_t * data);


#endif /* INC_MICS6814_H_ */
