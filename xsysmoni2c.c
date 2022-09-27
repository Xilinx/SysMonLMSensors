#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/err.h>

#define XSYSMONI2C_PROC_DENOM 128U
#define XSYSMONI2C_2COMP(X) ((((X) ^ 0xFFFF) + 1U) & 0x0000FFFFU)

static int xsysmoni2c_probe(struct i2c_client *client,const struct i2c_device_id *id);
static int xsysmoni2c_detect(struct i2c_client *client,  struct i2c_board_info *info);
static int xsysmoni2c_remove(struct i2c_client *client);
static int xsysmoni2c_read(struct device *dev, enum hwmon_sensor_types type, u32 attr, int channel, long *val);
static umode_t xsysmoni2c_is_visible(const void *data, enum hwmon_sensor_types type, u32 attr, int channel);

static const unsigned short normal_i2c[] = { 0x18, I2C_CLIENT_END };

static const struct i2c_device_id xsysmoni2c_id[] = {
	{ "xsysmoni2c", 0,},
	{ }
};
MODULE_DEVICE_TABLE(i2c, xsysmoni2c_id);

static const struct hwmon_channel_info *xsysmoni2c_info[] = {
	HWMON_CHANNEL_INFO(temp, HWMON_T_INPUT),
	NULL
};

static const struct hwmon_ops xsysmoni2c_hwmon_ops = {
	.is_visible = xsysmoni2c_is_visible,
	.read = xsysmoni2c_read,
	.write = (void *)NULL,
};

static const struct hwmon_chip_info xsysmoni2c_chip_info = {
	.ops = &xsysmoni2c_hwmon_ops,
	.info = xsysmoni2c_info,
};

static struct i2c_driver xsysmoni2c_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "xsysmoni2c",
	},
	.probe		= xsysmoni2c_probe,
	.remove		= xsysmoni2c_remove,
	.id_table	= xsysmoni2c_id,
	.detect		= xsysmoni2c_detect,
	.address_list	= normal_i2c,
};

struct xsysmoni2c_data {
	struct i2c_client *client;
	struct device *hwmon_dev;
	long temp[1];
};

static int xsysmoni2c_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if (i2c_check_functionality(adapter, I2C_FUNC_I2C) == 0U)
		return -ENODEV;

	strlcpy(info->type, "xsysmoni2c", I2C_NAME_SIZE);

	return 0;
}

static int xsysmoni2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct xsysmoni2c_data *data;

	data = kzalloc(sizeof(struct xsysmoni2c_data), GFP_KERNEL);
	if (!data) {
		return -ENOMEM;
	}

	i2c_set_clientdata(client, data);
	data->client = client;
	data->temp[0] = 0;
	
	data->hwmon_dev = hwmon_device_register_with_info(&client->dev, client->name, data, &xsysmoni2c_chip_info, NULL);
	if (IS_ERR(data->hwmon_dev)) {
		kfree(data);
		return PTR_ERR(data->hwmon_dev);
	}

	return 0;
}

static int xsysmoni2c_remove(struct i2c_client *client)
{
	struct xsysmoni2c_data *data = i2c_get_clientdata(client);
	hwmon_device_unregister(data->hwmon_dev);
	kfree(data);
	return 0;
}

static int __init sensors_xsysmoni2c_init(void)
{
	return i2c_add_driver(&xsysmoni2c_driver);
}

static void __exit sensors_xsysmoni2c_exit(void)
{
	i2c_del_driver(&xsysmoni2c_driver);
}

static umode_t xsysmoni2c_is_visible(const void *data, enum hwmon_sensor_types type,
			       u32 attr, int channel)
{
	return ((type == hwmon_temp) && (attr == hwmon_temp_input))? 0444 : 0;
}

static int xsysmoni2c_read(struct device *dev, enum hwmon_sensor_types type,
		     u32 attr, int channel, long *val)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct xsysmoni2c_data *data = i2c_get_clientdata(client);
	char write_data[8]= {0x00, 0x00, 0x00, 0x00, 0x0c, 0x04, 0x4, 0x00};
	char read_data[4];
	long raw;
	
	if ((type != hwmon_temp) || (attr != hwmon_temp_input)) {
		return -EINVAL;
	}
	
	i2c_master_send(data->client, write_data, 8);
	i2c_master_recv(data->client, read_data, 4);	
	raw = ((read_data[3] << 24) | (read_data[2] << 16) | (read_data[1] << 8) | read_data[0]);
	/* convert to millicelcius */
	*val = ((raw & 0x8000U) ? -(XSYSMONI2C_2COMP(raw)) : raw) * 1000 / XSYSMONI2C_PROC_DENOM;
	return 0;
}

MODULE_AUTHOR("Conall O'Griofa <conall.ogriofa@amd.com>");
MODULE_DESCRIPTION("XSYSMONI2C driver");
MODULE_LICENSE("GPL");

module_init(sensors_xsysmoni2c_init);
module_exit(sensors_xsysmoni2c_exit);