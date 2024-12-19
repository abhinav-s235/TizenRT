
#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <assert.h>
#include <errno.h>
#include <tinyara/irq.h>
#include <tinyara/gpio.h>
#include <tinyara/temperature/temperature_dev_s.h>
#include <tinyara/temperature/temperature.h>
#include "objects.h"
#include "gpio_irq_api.h"
#include "PinNames.h"
#include "gpio_api.h"

#define PIN_LOW		0
#define PIN_HIGH	1
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* i2c config */

#define SENSOR_I2C_PORT		2


#define SENSOR_I2C_FREQ		50000
#define SENSOR_I2C_ADDRLEN	7
#define SENSOR_I2C_ADDR		(0x5A) 

/****************************************************************************
 * Private Data
 ****************************************************************************/


static struct sensor_dev_s g_sensor_dev0={
	.i2c=NULL,
	.i2c_config = {
		.frequency = SENSOR_I2C_FREQ,
		.address = SENSOR_I2C_ADDR,
		.addrlen = SENSOR_I2C_ADDRLEN,
	},
};

/****************************************************************************
 * Name: rtl8730e_sensor_initialize
 *
 * Description: 
 *	This function is called by boot, It setup logic to configure and register
 *	the temperature sensor driver. This function will register the driver as
 *	/dev/temperatureN where N is determined the minor device number.
 *
 *	Input Parameters:
 *	None
 *
 *	Return Value:
 *	None
 *
 ****************************************************************************/

void rtl8730e_sensor_initialize(void)
{
	FAR struct i2c_dev_s *i2c;
	i2c = up_i2cinitialize(SENSOR_I2C_PORT);
	if(!i2c) {
	lldbg("ERROR: Failed to initialize I2C\n");
	return;
	}
	g_sensor_dev0.i2c=i2c;
	int ret = sensor_initialize(TEMPERATURE_SENSOR_DEV_PATH,&g_sensor_dev0);
	if(ret<0){
	lldbg("ERROR: Temperature driver register fail\n");
	return;
	}
	lldbg("Temperature driver register success\n");
}

