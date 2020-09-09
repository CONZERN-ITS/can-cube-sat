/*
 * strela-ms-rpi.c
 *
 *  Created on: Feb 21, 2020
 *      Author: developer
 */
#include "unistd.h"
#include "lis3mdl_STdC/driver/lis3mdl_reg.h"
#include "lsm6ds3_STdC/driver/lsm6ds3_reg.h"
#include "WMM2020_Linux/src/GeomagnetismHeader.h"

#define CONFIG_FILE_NAME "./WMM.COF"

typedef enum {
	ERROR_NO_ERROR = 0,
	ERROR_UNKNOWN_ID = -1,
	ERROR_COF_NOT_FOUND = -2,
	ERROR_NO_MEMORY = -3,
	ERROR_INCORRECT_VALUE = -4,

} error_t;

typedef struct {
    double latitude;
    double longitude;
    double altitude;
    int day;
    int month;
    int year;
} location_data_t;


int32_t lis3mdl_init (stmdev_ctx_t *lis3mdl_context){
	int32_t error = 0;
	uint8_t reset = 1;
	uint8_t id = 0;

	// Reset
    error = lis3mdl_reset_set(lis3mdl_context, PROPERTY_ENABLE);
	if (error) return error;
	usleep(50*1000);

	// Check id
	error = lis3mdl_device_id_get(lis3mdl_context, &id);
	if (error) return error;
	if (id != LIS3MDL_ID) return ERROR_UNKNOWN_ID;

	// Set configuration
	error = lis3mdl_block_data_update_set(lis3mdl_context, PROPERTY_ENABLE);
	if (error) return error;
	error = lis3mdl_data_rate_set(lis3mdl_context, LIS3MDL_UHP_20Hz);
	if (error) return error;
	error = lis3mdl_full_scale_set(lis3mdl_context, LIS3MDL_4_GAUSS);
	if (error) return error;
	error = lis3mdl_operating_mode_set(lis3mdl_context, LIS3MDL_CONTINUOUS_MODE);
	if (error) return error;
	error = lis3mdl_temperature_meas_set(lis3mdl_context, 1);
	return error;
}

int32_t lis3mdl_get_data_G (stmdev_ctx_t *lis3mdl_context, float* mag){
	uint8_t data_raw [6];
	uint8_t error;

	error = lis3mdl_magnetic_raw_get(lis3mdl_context, data_raw);
	if (error) return error;
	mag[0] = LIS3MDL_FROM_FS_4G_TO_G((int16_t)(((uint16_t)data_raw[1] << 8) | data_raw[0]));
	mag[1] = LIS3MDL_FROM_FS_4G_TO_G((int16_t)(((uint16_t)data_raw[3] << 8) | data_raw[2]));
	mag[2] = LIS3MDL_FROM_FS_4G_TO_G((int16_t)(((uint16_t)data_raw[5] << 8) | data_raw[4]));
	return 0;
}

int32_t lsm6ds3_init(stmdev_ctx_t *lsm6ds3_context){
	int32_t error = 0;
	uint8_t reset = 1;
	uint8_t id = 0;

	// Reset
	lsm6ds3_reg_t reg = {0};
	reg.ctrl3_c.sw_reset = 1;
	error = lsm6ds3_write_reg(lsm6ds3_context, LSM6DS3_CTRL3_C, &reg.byte, 1);
	if (error) return error;
	usleep(50*1000);

	// Check id
	error = lsm6ds3_device_id_get(lsm6ds3_context, &id);
	if (error) return error;
	if (id != LSM6DS3_ID) return ERROR_UNKNOWN_ID;

	// Set configuration
	error = lsm6ds3_block_data_update_set(lsm6ds3_context, PROPERTY_ENABLE);
	if (error) return error;
	error = lsm6ds3_xl_full_scale_set(lsm6ds3_context, LSM6DS3_4g);
	if (error) return error;
	error = lsm6ds3_gy_full_scale_set(lsm6ds3_context, LSM6DS3_1000dps);
	if (error) return error;
	error = lsm6ds3_xl_data_rate_set(lsm6ds3_context, LSM6DS3_XL_ODR_104Hz);
	if (error) return error;
	error = lsm6ds3_gy_data_rate_set(lsm6ds3_context, LSM6DS3_GY_ODR_104Hz);
	return error;
}


int32_t lsm6ds3_get_accel_data_mg(stmdev_ctx_t *lsm6ds3_context, float* accel)
{
	uint8_t data_raw [6];
	int32_t error;
	error = lsm6ds3_acceleration_raw_get(lsm6ds3_context, data_raw);
	if (error) return error;
	accel[0] = lsm6ds3_from_fs4g_to_mg((data_raw[1] << 8) | data_raw[0]);
	accel[1] = lsm6ds3_from_fs4g_to_mg((data_raw[3] << 8) | data_raw[2]);
	accel[2] = lsm6ds3_from_fs4g_to_mg((data_raw[5] << 8) | data_raw[4]);
	return 0;
}

int32_t date_to_year(MAGtype_Date *CalendarDate){
    int days = 0;
    int month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int total_days = 365;
    int i;
    if((CalendarDate->Month <= 0) || (CalendarDate->Month > 12) ||
       (CalendarDate->Day <= 0) || (CalendarDate->Day > month_days[CalendarDate->Month - 1]))
        return 1;

    if((CalendarDate->Year % 4 == 0 && CalendarDate->Year % 100 != 0) || CalendarDate->Year % 400 == 0){
    	month_days[1] += 1;
    	total_days += 1;
    }

    for(i = 0; i < CalendarDate->Month - 1; i++)
    	days += month_days[i];
    days += CalendarDate->Day - 1;
    CalendarDate->DecimalYear = CalendarDate->Year + (double)days / total_days;
    return 0;

}

int32_t find_true_north(location_data_t location, double * decl){
    MAGtype_MagneticModel * magnetic_models[1], *timed_magnetic_model;
    MAGtype_Ellipsoid ellip;
    MAGtype_CoordSpherical coord_spherical;
    MAGtype_CoordGeodetic coord_geodetic;
    MAGtype_Date user_date;
    MAGtype_GeoMagneticElements geo_magnetic_elements;
    int num_terms, n_max = 0;

    if(!MAG_robustReadMagModels(CONFIG_FILE_NAME, &magnetic_models, 1))
        return ERROR_COF_NOT_FOUND;
    if(n_max < magnetic_models[0]->nMax) n_max = magnetic_models[0]->nMax;
    num_terms = ((n_max + 1) * (n_max + 2) / 2);
    timed_magnetic_model = MAG_AllocateModelMemory(num_terms); // For storing the time modified WMM Model parameters
    if(magnetic_models[0] == NULL || timed_magnetic_model == NULL)
        return ERROR_NO_MEMORY;

    // Sets WGS-84 parameters
    ellip.a = 6378.137;
    ellip.b = 6356.7523142;
    ellip.fla = 1 / 298.257223563;
    ellip.eps = sqrt(1 - (ellip.b * ellip.b) / (ellip.a * ellip.a));
    ellip.epssq = (ellip.eps * ellip.eps);
    ellip.re = 6371.2;

    // Input
    coord_geodetic.phi = location.latitude; // North latitude positive, For example:30 for 30 degrees north
    coord_geodetic.lambda = location.longitude; // East longitude positive, West negative.  For example:-100.5 for 100.5 degrees west
    coord_geodetic.HeightAboveEllipsoid = location.altitude; // Height above WGS-84 ellipsoid
    user_date.Month = location.month;
    user_date.Day = location.day;
    user_date.Year = location.year;
    if(date_to_year(&user_date)) return ERROR_INCORRECT_VALUE;

    MAG_GeodeticToSpherical(ellip, coord_geodetic, &coord_spherical); // Convert from geodetic to Spherical Equations
    MAG_TimelyModifyMagneticModel(user_date, magnetic_models[0], timed_magnetic_model); // Time adjust the coefficients
    MAG_Geomag(ellip, coord_spherical, coord_geodetic, timed_magnetic_model, &geo_magnetic_elements); // Computes the geoMagnetic field elements and their time change
    MAG_CalculateGridVariation(coord_geodetic, &geo_magnetic_elements);

    *decl = geo_magnetic_elements.Decl; // Angle between the magnetic field vector and true north, positive east

    MAG_FreeMagneticModelMemory(timed_magnetic_model);
    MAG_FreeMagneticModelMemory(magnetic_models[0]);

    return ERROR_NO_ERROR;
}



