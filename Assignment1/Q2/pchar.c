#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/kfifo.h>

//number of devices
//#define DEVCNT 4
static int DEVCNT = 1;

module_param(DEVCNT,int,0644);

//size of buffer
#define SIZE 32

//pseudo device --private data structure
typedef struct pchar_device{
    struct kfifo buffer;
    struct cdev cdev;
}pchar_device_t;

//global variables
//memory for keeping devices
//array elements 0,1... corresponds to pchar0,pchar1,..
static pchar_device_t *devices;

static ssize_t pchar_write(struct file *pfile,const char __user *ubuf,size_t bufsize,loff_t *poffset);
static ssize_t pchar_read(struct file *pfile,char __user *ubuf,size_t bufsize,loff_t *poffset);
static int pchar_open(struct inode *pinode,struct file *pfile);
static int pchar_close(struct inode *pinode,struct file *pfile);




static int major;
static struct class *pclass;
static struct file_operations f_ops = {
    .open = pchar_open,
    .release = pchar_close,
    .read = pchar_read,
    .write = pchar_write
};
static int __init pchar_init(void)
{
    int ret,i;
    dev_t devno;
    struct device *pdevice;
    devices = (pchar_device_t*)kmalloc(sizeof(pchar_device_t)*DEVCNT,GFP_KERNEL);
    pr_info("%s:pchar_init() called\n",THIS_MODULE->name);
    //allocate device numbers 
    ret = alloc_chrdev_region(&devno,0,DEVCNT,"pchar");
    if(ret < 0)
    {
        pr_err("%s:alloc_chrdev_region() failed\n",THIS_MODULE->name);
        ret = -1;
        goto alloc_chrdev_region_failed;
    }
    major = MAJOR(devno);
    pr_info("%s:alloc_chrdev_region device numbers %d/%d\n",THIS_MODULE->name,major,MINOR(devno));

    //create device class

    pclass = class_create("pchar_class");
    if(IS_ERR(pclass))
    {
        pr_err("%s:class not created\n",THIS_MODULE->name);
        goto class_create_failed;
    }
    pr_info("%s:class created\n",THIS_MODULE->name);

    //create device file
    for(i=0;i<DEVCNT;i++){
        devno = MKDEV(major,i);
        pdevice = device_create(pclass,NULL,devno,NULL,"pchar%d",i);
        if(IS_ERR(pdevice))
        {
            pr_err("%s:device_create() failed for pchar%d\n",THIS_MODULE->name,i);
            ret = -1;
            goto device_create_failed;
        }
        pr_info("%s:device file created for pchar%d\n",THIS_MODULE->name,i);
    }

    //init cdev struct and add into kernel DB
    for(i = 0;i < DEVCNT;i++){
        cdev_init(&devices[i].cdev,&f_ops);
        devno = MKDEV(major,i);
        ret = cdev_add(&devices[i].cdev,devno,1);
        if(ret < 0){
            pr_err("%s:cdev_add() failed for pchar%d\n",THIS_MODULE->name,i);
            goto cdev_add_failed;
        }
        pr_info("%s:device cdev for pchar%d\n",THIS_MODULE->name,i);
    }

    //allocate device buffer(kfifo)
    for(i = 0;i<DEVCNT;i++){
        // ret = kfifo_alloc(&(*(devices+i)).buffer,SIZE,GFP_KERNEL);
          ret = kfifo_alloc(&devices[i].buffer,SIZE,GFP_KERNEL);
        if(ret){
            pr_err("%s:kfifo_alloc() failed for pchar%d\n",THIS_MODULE->name,i);
            goto kfifo_alloc_failed;
        }
        pr_info("%s:allocated device buffer for pchar%d of size %d\n",THIS_MODULE->name,i,kfifo_size(&devices[i].buffer));
    }
    return 0;


kfifo_alloc_failed:
    for(i=i-1;i>=0;i--){
        kfifo_free(&devices[i].buffer);
    }
        i = DEVCNT;
    
cdev_add_failed:
    for(i=i-1;i>=0;i--){
        cdev_del(&devices[i].cdev);
    }
    i = DEVCNT;
device_create_failed:
    for(i = i-1;i>=0;i--)
    {
        devno = MKDEV(major,i);
        device_destroy(pclass,devno);
    }
    class_destroy(pclass);
class_create_failed:
    unregister_chrdev_region(devno,DEVCNT);
alloc_chrdev_region_failed:
    return ret;
    
}

static void __exit pchar_exit(void)
{
    int ret,i;
    dev_t devno;
    pr_info("%s:pchar_exit() called\n",THIS_MODULE->name);
    //deallocate device buffer(kfifo)
    for(i = DEVCNT-1;i>=0;i--){
        kfifo_free(&devices[i].buffer);
        pr_info("%s:device buffer for pchar%d released\n",THIS_MODULE->name,i);
    }

    //remove device from kernel DB
    for(i = DEVCNT-1;i>=0;i--)
    {
        cdev_del(&devices[i].cdev);
        pr_info("%s:device cdev removed for pchar%d\n",THIS_MODULE->name,i);
    }

    //destroy device file
    for(i = DEVCNT-1;i>=0;i--){
        devno = MKDEV(major,i);
        device_destroy(pclass,devno);
        pr_info("%s:device file is destroyed for pchar%d\n",THIS_MODULE->name,i);
    }
    //class destroy
    class_destroy(pclass);
    pr_info("%s:device class is destroyed\n",THIS_MODULE->name);

    //unallocate device number
    unregister_chrdev_region(devno,DEVCNT);
    pr_info("%s:device number released\n",THIS_MODULE->name);
    
    kfree(devices);

}

//file operations
static int pchar_open(struct inode *pinode,struct file *pfile)
{
    pr_info("%s:pchar_open() called\n",THIS_MODULE->name);
    return 0;
}

static int pchar_close(struct inode *pinode,struct file *pfile)
{
    pr_info("%s:pchar_close() called\n",THIS_MODULE->name);
    return 0;
}

static ssize_t pchar_read(struct file *pfile,char __user *ubuf,size_t bufsize,loff_t *poffset)
{
    int nbytes,ret;
    pchar_device_t *dev = (pchar_device_t *)pfile->private_data;
    pr_info("%s:pchar_read() called\n",THIS_MODULE->name);
    //write data from device buffer to user buffer
    ret = kfifo_to_user(&dev->buffer,ubuf,bufsize,&nbytes);
    if(ret){
        pr_err("%s: failed to copy device buffer to user buffer",THIS_MODULE->name);
        return ret;
    }
    pr_info("%s:pchar_read() %d bytes read\n",THIS_MODULE->name,nbytes);

    return nbytes;
}

static ssize_t pchar_write(struct file *pfile,const char __user *ubuf,size_t bufsize,loff_t *poffset){
    int nbytes,ret;
    pr_info("%s:pchar_write() called\n",THIS_MODULE->name);
    pchar_device_t *dev = (pchar_device_t *)pfile->private_data;
    //write data from user to device buffer
    ret = kfifo_from_user(&dev->buffer,ubuf,bufsize,&nbytes);
    if(ret){
        pr_err("%s:failed to copy user buffer into device\n",THIS_MODULE->name);
        return ret;
    }
    pr_info("%s:pchar_write() %d bytes written\n",THIS_MODULE->name,nbytes);

    return nbytes;
}

module_init(pchar_init);
module_exit(pchar_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satyam <satyam@gmail.com>");
MODULE_DESCRIPTION("Hello char device driver");