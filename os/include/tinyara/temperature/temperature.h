#include<tinyara/config.h>
#include<tinyara/i2c.h>


/* Temperature_Sensor Device */
struct sensor_dev_s {
	struct i2c_dev_s *i2c;
	struct i2c_config_s i2c_config;
	struct temperature_dev_s *upper; 
	void *priv;		/* Used by the chipset-specific logic */
};
