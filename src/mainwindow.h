#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QColor>

/* Includes for I2C */
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QImage liveimage;
    QPixmap livepix;
    QLabel *livelabel;
    QTimer *timer;

    int i2c_fd;
    int16_t grideye[64];
    int16_t ambient;


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int InitGridEye();
    void UpdateLiveImage();
    int pokestream(uint8_t *data, uint8_t addr, int size);
    int peekstream(uint8_t *data, uint8_t addr, int size);

private slots:
    void ReadGridEye();
};

#endif // MAINWINDOW_H
