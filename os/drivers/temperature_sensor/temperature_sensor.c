#include <tinyara/config.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include <assert.h>
#include <errno.h>
#include <debug.h>
#include <tinyara/fs/fs.h>
#include <tinyara/fs/ioctl.h>
#include <tinyara/i2c.h>
#include <tinyara/irq.h>
#include <tinyara/temperature/temperature_dev_s.h>
#include <tinyara/temperature/temperature.h>

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int sensor_read(struct temperature_dev_s *dev, FAR uint16_t *buffer);

/****************************************************************************
 * Private Data
 ****************************************************************************/
struct temperature_ops_s g_sensor_ops = {
	.get_sensor_data = sensor_read,
	
};

/****************************************************************************
 * Name: Calculate_PEC
 *
 * Description: 
 ****************************************************************************/

uint8_t calculate_PEC(uint8_t initPEC, uint8_t newData)
{
	uint8_t data;
        uint8_t bitCheck;
        data = initPEC ^ newData;
        for (int i=0; i<8; i++ ) {
                bitCheck = data & 0x80;
                data = data << 1;
                if (bitCheck != 0) {data = data ^ 0x07;}
        }
        return data;
}

/****************************************************************************
 * Name: check_data
 *
 * Description:
 *	This function verify whether the data received from
 *	sensor is correct or not.
 *
 * Input Parameters:
 *	config_address: Address of temperature sensor
 *	register_address: Address of register inside temperature sensor
 *	buffer: It contains sensor data
 *
 * Return Value:
 * 	None
 *
 ****************************************************************************/

void verify_data(uint8_t config_address, uint8_t register_address, uint8_t *buffer)
{
	uint8_t pec;
        uint8_t sa;
        uint8_t cmd = 0;
        sa = config_address << 1;
	pec = sa;
        cmd = register_address;
        pec = calculate_PEC(0, pec);
        pec = calculate_PEC(pec, cmd);
        pec = calculate_PEC(pec, (sa|0x01));
        pec = calculate_PEC(pec, buffer[0]);
        pec = calculate_PEC(pec, buffer[1]);
        lldbg("reg value: %d, config.address: %x, pec value: %d, buffer[2]: %8x\n", register_address, config_address, pec, buffer[2]);
        if(pec != buffer[2]){
                lldbg("PEC error\n");
        } else {
                lldbg("PEC success\n");
        }
}

/****************************************************************************
 * Name: sensor_read
 *
 * Description: This function gets the temperature sensor data by calling i2c_writeread
 * Input Parameters:
 *	dev: Temperature sensor driver
 *	data: pointer to array passed from application
 *
 * Return Value:
 *	Zero (OK) is returned on success.  Otherwise -1 is
 *	returned on failure
 *
 ****************************************************************************/

static int sensor_read(struct temperature_dev_s *dev, FAR uint16_t *data)
{
	irqstate_t flags;
	struct sensor_dev_s *priv = (struct sensor_dev_s *)dev->priv;
	uint8_t reg[2]={0x06,0};
	uint8_t rbuffer[3];
	int ret=i2c_writeread(priv->i2c,&(priv->i2c_config),reg,1,rbuffer,3);
	if(ret<0) {
	lldbg("i2c_writeread fail!\n");
	return -1;
	}
	verify_data(priv->i2c_config.address,reg[0],rbuffer); //check PEC(Parity Error Checker)
	data[0]=(uint16_t)(rbuffer[0] | rbuffer[1] <<8);
	reg[0]=0x07;
	ret=i2c_writeread(priv->i2c,&(priv->i2c_config),reg,1,rbuffer,3);
	if(ret<0) {
	lldbg("i2c_writeread fail!\n");
	return -1;
	}
	verify_data(priv->i2c_config.address,reg[0],rbuffer); //check PEC(Parity Error Checker)
	data[1]=(uint16_t)(rbuffer[0] | rbuffer[1] <<8);
	priv->i2c_config.address=0x5D;
	reg[0]=0x06;
	ret=i2c_writeread(priv->i2c,&(priv->i2c_config),reg,1,rbuffer,3);
	if(ret<0) {
	priv->i2c_config.address=0x5A;
	lldbg("i2c_writeread fail!\n");
	return -1;
	}
	verify_data(priv->i2c_config.address,reg[0],rbuffer); //check PEC(Parity Error Checker)
	data[2]=(uint16_t)(rbuffer[0] | rbuffer[1] <<8);
	reg[0]=0x07;
	ret=i2c_writeread(priv->i2c,&(priv->i2c_config),reg,1,rbuffer,3);
	if(ret<0) {
	priv->i2c_config.address=0x5A;
	lldbg("i2c_writeread fail!\n");
	return -1;
	}
	verify_data(priv->i2c_config.address,reg[0],rbuffer); //check PEC(Parity Error Checker)
	data[3]=(uint16_t)(rbuffer[0] | rbuffer[1] <<8);
	priv->i2c_config.address=0x5A;
	return OK;
}

/****************************************************************************
 * Name: sensor_initialize
 *
 * Description: 
 * 	This function is called by board-specific, setup logic to configure
 *	and register the Temperature Sensor device.  This function will register the driver
 *	as /dev/temperatureN where N is determined by the minor device number.
 *
 * Input Parameters:
 *	i2c_dev - The i2c device structure for the I2C bus that the temperature sensor device is connected to
 *	config - The lower half temperature sensor driver structure that contains the temperature configuration information.
 *
 *	Returned Value:
 *	On success, a non-NULL pointer to the temperature sensor device structure temperature_dev_s is returned.
 *	On a failure, NULL is returned.
 *
 ****************************************************************************/

int sensor_initialize(const char*path, struct sensor_dev_s *priv)
{
	struct temperature_dev_s *upper = (struct temperature_dev_s *)kmm_zalloc(sizeof(struct temperature_dev_s));
	upper->ops = &g_sensor_ops;
	upper->priv = priv;
	priv->upper = upper;
	lldbg("Temperature Sensor Driver registered Successfully\n");
	return temperature_register(path, upper);
}


