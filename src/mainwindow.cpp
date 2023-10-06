#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QWidget *window = new QWidget();

    /* Set up background color to eTS blue */
    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, "#4a5fa5");
    window->setAutoFillBackground(true);
    window->setPalette(palette);

    this->showFullScreen();
    this->setCursor(Qt::BlankCursor);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setMargin(100);

    liveimage = QImage(8, 8, QImage::Format_RGB32);
    liveimage.fill(0);

    livepix = QPixmap(QPixmap::fromImage((liveimage)));

    livelabel = new QLabel();
    vbox->addWidget(livelabel);
    vbox->setAlignment(livelabel, Qt::AlignHCenter);
    livelabel->setPixmap(livepix.scaled(800, 800, Qt::KeepAspectRatio, Qt::FastTransformation));

    /* Set up eTS logo */
    QPixmap image(":/image/embeddedts-color-outline.svg");
    QLabel *imagelabel = new QLabel();
    imagelabel->setPixmap(image.scaledToHeight(112));
    vbox->addWidget(imagelabel);
    vbox->setAlignment(imagelabel, Qt::AlignRight);

    InitGridEye();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(ReadGridEye()));
    timer->start(100);




    window->setLayout(vbox);
    setCentralWidget(window);
}

MainWindow::~MainWindow()
{

}

int MainWindow::pokestream(uint8_t *data, uint8_t addr, int size)
{
        struct i2c_rdwr_ioctl_data packets;
        struct i2c_msg msg;
        uint8_t outdata[4096];

        outdata[0] = addr;
        memcpy(&outdata[1], data, size);

        msg.addr = 0x68;
        msg.flags = 0;
        msg.len = 1 + size;
        msg.buf = (unsigned char *)outdata;

        packets.msgs  = &msg;
        packets.nmsgs = 1;

        if(ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
                return 1;
        }
        return 0;
}

int MainWindow::peekstream(uint8_t *data, uint8_t addr, int size)
{
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg msgs[2];
    unsigned char busaddr[1];

    busaddr[0] = addr;

    msgs[0].addr  = 0x68;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = busaddr;

    msgs[1].addr  = 0x68;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len   = size;
    msgs[1].buf   = data;

    packets.msgs  = msgs;
    packets.nmsgs = 2;

    if(ioctl(i2c_fd, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 1;
    }

    return 0;
}


int MainWindow::InitGridEye()
{
    unsigned char buf[1];

    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
	qCritical() << "Failed to open I2C device";
        return 1;
    }

    if ( ioctl(i2c_fd, I2C_SLAVE, 0x68) < 0) {
        qCritical() << "Failed to set slave address 0x68";
        return 1;
    }


    /* Set normal mode */
    buf[0] = 0x00;
    pokestream(buf, 0x00, 1);

    /* Clear flags */
    buf[0] = 0x30;
    pokestream(buf, 0x01, 1);

    /* Set 10 FPS */
    buf[0] = 0x00;
    pokestream(buf, 0x02, 1);

    /* Set moving average mode */
    buf[0] = 0x50;
    pokestream(buf, 0x1F, 1);
    buf[0] = 0x45;
    pokestream(buf, 0x1F, 1);
    buf[0] = 0x57;
    pokestream(buf, 0x1F, 1);
    buf[0] = 0x20;
    pokestream(buf, 0x07, 1);
    buf[0] = 0x00;
    pokestream(buf, 0x1F, 1);

    return 0;
}

void MainWindow::ReadGridEye()
{
    uint8_t i, x;
    uint8_t grid_buf[128];
    uint8_t ambient_buf[2];

    peekstream(ambient_buf, 0x0E, 2);
    peekstream(grid_buf, 0x80, 128);


    /* Swizzle data from 2x 8bit quantities with sign to signed 16 bit
     * Datatype is stored as 2s compliment, but what we get from sensor
     * is not. Need to check for sign bit, and then apply 2s compliment math
     * if needed
     *
     */
    ambient = ((ambient_buf[0] & 0xFF) |
        ((ambient_buf[1] & 0x07) << 8));
    if (!!(ambient_buf[1] & (1 << 3))) {
        ambient = (~ambient) + 1;
    }

    /* Get grid point temperatures. Values are 0.25 deg. C per bit */
    for (i = 0; i < 128; i++) {
        x = i / 2;
        grideye[x] = ((grid_buf[i++] & 0xFF));
        grideye[x] |= ((grid_buf[i] & 0x07) << 8);
        /* If negative, base number is already 2s comp. Need to fill in rest of
         * bits of the variable to set that uniformly.
         */
        if (!!(grid_buf[i] & (1 << 3))) {
            grideye[x] |= 0xF800;
        }
        grideye[x] = (grideye[x]/4);
    }

    UpdateLiveImage();

    //qInfo("Ambient should be %d %d", ambient, grideye[0]);
}

void MainWindow::UpdateLiveImage()
{
    int x, y;
    QRgb value;
    int16_t point_val;

    /* The Grid-EYE sensor starts at the bottom left, while the pixmap starts
     * at top left. Both perspectives are looking at the sensor/pixmap.
     */

    for (y = 0; y < 8; y++) {
        for (x = 0; x < 8; x++){
            /* Normalize for 5 to 37 degrees C */
            point_val = grideye[(63-((y*8)+(7-x)))] - 5;
            if (point_val < 0) point_val = 0;
            if (point_val > 32) point_val = 31;
            point_val = point_val / 2;

            value = qRgb(
              (point_val*17),
              0,
              (255 - (point_val*17)));

            /* Below is a greenscale test */
            //value = qRgb(0, (grideye[(63-((y*8)+(7-x)))] * 2), 0);
            liveimage.setPixel(x, y, value);
        }
    }

    livepix = QPixmap::fromImage((liveimage));
    livelabel->setPixmap(livepix.scaled(800, 800, Qt::KeepAspectRatio, Qt::FastTransformation));
}

