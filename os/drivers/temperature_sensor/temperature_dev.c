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
#include<tinyara/temperature/temperature_dev_s.h>



/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int temperature_open(FAR struct file *filep);
static int temperature_close(FAR struct file *filep);
static int temperature_ioctl(FAR struct file *filep, int cmd, unsigned long arg);
static int temperature_read(FAR struct file *filep);
static int temperature_write(FAR struct file *filep);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_temperaturedev_fileops = {
	temperature_open,	/* open */
	temperature_close,	/* close */
	temperature_read,		/* read */
	temperature_write,	/* write */
	NULL,			/* seek */
	temperature_ioctl,	/*ioctl */
	NULL,
};

/************************************************************************************
 * Name: temperature_semtake
 ************************************************************************************/

static inline int temperature_semtake(FAR sem_t *sem, bool errout)
{
	/* Take a count from the semaphore, possibly waiting */
	while (sem_wait(sem) != OK) {
		/* EINTR is the only error that we expect */
		ASSERT(get_errno() == EINTR);
		if (errout) {
			return -EINTR;
		}
	}
	return OK;
}

/****************************************************************************
 * Name: temperature_semgive
 ****************************************************************************/

static inline void temperature_semgive(sem_t *sem)
{
	sem_post(sem);
}

/****************************************************************************
 * Name: temperature_open
 ****************************************************************************/

static int temperature_open(FAR struct file *filep)
{
	FAR struct temperature_dev_s *priv;
	priv = filep->f_inode->i_private;

	if (!priv) {
		return -EINVAL;
	}

	temperature_semtake(&priv->sem, false);
	/*if (priv->crefs == 0) {
		priv->ops->touch_enable(priv);
	}*/
	priv->crefs++;
	DEBUGASSERT(priv->crefs > 0);
	temperature_semgive(&priv->sem);
	return OK;
}

/****************************************************************************
 * Name: temperature_close
 ****************************************************************************/

static int temperature_close(FAR struct file *filep)
{
	FAR struct temperature_dev_s *priv;
	priv = filep->f_inode->i_private;

	if (!priv) {
		return -EINVAL;
	}

	temperature_semtake(&priv->sem,false);
	DEBUGASSERT(priv->crefs > 0);
	priv->crefs--;
	temperature_semgive(&priv->sem);

	return OK;
}

/****************************************************************************
 * Name: temperature_read
 *
 ****************************************************************************/

static int temperature_read(FAR struct file *filep){

	return OK;

}

/****************************************************************************
 * Name: temperature_write
 *
 ****************************************************************************/

static int temperature_write(FAR struct file *filep){

	return OK;

}

/****************************************************************************
 * Name: temperature_ioctl
 *
 * Description:
 *   This IOCTL is used to get or set the sensor data.
 *
 ****************************************************************************/

static int temperature_ioctl(FAR struct file *filep, int cmd, unsigned long arg)
{
	FAR struct temperature_dev_s *priv;
	priv = filep->f_inode->i_private;
	if (!priv) {
		return -EINVAL;
	}

	switch (cmd) {		

		case TEMPDEV_GETSENSORDATA: {
		uint16_t *data=(uint16_t *)arg;
		int ret=priv->ops->get_sensor_data(priv,data);
			
		}
		break;
		default: {
			lldbg("ERROR: ioctl not found, cmd: %d\n", cmd);
		}
		break;
	}
	return OK;
}

/****************************************************************************
 * Name: temperature_register
 *
 * Description:
 *   Register the TEMPERATURE SENSOR driver as /dev/temperatureN.
 *
 * Input Parameters:
 *   dev - The TEMPERATURE SENSOR driver address.
 *
 * Returned Value:
 *   Zero (OK) is returned on success.  Otherwise a negated errno value is
 *   returned to indicate the nature of the failure.
 *
 ****************************************************************************/

int temperature_register(const char *path, struct temperature_dev_s *dev)
{
	sem_init(&dev->sem, 0, 1);
	int ret = register_driver(path, &g_temperaturedev_fileops, 0666, dev);
	if (ret < 0) {
		kmm_free(dev);
		sem_destroy(&dev->sem);
		lldbg("Temperature Driver registration failed\n");
		return ret;
	}
	lldbg("Temperature Driver registered Successfully\n");
	return OK;
}


