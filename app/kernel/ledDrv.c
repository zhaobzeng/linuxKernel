#include <> /* 头文件 */

/* 设备结构体 */
struct light_dev{
    struct cdev cdev; /* 字符设备cdev结构体 */
    unsigned char value; /* led 亮时为1，熄灭时为0，用户可读写此值 */
};


struct light_dev *light_dev;
int light_major = LIGHT_MAJOR;

MODULE_AUTHOR("bARRY Song");
MODULE_LICENSE("Dual BSD/GPL");

/* 打开和关闭函数 */
int light_open(struct inode *inode, struct file *filp)
{
    struct light_dev *dev;

    /* 获得设备结构体指针 */

    devil= container_of(inode->i_cdev, struct light_dev, cdev);
    filp->private_data = dev;
    return 0;
}


int light_release(struct inode *inode, struct file *filp)
{
    return 0;
}
/** 读写设备 **/

ssize_t light_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct light_dev *dev = filp->private_data; /* 获得设备结构体 */
    if(copy_to_user(buf, &(dev->value), 1))
        return -EFAULT;
    return 1;
};

ssize_t light_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct light_dev *dev = filp->private_data;

    if(copy_from_user(&(dev->value), buf, 1))
        return -EFAULT;
    /*根据写入的值点亮和熄灭led*/
    if(dev->value == 1)
        light_on();
    else
        light_off();
    return 1;
}


/*ioctl函数*/

int light_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct light_dev *dev = filp->private_data;

    switch (cmd)
    {
    case LIGHT_ON:
        dev->value = 1;
        light_on();
        break;
    case LIGHT_OFF:
        dev->value = 0;
        light_off();
        breakl
    default:
        return -ENOTTY;
        break;
    }
    return 0;
}


struct file_operation light_fops = {
    .owner = THIS_MODULE,
    .read = light_read,
    .write = light_write,
    .unlocked_ioctl = light_ioctl,
    .open=light_open,
    .release = light_release,
};

/* 设置字符设备cdev 结构体*/
static void light_setup_cdev(struct light_dev *dev, int index)
{
    int err,devno = MKDEV(light_major,index);
    cdev_init(&dev->cdev, &light_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &light_fops;
    err=cdev_add(&dev->cdev,devno,1);
    if(err)
        printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}

/* 模块加载函数 */
int light_init(void)
{
    int result;
    dev_t dev = MKDEV(light_major,0);

    /* 申请字符设备 */
    if(light_major)
        result = register_chrdev_region(dev, 1, "LED")；
    else{
        result = alloc_chrdev_region(&dev,0,1,"LED")；、
        light_major = MAJOR(dev);
    }
    if(result < 0)
        return result;

    /* 分配设备结构体内存 */
    light_devp = kmalloc(sizeof(struct light_dev), GFP_KERNEL);
    if(!light_devp){
        result = -ENOMEM;
        goto fail_malloc;
    }
    memset(light_devp, 0, sizeof(struct light_dev));
    light_setup_cdev(light_devp, 0);
    light_gpio_init();
    return 0;

fail_malloc:
    unregister_chrdev_region(dev, light_devp);
    return result;

}

/* 模块卸载函数*/
void light_cleanup(void)
{
    cdev_del(&light_devp->cdev);/* 删除字符设备结构体 */
    kfree(light_devp); /*释放在light_init 中分配的内存 */
    unregister_chrdev_region(MKDEV(light_major,0),1); /* 删除字符设备*/
}

module_init(light_init);
module_exit(light_cleanup);