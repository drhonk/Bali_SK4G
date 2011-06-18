/* kxsd9_2042.c
 *
 * also acts as pedometer and free-fall detector
 * reports g-force as x,y,z
 * reports step count
 *
 */

#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/hrtimer.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#ifdef CONFIG_ANDROID_POWER
#include <linux/android_power.h>
#endif

#include "kxsd9_2042.h"

#define MODULE_NAME "kxsd9"

#define KXSD9_DEBUG  1
#define KXSD9_DUMP   0

#define KXSD9_FREE_FALL 0x800
#define KXSD9_WROP_BUF	30

struct kxsd9 {
	struct i2c_client *client;
	struct input_dev *inputdev;
	struct hrtimer timer;
	struct delayed_work work;
	struct mutex lock;
#ifdef CONFIG_ANDROID_POWER
	android_suspend_lock_t suspend_lock;
#endif
	int on,scale,rate,susp;
	unsigned short pedo_count;
	int pedo_up,pedo_lim;

	unsigned short wrop[KXSD9_WROP_BUF];
	int head,tail;
};

#define KXSD9_REG_RST	0x0A
#define KXSD9_RST_KEY	0xCA
#define KXSD9_REG_A	0x0E	// --- --- --- --- --- --- Mhi ---
#define KXSD9_REG_B	0x0D	// ClH ENA ST  -0- -0- Men -0- -0-
#define KXSD9_REG_C	0x0C	// LP2 LP1 LP0 Mle Mla -0- FS1 FS0

#define KXSD9_XOUT_H		0x00
#define KXSD9_XOUT_L		0x01
#define KXSD9_YOUT_H		0x02
#define KXSD9_YOUT_L		0x03
#define KXSD9_ZOUT_H		0x04
#define KXSD9_ZOUT_L		0x05
#define KXSD9_AUXOUT_H		0x06
#define KXSD9_AUXOUT_L		0x07
#define KXSD9_RESET_WRITE	0x0A
#define KXSD9_CTRL_REGC		0x0C
#define KXSD9_CTRL_REGB		0x0D
#define KXSD9_CTRL_REGA		0x0E

#define KXSD9_RESET_KEY		0xCA

#define KXSD9_REGC_LP2		(1 << 7)
#define KXSD9_REGC_LP1		(1 << 6)
#define KXSD9_REGC_LP0		(1 << 5)
#define KXSD9_REGC_MOTLEV	(1 << 4)
#define KXSD9_REGC_MOTLAT	(1 << 3)
#define KXSD9_REGC_FS1		(1 << 1)
#define KXSD9_REGC_FS0		(1 << 0)

enum {	/* operation     	   param */
	KXSD9_CTL_RESET,	// ignored	
	KXSD9_CTL_ENABLE,	// 0 = disabled
	KXSD9_CTL_SCALE,	// 1 (2G) .. 4 (8G)
	KXSD9_CTL_RATE		// samples per 10 seconds
};

typedef struct  {
		short x, /**< holds x-axis acceleration data sign extended. Range -512 to 511. */
			  y, /**< holds y-axis acceleration data sign extended. Range -512 to 511. */
			  z; /**< holds z-axis acceleration data sign extended. Range -512 to 511. */
} kxsd9acc_t;

struct class *acc_class;

int gacc_x = 0;
int gacc_y = 0;
int gacc_z = 0;

int kxsd9_open (struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t kxsd9_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

ssize_t kxsd9_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

int kxsd9_release (struct inode *inode, struct file *filp)
{
	return 0;
}

int kxsd9_ioctl(struct inode *inode, struct file *filp, unsigned int ioctl_num,  unsigned long arg)
{
	kxsd9acc_t accels;
	int err = 0;
	
	printk( "kxsd9_ioctl : receive cmd = %d\n", ioctl_num);
	
	switch( ioctl_num )
	{		case IOCTL_KXSD9_GET_ACC_VALUE :
			{
				printk( "ioctl : IOCTL_KXSD9_GET_ACC_VALUE\n" );
				
				accels.x = gacc_x;
				accels.y = gacc_y;
				accels.z = gacc_z;
				
				if( copy_to_user( (kxsd9acc_t*)arg, &accels, sizeof(kxsd9acc_t) ) )
				{
					err = -EFAULT;
				}   

			}
			break; 

		case IOC_SET_ACCELEROMETER :  
			{
				printk( "ioctl : IOC_SET_ACCELEROMETER\n" );
			}			
			break;  

	/* offset calibration routine */	
		case BMA150_CALIBRATION:
			{
				printk( "ioctl : BMA150_CALIBRATION\n" );
			}			
			break; 

		default:
			{
				printk( "ioctl : No case...\n" );
			}
			break;
	}

	return err;

}

static int kxsd9_wrop_put(struct kxsd9 *kxsd9,unsigned char reg,unsigned char val)
{
	int nt;
	
	nt = (kxsd9->tail + 1) % KXSD9_WROP_BUF;
	if (nt == kxsd9->head) {
		// buffer full
		return -1;
	}
	kxsd9->wrop[kxsd9->tail] = (reg << 8) | val;
	kxsd9->tail = nt;
	return 0;
}

static int kxsd9_wrop_get(struct kxsd9 *kxsd9,char *buf)
{
	if (kxsd9->head == kxsd9->tail) {
		// buffer empty
		return -1;
	}
	buf[0] = kxsd9->wrop[kxsd9->head] >> 8;
	buf[1] = kxsd9->wrop[kxsd9->head] & 0xFF;
	kxsd9->head = (kxsd9->head + 1) % KXSD9_WROP_BUF;
	return 0;
}

int kxsd9_control(struct kxsd9 *kxsd9,int oper,int param)
{
	int restart;
	
#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME ": %s(%d, %d)\n", __func__, oper, param);
#endif
	mutex_lock(&kxsd9->lock);
	restart = (kxsd9->head == kxsd9->tail);
	switch (oper)
	{
		case KXSD9_CTL_RESET:
			kxsd9_wrop_put(kxsd9,KXSD9_REG_RST,KXSD9_RST_KEY);
			kxsd9->pedo_up = kxsd9->pedo_lim = kxsd9->pedo_count = 0;
			break;
			
		case KXSD9_CTL_ENABLE:
			kxsd9->on = !!param;
			kxsd9_wrop_put(kxsd9,KXSD9_REG_B,param ? 0xC0 : 0x00);
			break;
			
		case KXSD9_CTL_SCALE:
			if (param < 1)
				param = 1;
			else if (param > 4)
				param = 4;
			kxsd9->scale = param;
			param = 4 - param;
			kxsd9_wrop_put(kxsd9,KXSD9_REG_C,0xE0 | (param & 0x03));
			break;
		
		case KXSD9_CTL_RATE:
			param &= 0x1FFF;
			restart = (param > kxsd9->rate);
			kxsd9->rate = param;
			break;
	}
	if (restart) {
		hrtimer_start(&kxsd9->timer, ktime_set(0,16 * NSEC_PER_MSEC), 
				HRTIMER_MODE_REL);
	}
	mutex_unlock(&kxsd9->lock);
	return 0;
}

static int kxsd9_i2c_read(struct i2c_client *client, unsigned id,
						char *buf, int len)
{
	int r;
	char outbuffer[2] = { 0, 0 };

	outbuffer[0] = id;
	// maejrep: Have to separate the "ask" and "read" chunks
	r = i2c_master_send(client, outbuffer, 1);
	if (r < 0) {
		printk(KERN_WARNING "%s: error asking for gsensor data at "
			"address %02x,%02x: %d\n",
			__func__, client->addr, id, r);
		return r;
	}
	mdelay(1);
	r = i2c_master_recv(client, buf, len);
	if (r < 0) {
		printk(KERN_ERR "%s: error reading gsensor data at "
			"address %02x,%02x: %d\n",
			__func__, client->addr, id, r);
		return r;
	}
	return 0;
}

static int kxsd9_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	int ret = i2c_smbus_write_byte_data(client, reg, val);

	if (ret < 0)
		dev_err(&client->dev, "%s: reg 0x%x, val 0x%x, err %d\n",
						__func__, reg, val, ret);
	return ret;
}

static int kxsd9_read_reg(struct i2c_client *client, u8 reg)
{
	int ret = i2c_smbus_read_byte_data(client, reg);

	if (ret < 0)
		dev_err(&client->dev, "%s, reg 0x%x, err %d\n",
						__func__, reg, ret);
	return ret;
}

static void kxsd9_read_xyz(struct i2c_client *client,
						s16 *x, s16 *y, s16 *z)
{
	char buf[6];

/*	
	*x = kxsd9_read_reg(client, KXSD9_XOUT_H) << 4 |
				kxsd9_read_reg(client, KXSD9_XOUT_L) >> 4;

	*y = kxsd9_read_reg(client, KXSD9_YOUT_H) << 4 |
				kxsd9_read_reg(client, KXSD9_YOUT_L) >> 4;

	*z = kxsd9_read_reg(client, KXSD9_ZOUT_H) << 4 |
				kxsd9_read_reg(client, KXSD9_ZOUT_L) >> 4;
*/
	kxsd9_i2c_read(client, 0, buf, 6);
	*x = 0x800 - (buf[2] << 4) - (buf[3] >> 4);
	*y = 0x800 - (buf[0] << 4) - (buf[1] >> 4);
	*z = (buf[4] << 4) + (buf[5] >> 4) - 0x800 - 64; // calib?

	*x = (*x/(s16)4);
	*y = (*y/(s16)4);
	*z = (*z/(s16)4);

	return;		

}

static enum hrtimer_restart kxsd9_poll_timer(struct hrtimer *timer)
{
	struct kxsd9 *kxsd9;

	kxsd9 = container_of(timer, struct kxsd9, timer);
#ifdef CONFIG_ANDROID_POWER
	android_lock_suspend(&kxsd9->suspend_lock);
#endif
	schedule_work(&kxsd9->work.work);
	return HRTIMER_NORESTART;
}

static void kxsd9_work(struct work_struct *work)
{
	struct kxsd9 *kxsd9;
	int err;
	char buf[6];
	int x,y,z;
	unsigned long long gabs;
	ktime_t restart_time = {0};
	char kxsd9_ctl_reg = 0xFF;

	kxsd9 = container_of(work, struct kxsd9, work.work);
	
	//printk("[%s] : kxsd9->susp = %d, kxsd9->on = %d\n", __func__, kxsd9->susp, kxsd9->on);
	if (kxsd9->susp == 1){
		restart_time.tv.nsec = (10000 / kxsd9->rate) * NSEC_PER_MSEC;
		hrtimer_start(&kxsd9->timer, restart_time, HRTIMER_MODE_REL);
		printk("[%s] : will not read kxsd regs.\n", __func__);
		
		return;
	}
	
	kxsd9_i2c_read(kxsd9->client, KXSD9_REG_B, &kxsd9_ctl_reg, 1);
	if( (kxsd9->susp == 0) && (kxsd9_ctl_reg == 0x0)){
		printk("[%s] : KXSD9_REG_B = %x\n", __func__, kxsd9_ctl_reg);
		msleep(100);
		kxsd9_control(kxsd9,KXSD9_CTL_ENABLE,1);
		msleep(50);
	}
	
	mutex_lock(&kxsd9->lock);
	
	if (kxsd9_wrop_get(kxsd9,buf) == 0) {
#if KXSD9_DEBUG
		printk(KERN_INFO MODULE_NAME ": write %02x to %02x\n",
				buf[1], buf[0]);
#endif
		err = i2c_master_send(kxsd9->client, buf, 2);
		if (err < 0) {
			printk(KERN_WARNING MODULE_NAME 
					": %s: error %d\n", __func__, err);
		}
		restart_time.tv.nsec = 4 * NSEC_PER_MSEC;
		hrtimer_start(&kxsd9->timer, restart_time, HRTIMER_MODE_REL);
	} else {
	
		err = kxsd9_i2c_read(kxsd9->client, 0, buf, 6);

		x = 0x800 - ((buf[0] << 4) + (buf[1] >> 4));
		y = 0x800 - ((buf[2] << 4) + (buf[3] >> 4));			
		z = 0x800 - ((buf[4] << 4) + (buf[5] >> 4)); 

		x = (1000* x) / 819;
		y = (1000* y) / 819;
		z = (1000* z) / 819;

		x = x / 4;
		y = y / 4;	
		z = z / 4; 

		gacc_x = x;
		gacc_y = y;
		gacc_z = z;
		
		// detect step
		gabs = x * x + y * y + z * z;
		if (kxsd9->pedo_up) {
			if (gabs > kxsd9->pedo_lim) {
				kxsd9->pedo_up = 0;
				kxsd9->pedo_lim = gabs / 2;
				kxsd9->pedo_count++;
				//input_report_abs(kxsd9->inputdev, ABS_GAS, kxsd9->pedo_count);
			} else if (kxsd9->pedo_lim > gabs * 2) {
				kxsd9->pedo_lim = gabs * 2;
			}
		} else {
			if (gabs < kxsd9->pedo_lim) {
				kxsd9->pedo_up = 1;
				kxsd9->pedo_lim = gabs * 2;
			} else if (kxsd9->pedo_lim < gabs / 2) {
				kxsd9->pedo_lim = gabs / 2;
			}
		}

#if KXSD9_DUMP
#if 1
		printk("G=(%6d, %6d, %6d) P=%d %s\n", x, y, z, kxsd9->pedo_count,	gabs < KXSD9_FREE_FALL ? "FF" : ""); // free-fall
#else
		printk(KERN_INFO "G=( %02X %02X  %02X %02X  %02X %02X )\n", 
				buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
#endif
#endif
		//input_report_abs(kxsd9->inputdev, ABS_X, x);
		//input_report_abs(kxsd9->inputdev, ABS_Y, y);
		//input_report_abs(kxsd9->inputdev, ABS_Z, z);
		//input_sync(kxsd9->inputdev);
		
		if (kxsd9->on && kxsd9->rate && !kxsd9->susp)
		{
			restart_time.tv.nsec = (10000 / kxsd9->rate)
					* NSEC_PER_MSEC;
			hrtimer_start(&kxsd9->timer, restart_time,
					HRTIMER_MODE_REL);
		}
#ifdef CONFIG_ANDROID_POWER
		android_unlock_suspend(&kxsd9->suspend_lock);
#endif
	}
	mutex_unlock(&kxsd9->lock);
}


static ssize_t kxsd9_fs_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;

	printk("kxsd9_fs_read --> x: %d,y: %d,z: %d\n", gacc_x, gacc_y, gacc_z );
	count = sprintf(buf,"%d,%d,%d\n", gacc_x, gacc_y, gacc_z );

	return count;
}

static ssize_t kxsd9_fs_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	//buf[size]=0;
	printk("kxsd9_fs_write --> input data --> %s\n", buf);

	return size;
}
static DEVICE_ATTR(acc_file, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, kxsd9_fs_read, kxsd9_fs_write);

static ssize_t kxsd9_ctl_rate_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(to_i2c_client(dev));

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME " %s\n", __func__);
#endif
	return sprintf(buf, "%u\n", kxsd9 ? kxsd9->rate : 0);
}

static ssize_t kxsd9_ctl_rate_store(struct device *dev, struct device_attribute *attr,
				const char *buf,size_t count)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(to_i2c_client(dev));
	unsigned long val = simple_strtoul(buf, NULL, 10);

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME " %s\n", __func__);
#endif
	kxsd9_control(kxsd9,KXSD9_CTL_RATE,val);
        return count;
}

static ssize_t kxsd9_ctl_scale_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(to_i2c_client(dev));

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME " %s\n", __func__);
#endif
	return sprintf(buf, "%u\n", kxsd9 ? kxsd9->scale : 0);
}

static ssize_t kxsd9_ctl_scale_store(struct device *dev, struct device_attribute *attr,
				const char *buf,size_t count)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(to_i2c_client(dev));
	unsigned long val = simple_strtoul(buf, NULL, 10);

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME " %s\n", __func__);
#endif
	kxsd9_control(kxsd9,KXSD9_CTL_SCALE,val);
        return count;
}

static ssize_t kxsd9_ctl_enable_show(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(to_i2c_client(dev));

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME " %s\n", __func__);
#endif
	return sprintf(buf, "%u\n", kxsd9 && kxsd9->on ? 1 : 0);
}

static ssize_t kxsd9_ctl_enable_store(struct device *dev, struct device_attribute *attr,
				const char *buf,size_t count)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(to_i2c_client(dev));
	unsigned long val = simple_strtoul(buf, NULL, 10);

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME " %s\n", __func__);
#endif
	kxsd9_control(kxsd9,KXSD9_CTL_ENABLE,!!val);
        return count;
}

struct device_attribute kxsd9_sysfs_ctl_rate = 
{
	.attr = {	.name = "rate",
			.mode = S_IWUSR | S_IRUGO },
	.show	= kxsd9_ctl_rate_show,
	.store	= kxsd9_ctl_rate_store,
};

struct device_attribute kxsd9_sysfs_ctl_scale = 
{
	.attr = {	.name = "scale",
			.mode = S_IWUSR | S_IRUGO },
	.show	= kxsd9_ctl_scale_show,
	.store	= kxsd9_ctl_scale_store,
};

struct device_attribute kxsd9_sysfs_ctl_enable = 
{
	.attr = {	.name = "enable",
			.mode = S_IWUSR | S_IRUGO },
	.show	= kxsd9_ctl_enable_show,
	.store	= kxsd9_ctl_enable_store,
};

struct file_operations acc_fops =
{
	.owner   = THIS_MODULE,
	.read    = kxsd9_read,
	.write   = kxsd9_write,
	.open    = kxsd9_open,
	.ioctl   = kxsd9_ioctl,
	.release = kxsd9_release,
};

static int kxsd9_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct kxsd9 *kxsd9;
	//struct input_dev *idev;

	struct device *dev_t;
	int result;

	printk("kxsd9_probe : Initializing KXSD9 driver at addr: 0x%02x\n", client->addr);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		printk(KERN_ERR MODULE_NAME ": i2c bus not supported\n");
		return -EINVAL;
	}

	kxsd9 = kzalloc(sizeof *kxsd9, GFP_KERNEL);
	if (kxsd9 < 0) {
		printk(KERN_ERR MODULE_NAME ": Not enough memory\n");
		return -ENOMEM;
	}
	mutex_init(&kxsd9->lock);
	kxsd9->client = client;
	i2c_set_clientdata(client, kxsd9);
/*
	idev = input_allocate_device();
	if (idev) {
		idev->name = MODULE_NAME;
		set_bit(EV_ABS, idev->evbit);
		input_set_abs_params(idev, ABS_X, -2048, 2047, 0, 0);
		input_set_abs_params(idev, ABS_Y, -2048, 2047, 0, 0);
		input_set_abs_params(idev, ABS_Z, -2048, 2047, 0, 0);
		input_set_abs_params(idev, ABS_GAS, 0, 65535, 0, 0);
		if (!input_register_device(idev)) {
			kxsd9->inputdev = idev;			
		} else {
			kxsd9->inputdev = 0;
			printk(KERN_ERR MODULE_NAME 
					": Failed to register input device\n");
		}
	}
*/
	result = register_chrdev( ACC_DEV_MAJOR, ACC_DEV_NAME, &acc_fops);

	if (result < 0) 
	{
		return result;
	}
	
	acc_class = class_create (THIS_MODULE, "accelerometer");

	if (IS_ERR(acc_class)) 
	{
		return PTR_ERR( acc_class );
	}
	
	dev_t = device_create( acc_class, NULL, MKDEV(ACC_DEV_MAJOR, 0), "%s", "accelerometer");

	if (device_create_file(dev_t, &kxsd9_sysfs_ctl_enable) != 0)
		printk(KERN_ERR MODULE_NAME ": Failed to create 'enable' file\n");
	if (device_create_file(dev_t, &kxsd9_sysfs_ctl_scale) != 0)
		printk(KERN_ERR MODULE_NAME ": Failed to create 'scale' file\n");
	if (device_create_file(dev_t, &kxsd9_sysfs_ctl_rate) != 0)
		printk(KERN_ERR MODULE_NAME ": Failed to create 'rate' file\n");
	if (device_create_file(dev_t, &dev_attr_acc_file) < 0)
		printk("Failed to create device file(%s)!\n", dev_attr_acc_file.attr.name);

//	if (device_create_file(dev_t, &dev_attr_calibration) < 0)
//		printk("Failed to create device file(%s)!\n", dev_attr_calibration.attr.name);

	if (IS_ERR(dev_t)) 
	{
		return PTR_ERR(dev_t);
	}
	
#ifdef CONFIG_ANDROID_POWER
	kxsd9->suspend_lock.name = MODULE_NAME;
	android_init_suspend_lock(&kxsd9->suspend_lock);
	android_lock_suspend(&kxsd9->suspend_lock);
#endif
	INIT_DELAYED_WORK(&kxsd9->work, kxsd9_work);
	hrtimer_init(&kxsd9->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	kxsd9->timer.function = kxsd9_poll_timer;
	kxsd9_control(kxsd9,KXSD9_CTL_RESET,0);
	kxsd9_control(kxsd9,KXSD9_CTL_SCALE,1);
	kxsd9_control(kxsd9,KXSD9_CTL_RATE,100);
#if KXSD9_DEBUG
	kxsd9_control(kxsd9,KXSD9_CTL_ENABLE,1);
#else
	kxsd9_control(kxsd9,KXSD9_CTL_ENABLE,0);
#endif

	kxsd9_write_reg(client, KXSD9_REG_B, 0xC0);

	printk("kxsd9_probe : Initializing KXSD9 driver end\n");
	return 0;
}

static int kxsd9_remove(struct i2c_client * client)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(client);

	//input_unregister_device(kxsd9->inputdev);
	//input_free_device(kxsd9->inputdev);
#ifdef CONFIG_ANDROID_POWER
	android_uninit_suspend_lock(&kxsd9->suspend_lock);
#endif
	kfree(kxsd9);
	return 0;
}

#if CONFIG_PM
static int kxsd9_suspend(struct i2c_client * client, pm_message_t mesg)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(client);

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME ": suspending device...\n");
#endif
	kxsd9->susp = 1;
	if (kxsd9->on) {
		kxsd9_control(kxsd9,KXSD9_CTL_ENABLE,0);
		kxsd9->on = 1;
	}
	
	kxsd9_write_reg(client, KXSD9_REG_B, 0x00);	
	
	return 0;
}

static int kxsd9_resume(struct i2c_client * client)
{
	struct kxsd9 *kxsd9 = i2c_get_clientdata(client);

#if KXSD9_DEBUG
	printk(KERN_INFO MODULE_NAME ": resuming device...\n");
#endif
	kxsd9->susp = 0;
	if (kxsd9->on){
		kxsd9_control(kxsd9,KXSD9_CTL_ENABLE,1);
	}
	
	return 0;
}
#else
#define kxsd9_suspend NULL
#define kxsd9_resume NULL
#endif

static const struct i2c_device_id kxsd9_ids[] = {
        { MODULE_NAME, 0 },
        { }
};

static struct i2c_driver kxsd9_driver = {
	.driver = {
		.name	= MODULE_NAME,
		.owner	= THIS_MODULE,
	},
	.id_table = kxsd9_ids,
	.probe = kxsd9_probe,
	.remove = kxsd9_remove,
#if CONFIG_PM
	.suspend = kxsd9_suspend,
	.resume = kxsd9_resume,
#endif
};

static int __init kxsd9_init(void)
{
	printk(KERN_INFO MODULE_NAME ": Registering KXSD9 driver\n");
	return i2c_add_driver(&kxsd9_driver);
}

static void __exit kxsd9_exit(void)
{
	printk(KERN_INFO MODULE_NAME ": Unregistered KXSD9 driver\n");
	i2c_del_driver(&kxsd9_driver);
}

MODULE_AUTHOR("Job Bolle");
MODULE_DESCRIPTION("KXSD9 driver");
MODULE_LICENSE("GPL");

module_init(kxsd9_init);
module_exit(kxsd9_exit);


