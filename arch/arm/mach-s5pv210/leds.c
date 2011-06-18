/*
 * linux/arch/arm/mach-s5pc110/leds.c
 *
 * S5PC100 LEDs dispatcher
 */
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/gpio.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/leds.h>
#include <asm/mach-types.h>

#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio-bank-h0.h>

#include <mach/map.h>
#include <asm/mach/irq.h>
#include "leds.h"

static irqreturn_t eint11_switch(int irq, void *dev_id)
{
	
	printk("EINT11 interrupt occures!!!\n");
	return IRQ_HANDLED;
}


static int __init
s5pv210_leds_init(void)
{
	if (machine_is_smdkc110())
		leds_event = s5pv210_leds_event;
	else
		return -1;

	if (machine_is_smdkc110())
	{
	  gpio_request(S5PV210_GPD0(3), "GPD0");
    gpio_direction_output(S5PV210_GPD0(3), 1);
    if(gpio_get_value(S5PV210_GPD0(3)) == 0)
    {
        printk(KERN_WARNING "LED: can't set GPD0(3) to output mode\n");
    }

    gpio_request(S5PV210_GPD0(2), "GPD0");
    gpio_direction_output(S5PV210_GPD0(2), 1);
    if(gpio_get_value(S5PV210_GPD0(2)) == 0)
    {
        printk(KERN_WARNING "LED: can't set GPD0(2) to output mode\n");
    }

    gpio_request(S5PV210_GPB(6), "GPB");
    gpio_direction_output(S5PV210_GPB(6), 1);
    if(gpio_get_value(S5PV210_GPB(6)) == 0)
    {
        printk(KERN_WARNING "LED: can't set GPB(6) to output mode\n");
    }

    gpio_request(S5PV210_GPA0(6), "GPA0");
    gpio_direction_output(S5PV210_GPA0(6), 1);
    if(gpio_get_value(S5PV210_GPA0(6)) == 0)
    {
        printk(KERN_WARNING "LED: can't set GPA0(6) to output mode\n");
    }
	}

	/* Get irqs */ /*
	set_irq_type(IRQ_EINT11, IRQ_TYPE_EDGE_FALLING);
	s3c_gpio_setpull(S5PC11X_GPH1(3), S3C_GPIO_PULL_NONE);
        if (request_irq(IRQ_EINT11, eint11_switch, IRQF_DISABLED, "EINT11", NULL)) {
                return -EIO;
        }
  */
	leds_event(led_start);
	return 0;
}

__initcall(s5pv210_leds_init);
