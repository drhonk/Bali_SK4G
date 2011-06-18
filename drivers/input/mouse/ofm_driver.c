/********************************************************************************
* (C) COPYRIGHT 2010 PARTRON							*
*										*
* File Name : ofm_driver.c							*
*										*
* Author    : Soo-Hwan Kim 							*
*	       Sang-Su Hong 							*
*										*
* Version   : V1.1								*
* Date      : 18/MAY/2010							*
*										*
* Modified By: Aakash Manik								*
* 			aakash.manik@samsung.com					*
*														*
*														*
* Version	: V1.2										*
* Date		: 04/September/2010							*
*														*
* Target CPU : Samsung S3C6410 Processor					*
* Linux Kernel : 2.6.29									*
* Android Ver : 1.5r2 Cupcake							*
*										*
* Version info									*
* v1.1 : 10/JUN/2010 	-  movements improved 			*
						(I2C / read_word_data)			*
*			-  ofm i2c address 0xA6   				*
* 			-  Author <Soo-Hwan Kim>				*
*										*
* Version info											*
* v1.2 : 04/September/2010 - driver configuration for 	*
* 			- Motion Pin Polling Mechanism				*
*			- Author <Aakash Manik>						*
********************************************************************************/

#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <linux/leds.h>
#include <linux/irq.h>
#include <linux/fs.h>
#include <linux/wakelock.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/io.h>

#include <asm/mach-types.h>
#include <linux/miscdevice.h>

#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#include <linux/time.h>
#include <linux/workqueue.h>

#include <linux/regulator/max8998.h>
#include <mach/max8998_function.h>

#define I2C_READ_WORD_DATA 0	//Byte reading supported NAGSM_Android_SEL_Kernel_Aakash_20100904
#define OFM_DEBUG 0
#define OFM_DEBUG_DATA 0	//For Minimal DEBUG Messages Enable this tag NAGSM_Android_SEL_Kernel_Aakash_20100904

static void ofm_motion_func(struct work_struct *work);

struct ofm_pin{
	int 	pin;
    	int 	pin_setting;
    	int 	irq;
        char	*name;
};

struct ofm{
	//struct 	delayed_work work;		Delayed work queue mechanism is removed in V1.2 //NAGSM_Android_SEL_Kernel_Aakash_20100904
	struct work_struct work;	//NAGSM_Android_SEL_Kernel_Aakash_20100904
	struct	i2c_client *client;
	struct	input_dev *input;
	struct	ofm_pin *ofm_power_dn;
	struct	ofm_pin *ofm_motion;
	struct	ofm_pin *ofm_left;
	struct hrtimer timer;		//NAGSM_Android_SEL_Kernel_Aakash_20100904 Introducing Timer for polling mechanism in Version1.2

};

static struct ofm_pin ofm_power_dn ={GPIO_OJ_SHDN, S3C_GPIO_OUTPUT, 0,   "power_down"};
static struct ofm_pin ofm_motion  ={GPIO_OJ_MOTION, S3C_GPIO_SFN(3),0, "motion"};	//Connected to data pin for polling method in version1.2	//NAGSM_Android_SEL_Kernel_Aakash_20100904
static struct ofm_pin ofm_left	 ={0,0,0, "ofm_left"};	//NC //NAGSM_Android_SEL_Kernel_Aakash_20100904

extern int backlight_level;

struct ofm	*gofm;
int OFM_Driver_Type = 0;	// 1 -> partron, 2 -> CrucialTec

#define OFM_PARTRON_DRIVER	0x01
#define OFM_CRUCIAL_DRIVER		0x02
#define OFM_OTHER_DRIVER		0x03

#define CUT_VALUE 	7
#define MINUS_ACC	10
#define MAX_MOVING	30

struct class *ojkey_class;
#define OJ_DEV_MAJOR 239

#define IOCTL_OJCTL_POWER_ON		0x01
#define IOCTL_OJCTL_POWER_OFF		0x02

//#define OFM_TIMER_CALL_TIME   100000
#define OFM_TIMER_CALL_TIME_FOR_CRUCIAL	20000
#define OFM_TIMER_CALL_TIME_FOR_PARTON	50000

#define OFM_COUNTER_NO_MOTION  30
#define OFM_COUNTER_FILTER_1   30

struct workqueue_struct *ofm_workqueue;

static int ofm_i2c_write(struct i2c_client *client, u_int8_t index, u_int8_t data);

static void Ofm_IOCTL_Power_Off(void)
{
	max8998_ldo_disable_direct(MAX8998_LDO16);	// disable OJ_2.8V
}

static void Ofm_IOCTL_Power_On(void)
{
	max8998_ldo_enable_direct(MAX8998_LDO16);	// enable OJ_2.8V

	mdelay(4);
	ofm_i2c_write(gofm->client,0x05,0x2D);
	udelay(10);
	ofm_i2c_write(gofm->client,0x29,0x04);		
	udelay(10);
	ofm_i2c_write(gofm->client,0x27,0x1A);
	udelay(10);
	ofm_i2c_write(gofm->client,0x2A,0x04);
	udelay(10);
	ofm_i2c_write(gofm->client,0x2B,0x04);
	udelay(10);
}

static Ofm_Power_Reset(void)
{
	Ofm_IOCTL_Power_Off();
	mdelay(3);
	Ofm_IOCTL_Power_On();
}

//motion Workqueue
static void ofm_motion_func(struct work_struct *work){
	struct ofm *ofm = container_of(work,struct ofm,work);	//NAGSM_Android_SEL_Kernel_Aakash_20100904
	struct input_dev *input = ofm->input;
	
	s32 error;
	s8  x,y, acc_x = 0, acc_y = 0;
	u16 feature_count;
	u8  feature_cnt_high, feature_cnt_low, exposure;
	int motion_val, i;

	static u32  no_motion_count = 0;
	static u32  filter_1_count = 0;
	
	static s8 accumulate_x = 0;
	static s8 accumulate_y = 0;
	static int oj_time_interval = 0;
	
	#if OFM_DEBUG
		printk("OFM : XY\n");
	#endif	
	
#if I2C_READ_WORD_DATA
/*******READ_WORD_DATA***********************************************************
*										*
*   	LOW  DATA = X								*
*	HIGH DATA = Y								*
*	WORDDATA =LOWDATA(X)+HIGHDATA(Y)					*
*										*
********************************************************************************/
	error = i2c_smbus_read_word_data(ofm->client,0x21);
	
	if(error<0){
		
			printk("OFM : 12C WORKD ERROR return(%d)\n",error);
		
		//goto out;
	}
	#if OFM_DEBUG
		printk("WORD DATA :%4X\n",error);
	#endif
	x = (error&0xff);
	y = ((error>>8)&0xff);



#else
/*******READ_BYTE_DATA***********************************************************
*										*	
*	REG 0x21 = X DATA							*
*	REG 0x22 = Y DATA							*
*										*
********************************************************************************/
	if(OFM_Driver_Type == OFM_PARTRON_DRIVER){
		
	if(gpio_get_value(ofm->ofm_motion->pin) == 0)
	{
		no_motion_count ++;
        if(no_motion_count > OFM_COUNTER_NO_MOTION)
		{
			Ofm_Power_Reset();
#if OFM_DEBUG
            printk("No Motion 1.5 second\n");
#endif
			no_motion_count = 0;
		}
		goto out;
	}
	no_motion_count = 0;

    error = i2c_smbus_read_byte_data(ofm->client,0x05);
    if(0 == error)
    {
        Ofm_Power_Reset();
        printk("0x05 is 0 -> Reset\n");
        goto out;
    }

	for(i = 0; i < 2; i ++)
	{
        if(gpio_get_value(ofm->ofm_motion->pin) == 0) // if no motion, break the loop
		{
			break;
		}
	udelay(10);

	error = i2c_smbus_read_byte_data(ofm->client,0x22); // Y read
	if(error<0){
            Ofm_Power_Reset();
		printk( "OFM : i2c error return(%d)\n",error);	
		goto out;
	}
	x = -error;
	udelay(10);
	
	error = i2c_smbus_read_byte_data(ofm->client,0x21); // X read
	if(error<0){
            Ofm_Power_Reset();
		printk( "OFM : i2c error return(%d)\n",error);	
		goto out;
	}
	y = error;	 
	
		if((abs(x) < 2) &&  (abs(y) < 2))
		{
            filter_1_count ++;
            if(filter_1_count > OFM_COUNTER_FILTER_1)
	{
				Ofm_Power_Reset();
                printk("Filter 1 count over -> Reset\n");
                filter_1_count = 0;
				goto out;
			}
#if OFM_DEBUG
            printk( "[Filtered] 1\n");
#endif
			continue;
		}
        filter_1_count = 0;
		
		exposure = i2c_smbus_read_byte_data(ofm->client,0x4F); // exposure read
			
#if OFM_DEBUG
		printk( "exposure: %d\n", exposure);	
#endif
			
		if(error<0){
            Ofm_Power_Reset();
			printk( "OFM : i2c error return(%d)\n",error);	
			goto out;
	}
		
		feature_cnt_high = i2c_smbus_read_byte_data(ofm->client,0x31);
		if(error<0){
            Ofm_Power_Reset();
			printk( "OFM : i2c error return(%d)\n",error);	
			goto out;
	}
		
		feature_cnt_low = i2c_smbus_read_byte_data(ofm->client,0x32);
		if(error<0){
            Ofm_Power_Reset();
			printk( "OFM : i2c error return(%d)\n",error);	
			goto out;
	}
	
		feature_count = (0xFF00 & (feature_cnt_high<<8)) | (0xFF & (feature_cnt_low));
			
#if OFM_DEBUG
		printk( "feature_count: %d\n", feature_count);	
#endif

			if((exposure > 160) || ((abs(x) < 3) &&  (abs(y) < 3)
			      && ((feature_count < 350) || (feature_count > 950) || (exposure > 120))))
	{
			printk( "[Filtered] 2\n");	
			continue;
		}
		
		acc_x += x;
		acc_y += y;
	}

		if (acc_x == 0 && acc_y == 0)			//NAGSM_Android_SEL_Kernel_Aakash_20100924 Donot report if RelX and RelY is zero
			goto out;

		printk("OFM v1.1 : X :[%3d]  Y:[%3d]\n",acc_x,acc_y); 

		if(acc_x>15)
			acc_x = (acc_x-15) / 12 +15;
		else if(acc_x<-15)
			acc_x = (acc_x+15) / 12 -15;
		
		if(acc_y>15)
			acc_y = (acc_y-15) / 12+15;
		else if(acc_y<-15)
			acc_y = (acc_y+15) / 12 -15;	

		input_report_rel(input,REL_X,acc_x);
		input_report_rel(input,REL_Y,acc_y);
		input_sync(input);  // X / Y DATA SEND		
		
	}else if(OFM_Driver_Type == OFM_CRUCIAL_DRIVER){

		#define OJ_DEGREE 7
		
		const unsigned int oj_sht_tbl[OJ_DEGREE] = {30, 1750, 2000, 2250, 2500, 2750, 2929};
		const unsigned char oj_pxsum_tbl[OJ_DEGREE] = {0, 0, 20, 30, 40, 50, 60};
		int i, sht;
		unsigned char motion, shutter_upper, shutter_lower, pxsum, squal;
		char delta_x, delta_y, app_code =0;
		
#if 1		
	
		if(gpio_get_value(GPIO_OJ_MOTION_C)){
			oj_time_interval++;

			if(oj_time_interval > 2){
				accumulate_x = accumulate_y = 0;
				oj_time_interval = 0;
			}
				
			goto out;
		}else{
			oj_time_interval = 0;
		}
#else
		if(i2c_smbus_read_byte_data(ofm->client,0x02) & 0x80 != 0x80){
			return;
		}
#endif
		
		shutter_upper = i2c_smbus_read_byte_data(ofm->client,0x06);
		shutter_lower = i2c_smbus_read_byte_data(ofm->client,0x07);
		pxsum = i2c_smbus_read_byte_data(ofm->client,0x09);
		
		acc_x = i2c_smbus_read_byte_data(ofm->client,0x03);
		acc_y = i2c_smbus_read_byte_data(ofm->client,0x04);
		
		sht = (shutter_upper<<8) | shutter_lower;
		
		for(i=1; i<OJ_DEGREE; i++)
		{
			if( ((oj_sht_tbl[i-1]<sht)&&(sht<=oj_sht_tbl[i])) && (oj_pxsum_tbl[i]<pxsum) )
			{
				app_code = 1;
			}
		}

		if(app_code == 0)
			return;

		acc_x *= -1;

	if (acc_x == 0 && acc_y == 0)			//NAGSM_Android_SEL_Kernel_Aakash_20100924 Donot report if RelX and RelY is zero
	goto out;

	if(acc_x>CUT_VALUE)
		acc_x = (acc_x-CUT_VALUE) / MINUS_ACC +CUT_VALUE;
	else if(acc_x<-CUT_VALUE)
		acc_x = (acc_x+CUT_VALUE) / MINUS_ACC -CUT_VALUE;
	
	if(acc_y>CUT_VALUE)
		acc_y = (acc_y-CUT_VALUE) / MINUS_ACC+CUT_VALUE;
	else if(acc_y<-CUT_VALUE)
		acc_y = (acc_y+CUT_VALUE) / MINUS_ACC -CUT_VALUE;
	

		//printk("OFM v1.1 : X :[%3d]  Y:[%3d]\n",acc_x,acc_y); 
		
		accumulate_x = accumulate_x + acc_x;
		accumulate_y = accumulate_y + acc_y;

		if((accumulate_x > MAX_MOVING) || (accumulate_y > MAX_MOVING)){
			if(accumulate_x > MAX_MOVING){
				accumulate_x  = 25;
				accumulate_y  = 0;
			}

			if(accumulate_y > MAX_MOVING){
				accumulate_x  = 0;
				accumulate_y  = 25;
			}
		}else if((accumulate_x < -MAX_MOVING) || (accumulate_y < -MAX_MOVING)){
			if(accumulate_x < -MAX_MOVING){
				accumulate_x  = -25;
				accumulate_y  = 0;
			}

			if(accumulate_y < -MAX_MOVING){
				accumulate_x  = 0;
				accumulate_y  = -25;
			}
		}else{
			return;
	}

		//printk("OFM v1.1 : accumulate_x :[%3d]  accumulate_y:[%3d]\n",accumulate_x,accumulate_y);
	
		//input_report_rel(input,REL_X,acc_x);
		//input_report_rel(input,REL_Y,acc_y);
		input_report_rel(input,REL_X,accumulate_x);
		input_report_rel(input,REL_Y,accumulate_y);
	input_sync(input);  // X / Y DATA SEND

		accumulate_x = accumulate_y = 0;
		
	}
		
#endif	


#if OFM_DEBUG_DATA
	printk("OFM v1.1 : X :[%3d]  Y:[%3d]\n",acc_x,acc_y); 	
	printk("OFM : motion state[%d]\n",gpio_get_value(ofm->ofm_motion->pin));
#endif		
	
out: 
	return;
	//enable_irq(ofm->ofm_motion->irq);

}
//NAGSM_Android_SEL_Kernel_Aakash_20100904 Interrupt Handler below is not utilised
static irqreturn_t ofm_motion_event(int irq, void *dev_id/*, struct pt_regs *regs*/)
{
	struct ofm *ofm = (struct ofm *)dev_id;

	if (!ofm){
		printk("OFM :ofm_motion_event interrupt error \n");
	return IRQ_HANDLED;
	}
	disable_irq_nosync(irq);

	//schedule_delayed_work(&ofm->work, 0 /*HZ/50*/);
	
	return IRQ_HANDLED;
}

//NAGSM_Android_SEL_Kernel_Aakash_20100904 Interrupt Handler below is not utilised
static irqreturn_t ofm_left_event(int irq, void *dev_id/*, struct pt_regs *regs*/)
{
	struct ofm *ofm = (struct ofm *)dev_id;
	int down;
	if (!ofm){
		printk("OFM : ofm_left_event interrupt error \n");
	return IRQ_HANDLED;
	}
	
	disable_irq_nosync(irq);
	
//	down = s3c_gpio_getpin(ofm->ofm_left->pin) ? 0 : 1; //  KEY_RELEASED : KEY_PRESSED
	down = gpio_get_value(ofm->ofm_left->pin) ? 0 : 1; //  KEY_RELEASED : KEY_PRESSED	

	input_report_key(ofm->input, BTN_LEFT, down);
	input_sync(ofm->input);
		if(down)
		printk("OFM : BUTTON DOWN (%d)\n",down);
		else
		printk("OFM : BUTTON UP   (%d)\n",down); 	
	
	
	
	enable_irq(irq);
	return IRQ_HANDLED;
}

	
static int ofm_i2c_write(struct i2c_client *client, u_int8_t index, u_int8_t data)
{
	int error;
	//u_int8_t buf[2] = {index , data};	
	//error= i2c_master_send(client, buf, 2);
	error = i2c_smbus_write_byte_data(client,index,data);//NAGSM_Android_SEL_Kernel_Aakash_20100904 Smbus write byte data mechanism in V1.2
	
		
		if(error>=0)
	{
		//printk("OFM : Success i2c send!!!index(%x) data(%x) return (%x)\n",index,data,error);
			return 0;
	}
	
#if OFM_DEBUG_DATA
	printk("OFM : ERROR i2c send!!!index(%x) data(%x) return (%x)\n",index,data,error);
#endif
	return error;
}

static int ofm_initial_reg(struct i2c_client *client)
{
	int err_i2c = 0;
	u8 temp;

	struct ofm *ofm = i2c_get_clientdata(client);

	//manual mode patch
	udelay(200);
	temp = i2c_smbus_read_byte_data(ofm->client,0x05);



	err_i2c = ofm_i2c_write(ofm->client,0x05,0x2D);
	if(err_i2c < 0)
		printk("OFM: Initialize I2C Error1");
	
	udelay(10);
	
	err_i2c = ofm_i2c_write(ofm->client,0x29,0x04);
	if(err_i2c < 0)
		printk("OFM: Initialize I2C Error1");
	
	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x27,0x1A);
	if(err_i2c < 0)
		printk("OFM: Initialize I2C Error2");

	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x2A,0x04);
	if(err_i2c < 0)
		printk("OFM: Initialize I2C Error3");

	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x2B,0x04);
	if(err_i2c < 0)
		printk("OFM: Initialize I2C Error4");

	udelay(10);
	

	err_i2c = ofm_i2c_write(ofm->client,0x0E,0x67);
	if(err_i2c < 0)
		printk("OFM: Initialize I2C Error5");

	udelay(10);

	
	return 0;
}

//NAGSM_Android_SEL_Kernel_Aakash_20100904 Timer function for polling method implemented V1.2
static enum hrtimer_restart ofm_timer_func(struct hrtimer *timer)
{
	struct ofm *ofm = container_of(timer, struct ofm, timer);
	ktime_t ofm_polling_time;			
	
	queue_work(ofm_workqueue, &ofm->work);
	//hrtimer_start(&gp2a->timer,ktime_set(LIGHT_PERIOD,0),HRTIMER_MODE_REL);
	ofm_polling_time = ktime_set(0,0);
	
	if(OFM_Driver_Type == OFM_CRUCIAL_DRIVER){
		if(backlight_level == 0){
			ofm_polling_time = ktime_add_us(ofm_polling_time, 2000000);
		}else{
			ofm_polling_time = ktime_add_us(ofm_polling_time, OFM_TIMER_CALL_TIME_FOR_CRUCIAL);		
		}
	}else{
		ofm_polling_time = ktime_add_us(ofm_polling_time, OFM_TIMER_CALL_TIME_FOR_PARTON);		//Reduce Timing if need it fast A1
	}
	
	hrtimer_start(&ofm->timer,ofm_polling_time,HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}
//NAGSM_Android_SEL_Kernel_Aakash_20100904 Timer function for polling method implemented V1.2


static ssize_t ojkey_power_read(struct device *dev, struct device_attribute *attr, char *buf)
{
	int count = 0;

	printk("ojkey_power_read --> LDO16 : %d\n", max8998_ldo_is_enabled_direct(MAX8998_LDO16));	

	return count;
}

static ssize_t ojkey_power_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	//buf[size]=0;
	printk("ojkey_power_write --> input data --> %s\n", buf);

	if(strncmp(buf, "1", 1) == 0){
		printk("ojkey_power_write --> turn on LDO16\n");
		Ofm_IOCTL_Power_On();		
	}else if(strncmp(buf, "0", 1) == 0){
		printk("ojkey_power_write --> turn off LDO16\n");
		Ofm_IOCTL_Power_Off();	
	}else{
		printk("ojkey_power_write --> do nothing\n");
	}

	return size;
}
static DEVICE_ATTR(ojkey_power, S_IRUGO | S_IWUSR | S_IWOTH | S_IXOTH, ojkey_power_read, ojkey_power_write);

int ojctl_open (struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t ojctl_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

ssize_t ojctl_write (struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

int ojctl_release (struct inode *inode, struct file *filp)
{
	return 0;
}

int ojctl_ioctl(struct inode *inode, struct file *filp, unsigned int ioctl_num,  unsigned long arg)
{
	int err = 0;
	int i = 0;
	int count = (int)arg;
	
	printk( "ojctl_ioctl : receive cmd = %d, arg = %d\n", ioctl_num, count);

	switch( ioctl_num )
	{
		case IOCTL_OJCTL_POWER_ON :
			{				
				Ofm_IOCTL_Power_On();
			}
			break;
		case IOCTL_OJCTL_POWER_OFF :			
			{
				Ofm_IOCTL_Power_Off();
			}
			break;				
		default:
			{
				printk( "ojctl_ioctl : No case...\n" );
			}
			break;
	}

	return err;

}

struct file_operations ojkey_fops =
{
	.owner   = THIS_MODULE,
	.read    = ojctl_read,
	.write   = ojctl_write,
	.open    =ojctl_open,
	.ioctl   = ojctl_ioctl,
	.release = ojctl_release,
};

static struct miscdevice ojctl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "ojkeyctl",
	.fops = &ojkey_fops,
};

static int Ofm_Check_RevID(void){

	u8 revID1, revID2;
	
	revID1 = i2c_smbus_read_byte_data(gofm->client,0x00);
	printk("%s : revID1 = %x  \n", __func__, revID1);

	if(revID1 == 0x00 ){
		return OFM_PARTRON_DRIVER;
	}else if(revID1 == 0x83 ){
		return OFM_CRUCIAL_DRIVER;
	}else{
		return OFM_CRUCIAL_DRIVER;
	}
}

static int ofm_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ofm	*ofm;
	struct input_dev *input;
	int error;
	ktime_t ofm_polling_time;
	
	struct device *dev_t;
	int result;
	u8 temp;	
	int err_i2c = 0;
	
#if OFM_DEBUG	
	dev_err(&client->dev, "OFM : I2C device probe  \n");
	printk("OFM : I2C device probe  \n");
#endif

	
	ofm = kzalloc(sizeof(struct ofm), GFP_KERNEL);
	if(!ofm){
		dev_err(&client->dev, "OFM : failed to allocate driver data\n");
		error = -ENOMEM;
		goto err0;
	}
	
	i2c_set_clientdata(client, ofm);

	input = input_allocate_device();
	if (!input) {
		dev_err(&client->dev, "OFM : Failed to allocate input device.\n");
		error = -ENOMEM;
		goto err1;
	}
	input->evbit[0] =BIT_MASK(EV_SYN)| BIT_MASK(EV_KEY)|BIT_MASK(EV_REL);
	input->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT);
	input->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
	
	input->name = client->name;
	input->id.bustype = BUS_I2C;
	input->dev.parent = &client->dev;
	input->phys = "ofm";
	input->id.vendor = 0xDEAD;
	input->id.product = 0xBEEF;
	input->id.version = 0x01;
	
	input_set_drvdata(input, ofm);
	
	ofm->client 	= client;
	ofm->input 	= input;
	ofm->ofm_left = &ofm_left;
	ofm->ofm_motion = &ofm_motion;
	ofm->ofm_power_dn = &ofm_power_dn;
	
	ofm->client->flags = I2C_M_IGNORE_NAK;
	
/*
*	pin setting
*/
	gofm = ofm;

	s3c_gpio_cfgpin(ofm->ofm_left->pin, ofm->ofm_left->pin_setting);
     	s3c_gpio_setpull(ofm->ofm_left->pin, S3C_GPIO_PULL_UP);
	
	s3c_gpio_cfgpin(ofm->ofm_power_dn->pin, ofm->ofm_power_dn->pin_setting);
	gpio_set_value(ofm->ofm_power_dn->pin, 0);		
	
	s3c_gpio_cfgpin(GPIO_OJ_NRST, S3C_GPIO_OUTPUT);
	gpio_set_value(GPIO_OJ_NRST, 1);	

	mdelay(100);

	gpio_set_value(GPIO_OJ_NRST, 0);	
	mdelay(50);
	gpio_set_value(GPIO_OJ_NRST, 1);	
	mdelay(100);

	OFM_Driver_Type = Ofm_Check_RevID();
	printk("%s : OFM_Driver_Type = %x\n", __func__, OFM_Driver_Type);
	
	if(OFM_Driver_Type == OFM_PARTRON_DRIVER){
	s3c_gpio_cfgpin(ofm->ofm_motion->pin, ofm->ofm_motion->pin_setting);
	s3c_gpio_setpull(ofm->ofm_motion->pin, S3C_GPIO_PULL_UP);
	}else if(OFM_Driver_Type == OFM_CRUCIAL_DRIVER){
		s3c_gpio_cfgpin(GPIO_OJ_MOTION_C, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(GPIO_OJ_MOTION_C, S3C_GPIO_PULL_UP);
	}
//end	

	error = input_register_device(input);
	if (error)
		goto err1;

	if(OFM_Driver_Type == OFM_CRUCIAL_DRIVER){
		goto skip_i2c_setting;
	}

#if OFM_DEBUG
	temp = i2c_smbus_read_byte_data(ofm->client,0x01);
	printk("temp %x \n",temp);
#endif

	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x05,0x2D);
	if(err_i2c < 0)
	printk("OFM: Probe I2C Error1");

#if OFM_DEBUG
	temp = i2c_smbus_read_byte_data(ofm->client,0x05);
	printk("temp %x \n",temp);
#endif

	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x29,0x04);
	if(err_i2c < 0)
	printk("OFM: Probe I2C Error1");

#if OFM_DEBUG
	temp = i2c_smbus_read_byte_data(ofm->client,0x29);
	printk("temp %x \n",temp);
#endif

	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x27,0x1A);
	if(err_i2c < 0)
	printk("OFM: Probe I2C Error2");

#if OFM_DEBUG
	temp = i2c_smbus_read_byte_data(ofm->client,0x27);
	printk("temp %x \n",temp);
#endif

	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x2A,0x04);
	//	ofm_i2c_write(ofm->client,0x2A,0x08);	original
	if(err_i2c < 0)
	printk("OFM: Probe I2C Error3");

#if OFM_DEBUG
	temp = i2c_smbus_read_byte_data(ofm->client,0x2A);
	printk("temp %x \n",temp);
#endif

	udelay(10);

	err_i2c = ofm_i2c_write(ofm->client,0x2B,0x04);
	//	ofm_i2c_write(ofm->client,0x2B,0x08);	original
	if(err_i2c < 0)
	printk("OFM: Probe I2C Error4");

#if OFM_DEBUG
	temp = i2c_smbus_read_byte_data(ofm->client,0x2B);
	printk("temp %x \n",temp);
#endif

skip_i2c_setting:
	
	mdelay(50);

	ofm_workqueue = create_singlethread_workqueue("ofm_workqueue");
    if (!ofm_workqueue)
	    return -ENOMEM;
    INIT_WORK(&ofm->work, ofm_motion_func);
	
	hrtimer_init(&ofm->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	ofm->timer.function = ofm_timer_func;

	ofm_polling_time = ktime_set(0,0);
	ofm_polling_time = ktime_add_us(ofm_polling_time,100000);	//Decrease timer to smooth transition
    hrtimer_start(&ofm->timer,ofm_polling_time,HRTIMER_MODE_REL);

#if OFM_DEBUG
	printk("Workqueue Settings complete\n");
	printk("OFM Probe Complete\n");

#endif

	result = misc_register(&ojctl_device);

	if (result < 0) 
	{
		return result;
	}

	ojkey_class = class_create (THIS_MODULE, "ojkey");

	if (IS_ERR(ojkey_class)) 
	{
		return PTR_ERR( ojkey_class );
	}
	
	dev_t = device_create(ojkey_class, NULL, MKDEV(OJ_DEV_MAJOR, 0), "%s", "ojkey");

	if (device_create_file(dev_t, &dev_attr_ojkey_power) < 0)
		printk("Failed to create device file(%s)!\n", dev_attr_ojkey_power.attr.name);

	if (IS_ERR(dev_t)) 
	{
		return PTR_ERR(dev_t);
	}

	return 0;		
	
err2:
	input_unregister_device(input);
	input = NULL; /* so we dont try to free it below */
err1:
	input_free_device(input);
	kfree(ofm);
err0:
	dev_set_drvdata(&client->dev, NULL);
	printk("OFM : I2C device probe error (%d) \n",error);
	return error;
}

static int ofm_i2c_remove(struct i2c_client *client)
{
	struct ofm *ofm = i2c_get_clientdata(client);
	destroy_workqueue(ofm_workqueue);
	kfree(ofm);
	
	return 0;
}

//NAGSM_Android_SEL_Kernel_Aakash_20100928
static int ofm_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{

	int err_i2c = 0;
	struct ofm *ofm = i2c_get_clientdata(client);

	printk("OFM : I2C device Suspend\n");

	if(OFM_Driver_Type == OFM_PARTRON_DRIVER){
	//manual mode patch
	err_i2c = ofm_i2c_write(ofm->client,0x0F,0x40);		//Entering Manual Mode //NAGSM_Android_SEL_Kernel_Aakash_20100928
	if(err_i2c < 0)
		printk("OFM: Manual Mode Setting I2C Error");

	err_i2c = ofm_i2c_write(ofm->client,0x0E,0x65);		//Entering Manual Mode //NAGSM_Android_SEL_Kernel_Aakash_20100928
	if(err_i2c < 0)
		printk("OFM: Manual Mode Setting I2C Error");
	
	err_i2c = ofm_i2c_write(ofm->client,0x05,0x2C);		//Entering Manual Mode //NAGSM_Android_SEL_Kernel_Aakash_20100928
	if(err_i2c < 0)
		printk("OFM: Manual Mode Setting I2C Error");

	udelay(200);
	
	gpio_set_value(ofm->ofm_power_dn->pin, 1);
	}else if(OFM_Driver_Type == OFM_CRUCIAL_DRIVER){
		gpio_set_value(ofm->ofm_power_dn->pin, 0);
	}
	
	return 0;
}


static int ofm_i2c_resume(struct i2c_client *client)
{

	int err_i2c = 0;
	struct ofm *ofm = i2c_get_clientdata(client);

	printk("OFM : I2C device Resume\n");	

	if(OFM_Driver_Type == OFM_PARTRON_DRIVER){
	gpio_set_value(ofm->ofm_power_dn->pin, 0);
		msleep(5);
	// initialize regs and set auto mode again 
	ofm_initial_reg(client);
	}else if(OFM_Driver_Type == OFM_CRUCIAL_DRIVER){
		gpio_set_value(ofm->ofm_power_dn->pin, 0);

		s3c_gpio_cfgpin(GPIO_OJ_NRST, S3C_GPIO_OUTPUT);
		gpio_set_value(GPIO_OJ_NRST, 1);	
		msleep(100);
		gpio_set_value(GPIO_OJ_NRST, 0);	
		msleep(10);
		gpio_set_value(GPIO_OJ_NRST, 1);	
		msleep(30);
	}
	
	return 0;
}
//NAGSM_Android_SEL_Kernel_Aakash_20100928



static const struct i2c_device_id ofm_i2c_id[]={
	{"ofm", 0 },	//s3c_device_i2c14 mach-aries
	{}
};

MODULE_DEVICE_TABLE(i2c, ofm_i2c_id);

static struct i2c_driver ofm_i2c_driver = {
	.driver	=	{
		.name	="ofm",
		.owner	=THIS_MODULE,
	},
	.probe	= ofm_i2c_probe,
	.remove	= ofm_i2c_remove,
	.suspend	= ofm_i2c_suspend,	//NAGSM_Android_SEL_Kernel_Aakash_20100928
	.resume		= ofm_i2c_resume,	//NAGSM_Android_SEL_Kernel_Aakash_20100928

	.id_table	=	ofm_i2c_id,
};

//NAGSM_Android_SEL_Kernel_Aakash_20100904 Power Configuration in Version1.2 Untilised Function
static void ofm_ldo_enable(void)
{
	
 	//Turn OFM2.8V on
	Set_MAX8998_PM_OUTPUT_Voltage(LDO9, VCC_2p800);		

	Set_MAX8998_PM_REG(ELDO9, 1);
	max8998_ldo_enable_direct(MAX8998_LDO9);
}

static int __init ofm_init(void)
{
	int ret;

	//ofm_ldo_enable();		//NAGSM_Android_SEL_Kernel_Aakash_20100904

	ret = i2c_add_driver(&ofm_i2c_driver);
	
	if(ret!=0)
	printk("OFM : I2C device init Failed! return(%d) \n",  ret);
	
	return ret;
}
late_initcall(ofm_init);
static void __exit ofm_exit(void)
{
	i2c_del_driver(&ofm_i2c_driver);
}
module_exit(ofm_exit);
MODULE_DESCRIPTION("OFM Device Driver");
MODULE_AUTHOR("Partron Sensor Lab");
MODULE_LICENSE("GPL");

