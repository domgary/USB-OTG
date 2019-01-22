#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/usbdevice_fs.h>
#include <linux/usb/ch9.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define USB_DEV "/proc/bus/usb/001/002"

int main()
{
    int fd, ret, err=-1;
    struct usbdevfs_connectinfo connectinfo;
    struct usbdevfs_bulktransfer transfert;
    uint32_t val;
    char buffer[512];

    printf("Build %s @ %s\n", __DATE__, __TIME__);

    fd = open(USB_DEV, O_RDWR);

    if (fd <= 0)
    {
        printf("Unable to open %s (%m)\n", USB_DEV);
        return 1;
    }

    printf("Device opened\n");

    // Optional get information
    ret = ioctl(fd, USBDEVFS_CONNECTINFO, &connectinfo);
    if (ret)
    {
        printf("USBDEVFS_CONNECTINFO error %d (%m)\n", ret);
        goto end;
    }

    printf("devnum %d, slow %d\n",
           connectinfo.devnum, connectinfo.slow);

    // Claim interface 0
    val = 0;
    ret = ioctl(fd, USBDEVFS_CLAIMINTERFACE, &val);
    if (ret)
    {
        printf("USBDEVFS_CLAIMINTERFACE error %d (%m)\n", ret);
        goto end;
    }
    else
        printf("Interface claimed\n");

    // Send data on ep2out
    strcpy(buffer, "What is your name ?");

    transfert.ep = USB_DIR_OUT + 2;
    transfert.len = strlen(buffer)+1;
    transfert.timeout = 200;
    transfert.data = buffer;

    ret = ioctl(fd, USBDEVFS_BULK, &transfert);
    if (ret < 0)
    {
        printf("USBDEVFS_BULK 1 error %d (%m)\n", ret);
        goto end;
    }
    else
        printf("Transfert 1 OK %d\n", ret);

    // Receive data on ep1in
    transfert.ep = USB_DIR_IN + 1;
    transfert.len = sizeof(buffer);
    transfert.timeout = 200;
    transfert.data = buffer;

    ret = ioctl(fd, USBDEVFS_BULK, &transfert);
    if (ret < 0)
    {
        printf("USBDEVFS_BULK 2 error %d (%m)\n", ret);
        goto end;
    }
    else
        printf("Transfert 2 OK %d %s\n", ret, buffer);

    // Release interface 0
    val = 0;
    ret = ioctl(fd, USBDEVFS_RELEASEINTERFACE, &val);
    if (ret)
    {
        printf("USBDEVFS_RELEASEINTERFACE error %d (%m)\n", ret);
        goto end;
    }

    printf("Interface released\n");

    err = 0;

end:
    close(fd);

    return err;
}
