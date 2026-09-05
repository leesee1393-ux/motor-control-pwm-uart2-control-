#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QWidget>

class QComboBox;
class QPushButton;
class QSlider;
class QLabel;
class QPlainTextEdit;
class QSerialPort;

class HairDryerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HairDryerWidget(QWidget *parent = nullptr);
    void setSpeed(int speed);
    void setMotorOn(bool on);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int speed = 0;
    bool motorOn = false;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void refreshPorts();
    void connectSerial();
    void toggleMotor();
    void changeSpeed(int value);

    // 필요한 기능만 추가
    void setForward();
    void setReverse();

    void readData();

private:
    void setupUi();
    void sendCommand(const QString &command);
    void processMessage(const QByteArray &message);
    void setMotorState(bool on);
    void appendLog(const QString &message);

    QSerialPort *serial;
    QByteArray rxBuffer;

    QComboBox *portCombo;
    QPushButton *refreshButton;
    QPushButton *connectButton;

    QPushButton *motorButton;
    QPushButton *forwardButton;
    QPushButton *reverseButton;

    QSlider *speedSlider;

    QLabel *connectionLabel;
    QLabel *motorStateLabel;
    QLabel *directionLabel;
    QLabel *speedLabel;

    QPlainTextEdit *logEdit;
    HairDryerWidget *hairDryerWidget;

    bool motorOn = false;
};

#endif // MAINWINDOW_H
