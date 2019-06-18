#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/videodev2.h>
#include <string.h>

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

int main(int argc, char *argv[]) {
    int fd = 0;
    int ret = 0;
    struct v4l2_capability caps;
    struct v4l2_fmtdesc fmtdesc;
    // struct v4l2_crop crop;
    struct v4l2_format fmt;
    // struct v4l2_streamparm parm;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    struct v4l2_frmsizeenum frmsize;

    if (argc != 2) {
        fprintf(stderr, "usage: v4l2_cam device\n");
        return -1;
    }

    /* open video device */
    fd = open(argv[1], O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "open device %s failed, error: %d(%m)",
                            argv[1], errno);
        return -1;
    }

    /* query device capabilities */
    ret = ioctl(fd, VIDIOC_QUERYCAP, &caps);
    if (ret < 0) {
        fprintf(stderr, "query caps failed, error: %d(%m)\n", errno);
        return -1;
    }

    printf("video device capabilities:\n");
    printf("driver: %s\n", caps.driver);
    printf("card: %s\n", caps.card);
    printf("bus_info: %s\n", caps.bus_info);
    printf("version: 0x%u\n", caps.version);
    printf("capabilities: 0x%x\n", caps.capabilities);

    if (V4L2_CAP_VIDEO_CAPTURE & caps.capabilities) {
        printf("video capture\n");
    }

    if (V4L2_CAP_VIDEO_OUTPUT & caps.capabilities) {
        printf("video output\n");
    }

    printf("device_caps: 0x%x\n", caps.device_caps);
    printf("reserved[1]: %u\n", caps.reserved[1]);
    printf("reserved[2]: %u\n", caps.reserved[2]);
    printf("reserved[3]: %u\n", caps.reserved[3]);

    if (!V4L2_CAP_VIDEO_CAPTURE & caps.capabilities) {
        fprintf(stderr, "device doesn't support video capture!! exit...\n");
        return -1;
    }

    printf("enum format:\n");
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    /* enum capture device supported format... */
    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1) {
        printf("\t%d.%s\n", fmtdesc.index+1, fmtdesc.description);
        fmtdesc.index++;
    }
    printf("enum format end:\n");

    printf("enum support frame size...\n");
    while(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) != -1) {
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            printf("line:%d %dx%d\n",__LINE__, frmsize.discrete.width, frmsize.discrete.height);
        } else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE) {
            printf("line:%d %dx%d\n",__LINE__, frmsize.discrete.width, frmsize.discrete.height);
        }
    }
    printf("enum support frame size end\n");

    /* set capture device parameter */
    // standard, such as pal, ntsc... no need...
    // ioctl(fd, VIDIOC_S_STD, &std_id);

    // no need to set crop...
    // ioctl(fd, VIDIOC_S_CROP, &crop);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.height = IMAGE_HEIGHT;
    fmt.fmt.pix.width = IMAGE_WIDTH;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    // fmt.fmt.pix.field = V4L2_FIELD_ANY;
    ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        fprintf(stderr, "v4l2 set format failed, error: %d(%m)\n", errno);
        return -1;
    }


    ret = ioctl(fd, VIDIOC_G_FMT, &fmt);
    if (ret < 0) {
        fprintf(stderr, "v4l2 get format failed, error: %d(%m)\n", errno);
        return -1;
    }

    printf("v4l2 get format:\n");
    printf("type: %d\n", fmt.type);
    printf("pixelformat: %c%c%c%c\n", fmt.fmt.pix.pixelformat & 0xFF,
            (fmt.fmt.pix.pixelformat>>8) & 0xFF,
            (fmt.fmt.pix.pixelformat>>16) & 0xFF,
            (fmt.fmt.pix.pixelformat>>24) & 0xFF);
    printf("height: %d\n", fmt.fmt.pix.height);
    printf("width: %d\n", fmt.fmt.pix.width);
    printf("field:%d\n", fmt.fmt.pix.field);

    // set framerate
    // ioctl(fd, VIDIOC_S_PARM)

    // request buffer
    memset(&req, 0, sizeof(req));
    req.count = 3;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    // req.memory = V4L2_MEMORY_DMABUF;
    req.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(fd, VIDIOC_QBUF, &req);
    if (ret < 0) {
        printf("request buffer failed... errno:%d(%m)\n", errno);
        return -1;
    }

    // query buffer
    ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
    printf("buffer info:\n");
    printf("memory: 0x%x\n", buf.memory);
    printf("offset: 0x%x\n", buf.m.offset);
    printf("length: %d\n", buf.length);

    // close video device
    close(fd);

    printf("exit main...\n");
    return 0;
}
