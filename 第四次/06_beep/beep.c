/* Chapter 46: device-tree GPIO character driver, Linux 4.1.15. */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/uaccess.h>

static dev_t beep_id;
static struct cdev beep_cdev;
static struct class *beep_class;
static int beep_gpio;
static struct platform_device *beep_platform;
static struct pinctrl *beep_pinctrl;

static ssize_t beep_write(struct file *file, const char __user *buf,
                          size_t count, loff_t *offset)
{
    unsigned char command;

    if (!count)
        return 0;
    if (count != 1)
        return -EINVAL;
    if (copy_from_user(&command, buf, 1))
        return -EFAULT;
    if (command > 1)
        return -EINVAL;
    gpio_set_value(beep_gpio, command ? 0 : 1);
    pr_info("beep: %s\n", command ? "ON" : "OFF");
    return 1;
}

static const struct file_operations beep_fops = {
    .owner = THIS_MODULE,
    .write = beep_write,
    .llseek = no_llseek,
};

static int __init beep_init(void)
{
    struct device_node *node;
    struct device *device;
    int ret;

    node = of_find_node_by_path("/beep");
    if (!node)
        return -ENODEV;
    beep_gpio = of_get_named_gpio(node, "beep-gpio", 0);
    beep_platform = of_find_device_by_node(node);
    of_node_put(node);
    if (!beep_platform)
        return -ENODEV;
    if (beep_gpio < 0) {
        ret = beep_gpio;
        goto put_platform;
    }
    /* This lesson uses module_init rather than a platform probe. Select
     * the pinctrl state explicitly on the factory device tree. */
    beep_pinctrl = pinctrl_get_select_default(&beep_platform->dev);
    if (IS_ERR(beep_pinctrl)) {
        ret = PTR_ERR(beep_pinctrl);
        goto put_platform;
    }
    ret = gpio_request(beep_gpio, "lab4-beep");
    if (ret)
        goto put_pinctrl;
    ret = gpio_direction_output(beep_gpio, 1);
    if (ret)
        goto free_gpio;
    ret = alloc_chrdev_region(&beep_id, 0, 1, "beep");
    if (ret)
        goto free_gpio;
    cdev_init(&beep_cdev, &beep_fops);
    beep_cdev.owner = THIS_MODULE;
    ret = cdev_add(&beep_cdev, beep_id, 1);
    if (ret)
        goto free_id;
    beep_class = class_create(THIS_MODULE, "beep");
    if (IS_ERR(beep_class)) {
        ret = PTR_ERR(beep_class);
        goto free_cdev;
    }
    device = device_create(beep_class, NULL, beep_id, NULL, "beep");
    if (IS_ERR(device)) {
        ret = PTR_ERR(device);
        goto free_class;
    }
    pr_info("beep: /dev/beep ready, gpio=%d, major=%u, default OFF\n",
            beep_gpio, MAJOR(beep_id));
    return 0;

free_class:
    class_destroy(beep_class);
free_cdev:
    cdev_del(&beep_cdev);
free_id:
    unregister_chrdev_region(beep_id, 1);
free_gpio:
    gpio_free(beep_gpio);
put_pinctrl:
    pinctrl_put(beep_pinctrl);
put_platform:
    put_device(&beep_platform->dev);
    return ret;
}

static void __exit beep_exit(void)
{
    gpio_set_value(beep_gpio, 1);
    device_destroy(beep_class, beep_id);
    class_destroy(beep_class);
    cdev_del(&beep_cdev);
    unregister_chrdev_region(beep_id, 1);
    gpio_free(beep_gpio);
    pinctrl_put(beep_pinctrl);
    put_device(&beep_platform->dev);
    pr_info("beep: unloaded, OFF\n");
}

module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Chapter 46 device-tree GPIO beep character driver");
