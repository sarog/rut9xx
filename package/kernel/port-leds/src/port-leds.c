#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include<linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include <linux/gpio.h>
// #include <arch/mips/ath79/common.h>
#include <linux/types.h>
#include <linux/init.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#define RUT900_GPIO_LED_LAN1        14
#define RUT900_GPIO_LED_LAN2        13
#define RUT900_GPIO_LED_LAN3        22
#define RUT900_GPIO_LED_WAN        1

int len,temp;

char *msg;
 static DEFINE_SPINLOCK(ath79_gpio_lock);


void mano_ath79_gpio_output_select(unsigned gpio, u8 val)
{
	void __iomem *base;
	unsigned long flags;
	unsigned int reg;
	u32 t, s;
	void __iomem *ath79_gpio_base;
	
	ath79_gpio_base = ioremap_nocache(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);
	base = ath79_gpio_base;

	if (gpio >= AR934X_GPIO_COUNT)
		return;

	reg = AR934X_GPIO_REG_OUT_FUNC0 + 4 * (gpio / 4);
	s = 8 * (gpio % 4);

 	spin_lock_irqsave(&ath79_gpio_lock, flags);

	t = __raw_readl(base + reg);
 	t &= ~(0xff << s);
 	t |= val << s;
  	__raw_writel(t, base + reg);

	/* flush write */
 	(void) __raw_readl(base + reg);

  	spin_unlock_irqrestore(&ath79_gpio_lock, flags);
}


void mano_ath79_gpio_set(unsigned gpio)
{
	void __iomem *base;
	unsigned long flags;
	u32 t;
	void __iomem *ath79_gpio_base;
	
	ath79_gpio_base = ioremap_nocache(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);
	base = ath79_gpio_base;

	if (gpio >= AR934X_GPIO_COUNT)
		return;

 	spin_lock_irqsave(&ath79_gpio_lock, flags);


 	t = 1 << gpio;
  	__raw_writel(t, base +  AR71XX_GPIO_REG_SET);

	/* flush write */
 	(void) __raw_readl(base +  AR71XX_GPIO_REG_SET);

  	spin_unlock_irqrestore(&ath79_gpio_lock, flags);
}

int read_proc(struct file *filp,char *buf,size_t count,loff_t *offp ) 
{
	if(count>temp)
	{
		count=temp;
	}
	temp=temp-count;
	copy_to_user(buf,msg, count);
	if(count==0)
		temp=len;

	return count;
}

int write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{

	copy_from_user(msg,buf,count);
	len=count;
	temp=len;

	if(msg[0] == '0'){
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_LAN1, 0);
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_LAN2, 0);
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_LAN3, 0);
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_WAN, 0);
		mano_ath79_gpio_set(RUT900_GPIO_LED_LAN1);
		mano_ath79_gpio_set(RUT900_GPIO_LED_LAN2);
		mano_ath79_gpio_set(RUT900_GPIO_LED_LAN3);
		mano_ath79_gpio_set(RUT900_GPIO_LED_WAN);
		printk(KERN_INFO "Ports leds OFF \n");
		
	}else if(msg[0] == '1'){
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_LAN1, AR934X_GPIO_OUT_LED_LINK3);
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_LAN2, AR934X_GPIO_OUT_LED_LINK2);
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_LAN3, AR934X_GPIO_OUT_LED_LINK1);
		mano_ath79_gpio_output_select(RUT900_GPIO_LED_WAN, AR934X_GPIO_OUT_LED_LINK4);
		printk(KERN_INFO "Ports leds ON \n");
	}
	return count;
}

struct file_operations proc_fops = {
	read: read_proc,
	write: write_proc
};

void create_new_proc_entry(void){
	proc_create("port-leds",0,NULL,&proc_fops);
	msg=kmalloc(GFP_KERNEL,10*sizeof(char));
}


int proc_init (void) {
	create_new_proc_entry();
	return 0;
}

void proc_cleanup(void) {
	remove_proc_entry("port-leds",NULL);
	printk(KERN_INFO "Module deleted\n");
}

MODULE_LICENSE("GPL"); 
module_init(proc_init);
module_exit(proc_cleanup);
