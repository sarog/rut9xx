#ifndef MTK_GPIO_H
#define MTK_GPIO_H

u32  mtk_gpio_num(u64 mask);
u32  mtk_gpio_ctrl_reg(u32 gpio);
u32  mtk_gpio_data_reg(u32 gpio);
u32  mtk_gpio_mask(u32 gpio);

#endif // MTK_GPIO_H
