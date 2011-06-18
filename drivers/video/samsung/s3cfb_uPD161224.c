#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/lcd.h>
#include <linux/backlight.h>

#include <plat/gpio-cfg.h>

#if defined(CONFIG_LCD_NEC_uPD161224) 
#include <linux/pwm.h>
#include <linux/regulator/max8998.h>
#include <mach/max8998_function.h>
#endif

#include "s3cfb.h"

#define DIM_BL	20
#define MIN_BL	30
#define MAX_BL	255

#define CRITICAL_BATTERY_LEVEL 5

#define LCD_TUNNING_VALUE 1 // NAGSM_Android_HQ_KERNEL_CLee

#if defined (LCD_TUNNING_VALUE)
// Values coming from platform
#define MAX_BRIGHTNESS_LEVEL 255
#define LOW_BRIGHTNESS_LEVEL 30
#define DIM_BACKLIGHT_LEVEL 20	
// Values that kernel ernel tries to set.
#define MAX_BACKLIGHT_VALUE 160
#define LOW_BACKLIGHT_VALUE 7 
#define DIM_BACKLIGHT_VALUE 7 

static int s5p_bl_convert_to_tuned_value(int intensity);
#endif

#define BACKLIGHT_SYSFS_INTERFACE 1 // NAGSM_Android_SEL_KERNEL_Subhransu

#if defined(BACKLIGHT_SYSFS_INTERFACE)
	struct class *ldi_class;
	struct device *ldi_dev;
	static int max_brightness = 0;
	static int saved_brightness = 0;
#endif

/*********** for debug **********************************************************/
#if 0
#define gprintk(fmt, x... ) printk( "%s(%d): " fmt, __FUNCTION__ ,__LINE__, ## x)
#else
#define gprintk(x...) do { } while (0)
#endif
/*******************************************************************************/

static int ldi_enable = 0;

#if defined(CONFIG_LCD_NEC_uPD161224) 
struct pwm_device	*backlight_pwm_dev;
long int bl_freq_count = 100000;
#endif

typedef enum {
	BACKLIGHT_LEVEL_OFF		= 0,
	BACKLIGHT_LEVEL_DIMMING	= 1,
	BACKLIGHT_LEVEL_NORMAL	= 6
} backlight_level_t;

backlight_level_t backlight_level = BACKLIGHT_LEVEL_OFF;
static int bd_brightness = 0;

static DEFINE_MUTEX(spi_use);

struct s5p_lcd {
	struct spi_device *g_spi;
	struct lcd_device *lcd_dev;
	struct backlight_device *bl_dev;
};

static struct s5p_lcd lcd;

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00

const unsigned short SEQ_PANEL_LCD_SET_MANUFACTURE_COMMAND_CODE[] = { 
	0xB2, 0x00,
	0x53, 0x00,
#if defined(CONFIG_SIDEKICK_VER_B2)
	0x54, 0x03,
#else
	0x54, 0x00,
#endif
	0x57, 0x00,
	0x62, 0x05,
	0x67, 0x70, 	/* AVDD  */
	0x68, 0x23,		/* Vcs */
//	0x69, 0x2A,		/* Vcom */
	0x6A, 0x03,
	0x6B, 0x01,
	0x6C, 0x0F,
	0x6D, 0x0F,
	0x6F, 0x0B,
	0x77, 0x53,
	0x78, 0x00,
	0x79, 0x48,
	/* Start - RGB */
	/* Red */
	0x7A, 0x00,
	0x7B, 0xFF,
	0x7C, 0x14,
	0x7D, 0x2E,
	0x7E, 0x18,
	0x7F, 0xF0,
	/* Green */
	0x80, 0x00,
	0x81, 0xFF,
	0x82, 0xF5,
	0x83, 0x0B,
	0x84, 0xEB,
	0x85, 0xF0,
	/* Blue */
	0x86, 0x00,
	0x87, 0xFF,
	0x88, 0xB9,
	0x89, 0xCD,
	0x8A, 0x89,
	0x8B, 0x10,
	/* End - RGB */
	0x9F, 0x03,
	0xA0, 0x20,
	0xB3, 0x00,
	0xB4, 0x52,
	0xC7, 0x00,
	0xC8, 0x01,
	0xC9, 0x03,
	0xCA, 0x20,
	0xB2, 0x03,

	ENDDEF, 0x0000	
};

static struct s3cfb_lcd s6e63m0 = {
	.width = 480,
	.height = 800,
	.p_width = 52,
	.p_height = 86,
	.bpp = 24,
	.freq = 60,
	
  .timing = {
    .h_fp = 82, 
    .h_bp = 2, 
    .h_sw = 4,  
    .v_fp = 5,  
    .v_fpe = 1,
    .v_bp = 1,
    .v_bpe = 1,
    .v_sw = 2,
    },

	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static void wait_ldi_enable(void);
static void update_brightness(int gamma);

static int uPD161224_spi_write_driver(unsigned char address, unsigned short command)
{
	int ret;
  u16 buf[1]={0, };
  struct spi_message msg;

  struct spi_transfer xfer = {
    .len    = 2,
    .tx_buf = buf,
  };

  buf[0] = (address << 8) | command;
  //gprintk("REG=[%d](0x%x)\n", buf[0], buf[0]); 

  spi_message_init(&msg);
  spi_message_add_tail(&xfer, &msg);
  
	if(!(lcd.g_spi))
	{
	  gprintk("***** !(lcd.g_spi) *****\n");
	  return -EINVAL;
  }	      	

  ret = spi_sync(lcd.g_spi, &msg);
  if (ret < 0)
  {
		pr_err("%s::%d -> spi_sync failed Err=%d\n",__func__,__LINE__,ret);
	}
		
	return ret ;
}

int IsLDIEnabled(void)
{
	return ldi_enable;
}
EXPORT_SYMBOL(IsLDIEnabled);

#if defined(BACKLIGHT_SYSFS_INTERFACE)
static ssize_t update_brightness_cmd_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf,"%d\n", saved_brightness);
}
static ssize_t upadate_brightness_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
        int brightness = 0;
        sscanf(buf, "%d", &brightness);
		
        if(!IsLDIEnabled())
        {
                printk(KERN_ERR "LDI not enabled");
                return 0;
        }

        /* Sanity checking if brightness value is -ve by mistake, then set to minimum value*/
        /*                 brightness value > max value, set to max value */
        if(brightness < 0)
                brightness = 0;

        if(brightness > max_brightness)
                brightness = max_brightness;
        update_brightness(brightness);
		
		saved_brightness = brightness;
        return 0;
}
static DEVICE_ATTR(update_brightness_cmd,0666, update_brightness_cmd_show, upadate_brightness_cmd_store);
#endif

static void uPD161224_spi_write(unsigned char address, unsigned short command)
{
  uPD161224_spi_write_driver(0x0, address);
  uPD161224_spi_write_driver(0x1, command);
}

static void uPD161224_panel_send_sequence(const unsigned short *wbuf)
{
  int i = 0;

  gprintk("#################SPI start##########################\n");
  while ((wbuf[i] & DEFMASK) != ENDDEF) {
    if ((wbuf[i] & DEFMASK) != SLEEPMSEC)
      uPD161224_spi_write(wbuf[i], wbuf[i+1]);
    else
      mdelay(wbuf[i+1]);
    i += 2;
  }
  gprintk("#################SPI enb##########################\n");
}

static void SetLDIEnabledFlag(int OnOff)
{
	ldi_enable = OnOff;
}

static void uPD161224_ldi_sleep()
{
  /* SDX Signal "H" -> "L" */
  gprintk("gpio_get_value(GPIO_DISPLAY_SDX)=[%d]\n", gpio_get_value(GPIO_DISPLAY_SDX));
  //gpio_direction_output(GPIO_DISPLAY_SDX, 1);
  gpio_set_value(GPIO_DISPLAY_SDX, 0); 
  gprintk("gpio_get_value(GPIO_DISPLAY_SDX)=[%d]\n", gpio_get_value(GPIO_DISPLAY_SDX));
  /* Wait 85ms(min) == (Wait 5 frame(min)) */
  msleep(100);
  /* RGB signals OFF */
  /* uPD161224_ldi_standby_off */
}

static void uPD161224_ldi_wakeup()
{
  /* uPD161224_ldi_standby_on */
  /* SDX Signal "L" -> "H" */
  gprintk("gpio_get_value(GPIO_DISPLAY_SDX)=[%d]\n", gpio_get_value(GPIO_DISPLAY_SDX));
  //gpio_direction_output(GPIO_DISPLAY_SDX, 1);
  gpio_set_value(GPIO_DISPLAY_SDX, 1);   
  gprintk("gpio_get_value(GPIO_DISPLAY_SDX)=[%d]\n", gpio_get_value(GPIO_DISPLAY_SDX));
  /* Wait 4us(min) */
  msleep(1);  
  /* Input RGB signals (RGB I/F)   */
  /* Wait 17ms(typ) == (Wait 1 frame(typ)) */
  msleep(20); 
  /* LDI Initial CODE (Serial I/F) */
  uPD161224_panel_send_sequence(SEQ_PANEL_LCD_SET_MANUFACTURE_COMMAND_CODE);
  /* Wait 85ms(min) == (Wait 5 frame(min)) */

#if defined(CONFIG_LCD_NEC_uPD161224)   
  msleep(140); 
#else
  msleep(100); 
#endif
}

static void uPD161224_ldi_standby_on()
{
  /* Power on */
  /* Wait 30us(min) */
  msleep(1); 
  /* Hard RESET "L" -> "H" */
  gprintk("gpio_get_value(GPIO_MLCD_RST)=[%d]\n", gpio_get_value(GPIO_MLCD_RST));
  //gpio_direction_output(GPIO_MLCD_RST, 1);
  gpio_set_value(GPIO_MLCD_RST, 1);
  gprintk("gpio_get_value(GPIO_MLCD_RST)=[%d]\n", gpio_get_value(GPIO_MLCD_RST));
  /* Wait 3ms(min) */
  	
#if defined(CONFIG_LCD_NEC_uPD161224)   
  msleep(5); 
#else 	
  msleep(10); 
#endif
}

static void uPD161224_ldi_standby_off()
{
  /* Hard RESET "H" -> "L"  */
  gprintk("gpio_get_value(GPIO_MLCD_RST)=[%d]\n", gpio_get_value(GPIO_MLCD_RST));
  //gpio_direction_output(GPIO_MLCD_RST, 1);
  gpio_set_value(GPIO_MLCD_RST, 0);
  gprintk("gpio_get_value(GPIO_MLCD_RST)=[%d]\n", gpio_get_value(GPIO_MLCD_RST));
  /* Wait 30us(min) */
  msleep(10); 
  /* Power off */
}

void uPD161224_ldi_init(void)
{
#if defined(CONFIG_LCD_NEC_uPD161224) 
	int ret_val_pwm_config1 = 0;
	int ret_val_pwm_config2 = 0;
#endif
  uPD161224_ldi_standby_on();
  uPD161224_ldi_wakeup();

	SetLDIEnabledFlag(1);	
	printk(KERN_DEBUG "LDI enable ok\n");
	pr_info("%s::%d -> ldi initialized\n",__func__,__LINE__);	
}

void uPD161224_ldi_enable(void)
{
  return;
}

void uPD161224_ldi_disable(void)
{
  uPD161224_ldi_sleep();
  uPD161224_ldi_standby_off();

	SetLDIEnabledFlag(0);
	printk(KERN_DEBUG "LDI disable ok\n");
	pr_info("%s::%d -> ldi disabled\n",__func__,__LINE__);	
}

void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
	s6e63m0.init_ldi = NULL;
	ctrl->lcd = &s6e63m0;
}

//mkh:lcd operations and functions
static int s5p_lcd_set_power(struct lcd_device *ld, int power)
{
	printk(KERN_DEBUG "s5p_lcd_set_power is called: %d", power);

	return 0;
}

static struct lcd_ops s5p_lcd_ops = {
	.set_power = s5p_lcd_set_power,
};

//mkh:backlight operations and functions
static void wait_ldi_enable(void)
{
	int i = 0;

	for (i = 0; i < 100; i++)	{
		gprintk("ldi_enable : %d \n", ldi_enable);

		if(IsLDIEnabled())
			break;
		
		msleep(10);
	};
}

#if defined (LCD_TUNNING_VALUE)
static int s5p_bl_convert_to_tuned_value(int intensity)
{
	int tune_value;

	if(intensity >= LOW_BRIGHTNESS_LEVEL)
	{
		tune_value = (intensity - LOW_BRIGHTNESS_LEVEL) * (MAX_BACKLIGHT_VALUE-LOW_BACKLIGHT_VALUE) / (MAX_BRIGHTNESS_LEVEL-LOW_BRIGHTNESS_LEVEL) + LOW_BACKLIGHT_VALUE;
	}
	else if(intensity >= DIM_BACKLIGHT_LEVEL)
	{
		tune_value = (intensity - DIM_BACKLIGHT_LEVEL) * (LOW_BACKLIGHT_VALUE-DIM_BACKLIGHT_VALUE) / (LOW_BRIGHTNESS_LEVEL-DIM_BACKLIGHT_LEVEL) + DIM_BACKLIGHT_VALUE;
	}	
	else if(intensity > 0)
	{
		tune_value = (intensity) * (DIM_BACKLIGHT_VALUE) / (DIM_BACKLIGHT_LEVEL);
	}
	else
	{
		tune_value = intensity;
	}

	gprintk("Passed value : [%d], Converted value : [%d]\n", intensity, tune_value);

	return tune_value;
}
#endif

void off_display(void)
{
#if defined(CONFIG_LCD_NEC_uPD161224) 
	pwm_disable(backlight_pwm_dev);
#endif
	msleep(20);
	bd_brightness = 0;
	backlight_level = BACKLIGHT_LEVEL_OFF;
}

static s32 s5p_bl_set_power(int power)
{
	if(power)
	{ 
  	gpio_set_value(GPIO_LCD_BL_EN, GPIO_LEVEL_HIGH);
	}
	else
	{
	  gpio_set_value(GPIO_LCD_BL_EN, GPIO_LEVEL_LOW);
  }
  
	return 0;
}

static void update_brightness(int brightness)
{ 
  int brightness_pulse;

#if defined (CONFIG_LCD_NEC_uPD161224)
#if defined (LCD_TUNNING_VALUE)
  int tunned_brightness = 0;
  tunned_brightness = s5p_bl_convert_to_tuned_value(brightness);
  pwm_config(backlight_pwm_dev, (bl_freq_count * tunned_brightness)/MAX_BL, bl_freq_count);
  pwm_enable(backlight_pwm_dev);
#else
  pwm_config(backlight_pwm_dev, (bl_freq_count * brightness)/255, bl_freq_count);
  pwm_enable(backlight_pwm_dev);
  // gprintk("## brightness = [%ld], (bl_freq_count * brightness)/255 =[%ld], ret_val_pwm_config=[%ld] \n", brightness, (bl_freq_count * brightness)/255, ret_val_pwm_config );
#endif
#endif
}

static int s5p_bl_update_status(struct backlight_device* bd)
{
	int bl = bd->props.brightness;
	backlight_level_t level = BACKLIGHT_LEVEL_OFF;

#if defined(CONFIG_LCD_NEC_uPD161224)
	wait_ldi_enable();
#endif

	gprintk("\nupdate status brightness[0~255] : (%d) \n",bd->props.brightness);
	
	if(bl == 0) 
	{
		level = BACKLIGHT_LEVEL_OFF;	//lcd off
	}
	else if((bl < MIN_BL) && (bl > 0))
	{
		level = BACKLIGHT_LEVEL_DIMMING;	//dimming
	}
	else
	{
		level = BACKLIGHT_LEVEL_NORMAL;	//normal
	}
		
	bd_brightness = bd->props.brightness;
	backlight_level = level;

	switch (level)	{
	  case BACKLIGHT_LEVEL_OFF:
	    off_display();
	    gprintk("[OFF] s5p_bl_update_status is called. level : (%d), brightness[0~255] : (%d)\n",level, bl);
	    break;
		case BACKLIGHT_LEVEL_DIMMING:
			update_brightness(bl);
			printk("[DIMMING] s5p_bl_update_status is called. level : (%d), brightness[0~255] : (%d)\n",level, bl);
			break;
		case BACKLIGHT_LEVEL_NORMAL:
			update_brightness(bl);
			gprintk("[NORMAL] s5p_bl_update_status is called. level : (%d), brightness[0~255] : (%d)\n",level, bl);
			break;
		default:
			break;
	}

	return 0;
}

static int s5p_bl_get_brightness(struct backlight_device* bd)
{
	printk(KERN_DEBUG "\n reading brightness \n");
	gprintk("\n5p_bl_get_brightness is called. bd_brightness [0~255] : (%d) \n",bd->props.brightness);
	return bd_brightness;
}

static struct backlight_ops s5p_bl_ops = {
	.update_status = s5p_bl_update_status,
	.get_brightness = s5p_bl_get_brightness,	
};

static int __init uPD161224_probe(struct spi_device *spi)
{
	int ret;

	spi->bits_per_word = 16;
	ret = spi_setup(spi);
	lcd.g_spi = spi;
	lcd.lcd_dev = lcd_device_register("s5p_lcd",&spi->dev,&lcd,&s5p_lcd_ops);
	lcd.bl_dev = backlight_device_register("s5p_bl",&spi->dev,&lcd,&s5p_bl_ops);
	lcd.bl_dev->props.max_brightness = 255;
	dev_set_drvdata(&spi->dev,&lcd);

#if defined(CONFIG_LCD_NEC_uPD161224) 
	backlight_pwm_dev = pwm_request(0, "Hawk_PWM");
	pwm_config(backlight_pwm_dev, (bl_freq_count*70)/100, bl_freq_count);	
	pwm_enable(backlight_pwm_dev);		
#endif

	SetLDIEnabledFlag(1);

#if defined(BACKLIGHT_SYSFS_INTERFACE)
	max_brightness = lcd.bl_dev->props.max_brightness;	//NAGSM_Android_SEL_KERNEL_Subhransu_
	
	// Class and device file creation 
	printk(KERN_ERR "ldi_class create\n");
	ldi_class = class_create(THIS_MODULE, "ldi_class");
	if (IS_ERR(ldi_class))
		pr_err("Failed to create class(ldi_class)!\n");

	ldi_dev = device_create(ldi_class, NULL, 0, NULL, "ldi_dev");
	if (IS_ERR(ldi_dev))
		pr_err("Failed to create device(ldi_dev)!\n");

	if (device_create_file(ldi_dev, &dev_attr_update_brightness_cmd) < 0)
		pr_err("Failed to create device file(%s)!\n", dev_attr_update_brightness_cmd.attr.name);
#endif

	if (ret < 0)	
	{
		pr_err("%s::%d-> s6e63m0 probe failed Err=%d\n",__func__,__LINE__,ret);
		return 0;
	}
	
	pr_info("%s::%d->s6e63m0 probed successfuly\n",__func__,__LINE__);
	
	return ret;
}

static struct spi_driver uPD161224_driver = {
	.driver = {
		.name	= "uPD161224",
		.owner	= THIS_MODULE,
	},
	.probe		= uPD161224_probe,
	.remove		= __exit_p(uPD161224_remove),
	.suspend	= NULL,
	.resume		= NULL,
};

static int __init uPD161224_init(void)
{
	return spi_register_driver(&uPD161224_driver);
}

static void __exit uPD161224_exit(void)
{
	spi_unregister_driver(&uPD161224_driver);
}


module_init(uPD161224_init);
module_exit(uPD161224_exit);


MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("uPD161224 LDI driver");
MODULE_LICENSE("GPL");
