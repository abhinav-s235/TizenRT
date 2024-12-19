
#ifndef __INCLUDE_TINYARA_TEMPERATURE_SENSOR_H
#define __INCLUDE_TINYARA_TEMPERATURE_SENSOR_H



#if defined(CONFIG_TEMPERATURE_SENSOR)
#include<semaphore.h>




#define TEMPDEV_GETSENSORDATA 		_SNIOC(0)
#define TEMPERATURE_SENSOR_DEV_PATH "/dev/temperature0"






struct temperature_ops_s{
	int (*get_sensor_data)(struct temperature_dev_s *dev, FAR uint16_t *data);
};


struct temperature_dev_s{
	sem_t sem;
	uint8_t crefs;
	const struct temperature_ops_s *ops;
	void *priv;
};



#ifdef __cplusplus
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

#undef EXTERN
#ifdef __cplusplus
}
#endif


#endif
#endif
