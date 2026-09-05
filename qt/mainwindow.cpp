#include "mainwindow.h"

#include <QComboBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QLinearGradient>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>


// ============================================================
// HairDryerWidget
// ============================================================

HairDryerWidget::HairDryerWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}


void HairDryerWidget::setSpeed(int value)
{
    speed = qBound(0, value, 100);
    update();
}


void HairDryerWidget::setMotorOn(bool on)
{
    motorOn = on;
    update();
}


void HairDryerWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRect area = rect().adjusted(10, 10, -10, -10);
    painter.fillRect(area, QColor("#f7f9fc"));

    // 헤어드라이기 본체
    const int bodyX = width() / 2 - 155;
    const int bodyY = height() / 2 - 55;
    const QRectF body(bodyX, bodyY, 210, 110);

    QLinearGradient bodyGradient(body.topLeft(), body.bottomRight());
    bodyGradient.setColorAt(0.0, QColor("#eef4ff"));
    bodyGradient.setColorAt(1.0, QColor("#b7c9e8"));

    painter.setBrush(bodyGradient);
    painter.setPen(QPen(QColor("#60789f"), 3));
    painter.drawRoundedRect(body, 34, 34);

    // 노즐
    QPolygonF nozzle;
    nozzle << QPointF(body.right() - 4, body.top() + 28)
           << QPointF(body.right() + 62, body.top() + 42)
           << QPointF(body.right() + 62, body.bottom() - 42)
           << QPointF(body.right() - 4, body.bottom() - 28);

    painter.setBrush(QColor("#8ea6cc"));
    painter.setPen(QPen(QColor("#60789f"), 3));
    painter.drawPolygon(nozzle);

    // 손잡이
    QPainterPath handle;
    handle.moveTo(body.left() + 65, body.bottom() - 5);
    handle.lineTo(body.left() + 92, body.bottom() - 5);
    handle.lineTo(body.left() + 116, body.bottom() + 105);
    handle.quadTo(body.left() + 119, body.bottom() + 124,
                  body.left() + 99, body.bottom() + 126);
    handle.lineTo(body.left() + 65, body.bottom() + 126);
    handle.quadTo(body.left() + 46, body.bottom() + 124,
                  body.left() + 51, body.bottom() + 105);
    handle.closeSubpath();

    painter.setBrush(QColor("#a9bde0"));
    painter.drawPath(handle);

    // 전원 표시등
    painter.setBrush(motorOn ? QColor("#36d399") : QColor("#a0aec0"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(body.left() + 43, body.center().y()), 9, 9);

    // 풍량 표시
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(11);
    painter.setFont(font);
    painter.setPen(QColor("#53657f"));

    painter.drawText(QRectF(body.left() + 70, body.top() + 25, 120, 25),
                     Qt::AlignCenter,
                     QString("AIR %1%").arg(speed));

    // 바람 표시
    const qreal intensity = motorOn ? speed / 100.0 : 0.0;
    const int alpha = motorOn ? 65 + qRound(190 * intensity) : 35;
    const int penWidth = motorOn ? 2 + qRound(4 * intensity) : 2;

    const QColor windColor(38, 153, 255, alpha);

    painter.setPen(QPen(windColor,
                        penWidth,
                        Qt::SolidLine,
                        Qt::RoundCap));

    painter.setBrush(Qt::NoBrush);

    const int windX = body.right() + 82;
    const int centerY = body.center().y();
    const int extra = qRound(30 * intensity);

    for(int i = 0; i < 3; ++i)
    {
        const int y = centerY - 42 + i * 42;

        QPainterPath wind;

        wind.moveTo(windX, y);

        wind.cubicTo(windX + 25 + extra, y - 15,
                     windX + 46 + extra, y + 15,
                     windX + 78 + extra, y);

        painter.drawPath(wind);
    }

    font.setPointSize(10);
    painter.setFont(font);
    painter.setPen(QColor("#718096"));

    painter.drawText(QRectF(0,
                            height() - 28,
                            width(),
                            22),
                     Qt::AlignCenter,
                     motorOn
                         ? QString("Wind strength: %1%").arg(speed)
                         : QStringLiteral("Motor stopped"));
}


// ============================================================
// MainWindow
// ============================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      serial(new QSerialPort(this))
{
    // .ui 파일 사용하지 않음
    setupUi();

    connect(serial,
            &QSerialPort::readyRead,
            this,
            &MainWindow::readData);

    connect(serial,
            &QSerialPort::errorOccurred,
            this,
            [this](QSerialPort::SerialPortError error)
            {
                if(error != QSerialPort::NoError)
                {
                    appendLog(QString("Serial error: %1")
                                  .arg(serial->errorString()));
                }
            });

    refreshPorts();
}


MainWindow::~MainWindow()
{
    if(serial->isOpen())
    {
        serial->close();
    }
}


// ============================================================
// C++ 코드로 UI 생성
// ============================================================

void MainWindow::setupUi()
{
    setWindowTitle("STM32 Hair Dryer Motor Control");

    resize(760, 800);

    auto *central = new QWidget(this);

    auto *mainLayout = new QVBoxLayout(central);

    mainLayout->setContentsMargins(20, 18, 20, 20);
    mainLayout->setSpacing(14);


    // --------------------------------------------------------
    // Title
    // --------------------------------------------------------

    auto *title =
        new QLabel("HAIR DRYER MOTOR CONTROL", central);

    title->setObjectName("titleLabel");


    auto *subtitle =
        new QLabel("STM32 · ST-LINK VCP · EZ R300 · PWM speed control",
                   central);

    subtitle->setObjectName("subtitleLabel");


    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);


    // --------------------------------------------------------
    // Hair Dryer
    // --------------------------------------------------------

    hairDryerWidget = new HairDryerWidget(central);

    hairDryerWidget->setObjectName("hairDryerWidget");

    mainLayout->addWidget(hairDryerWidget, 1);


    // --------------------------------------------------------
    // Serial
    // --------------------------------------------------------

    auto *serialGroup =
        new QGroupBox("Serial connection", central);

    auto *serialLayout =
        new QHBoxLayout(serialGroup);


    portCombo = new QComboBox(serialGroup);

    refreshButton =
        new QPushButton("Refresh", serialGroup);

    connectButton =
        new QPushButton("Connect", serialGroup);

    connectionLabel =
        new QLabel("Disconnected", serialGroup);


    connectionLabel->setStyleSheet(
        "color: #b00020; font-weight: bold;"
    );


    serialLayout->addWidget(
        new QLabel("Port:", serialGroup)
    );

    serialLayout->addWidget(portCombo, 1);

    serialLayout->addWidget(refreshButton);

    serialLayout->addWidget(connectButton);

    serialLayout->addWidget(connectionLabel);


    // --------------------------------------------------------
    // Motor
    // --------------------------------------------------------

    auto *motorGroup =
        new QGroupBox("Motor control", central);

    auto *motorLayout =
        new QFormLayout(motorGroup);


    motorStateLabel =
        new QLabel("OFF", motorGroup);

    motorStateLabel->setStyleSheet(
        "color: #b00020;"
        "font-size: 18px;"
        "font-weight: bold;"
    );


    directionLabel =
        new QLabel("FORWARD", motorGroup);


    speedSlider =
        new QSlider(Qt::Horizontal, motorGroup);

    speedSlider->setRange(0, 100);

    speedSlider->setValue(50);

    speedSlider->setTickPosition(
        QSlider::TicksBelow
    );

    speedSlider->setTickInterval(10);


    speedLabel =
        new QLabel("50 %", motorGroup);


    motorButton =
        new QPushButton("Motor ON", motorGroup);

    motorButton->setEnabled(false);


    forwardButton =
        new QPushButton("Forward", motorGroup);

    reverseButton =
        new QPushButton("Reverse", motorGroup);


    forwardButton->setEnabled(false);

    reverseButton->setEnabled(false);


    auto *directionButtons =
        new QHBoxLayout();

    directionButtons->addWidget(
        forwardButton
    );

    directionButtons->addWidget(
        reverseButton
    );


    motorLayout->addRow(
        "Motor state:",
        motorStateLabel
    );

    motorLayout->addRow(
        "Direction:",
        directionLabel
    );

    motorLayout->addRow(
        "Speed:",
        speedSlider
    );

    motorLayout->addRow(
        "Current speed:",
        speedLabel
    );

    motorLayout->addRow(
        "Command:",
        motorButton
    );

    motorLayout->addRow(
        "Direction command:",
        directionButtons
    );


    // --------------------------------------------------------
    // Log
    // --------------------------------------------------------

    logEdit =
        new QPlainTextEdit(central);

    logEdit->setReadOnly(true);

    logEdit->setPlaceholderText(
        "Serial messages will appear here..."
    );


    mainLayout->addWidget(serialGroup);

    mainLayout->addWidget(motorGroup);

    mainLayout->addWidget(
        new QLabel("Communication log:", central)
    );

    mainLayout->addWidget(logEdit, 1);


    setCentralWidget(central);


    // --------------------------------------------------------
    // Style
    // --------------------------------------------------------

    setStyleSheet(R"(
        QMainWindow {
            background: #eef2f7;
        }

        QLabel#titleLabel {
            color: #1f3a5f;
            font-size: 24px;
            font-weight: 800;
        }

        QLabel#subtitleLabel {
            color: #718096;
            font-size: 12px;
            margin-bottom: 2px;
        }

        QGroupBox {
            background: white;
            border: 1px solid #d9e2ef;
            border-radius: 12px;
            margin-top: 10px;
            padding: 14px;
            font-weight: 700;
            color: #334e68;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 14px;
            padding: 0 6px;
        }

        QPushButton {
            background: #2f80ed;
            color: white;
            border: none;
            border-radius: 7px;
            padding: 8px 14px;
            font-weight: 700;
        }

        QPushButton:hover {
            background: #1769d0;
        }

        QPushButton:disabled {
            background: #b7c2d0;
        }

        QComboBox,
        QPlainTextEdit {
            background: white;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 5px;
        }

        QSlider::groove:horizontal {
            height: 8px;
            background: #dbe6f4;
            border-radius: 4px;
        }

        QSlider::sub-page:horizontal {
            background: #2f80ed;
            border-radius: 4px;
        }

        QSlider::handle:horizontal {
            width: 20px;
            margin: -7px 0;
            border-radius: 10px;
            background: #1769d0;
        }
    )");


    // --------------------------------------------------------
    // Signal / Slot
    // --------------------------------------------------------

    connect(refreshButton,
            &QPushButton::clicked,
            this,
            &MainWindow::refreshPorts);


    connect(connectButton,
            &QPushButton::clicked,
            this,
            &MainWindow::connectSerial);


    connect(motorButton,
            &QPushButton::clicked,
            this,
            &MainWindow::toggleMotor);


    connect(speedSlider,
            &QSlider::valueChanged,
            this,
            &MainWindow::changeSpeed);


    connect(forwardButton,
            &QPushButton::clicked,
            this,
            &MainWindow::setForward);


    connect(reverseButton,
            &QPushButton::clicked,
            this,
            &MainWindow::setReverse);
}


// ============================================================
// COM Port Refresh
// ============================================================

void MainWindow::refreshPorts()
{
    const QString selected =
        portCombo->currentText();

    portCombo->clear();


    const auto ports =
        QSerialPortInfo::availablePorts();


    for(const QSerialPortInfo &info : ports)
    {
        portCombo->addItem(
            info.portName(),
            info.portName()
        );
    }


    const int index =
        portCombo->findText(selected);


    if(index >= 0)
    {
        portCombo->setCurrentIndex(index);
    }


    appendLog(
        QString("Detected ports: %1")
            .arg(ports.size())
    );
}


// ============================================================
// Serial Connect
// ============================================================

void MainWindow::connectSerial()
{
    // 연결 해제
    if(serial->isOpen())
    {
        serial->close();

        connectButton->setText(
            "Connect"
        );

        connectionLabel->setText(
            "Disconnected"
        );

        connectionLabel->setStyleSheet(
            "color: #b00020;"
            "font-weight: bold;"
        );

        motorButton->setEnabled(false);

        forwardButton->setEnabled(false);

        reverseButton->setEnabled(false);

        appendLog(
            "Serial disconnected"
        );

        return;
    }


    if(portCombo->currentText().isEmpty())
    {
        QMessageBox::warning(
            this,
            "Serial",
            "Select a COM port first."
        );

        return;
    }


    serial->setPortName(
        portCombo->currentText()
    );


    serial->setBaudRate(
        QSerialPort::Baud115200
    );

    serial->setDataBits(
        QSerialPort::Data8
    );

    serial->setParity(
        QSerialPort::NoParity
    );

    serial->setStopBits(
        QSerialPort::OneStop
    );

    serial->setFlowControl(
        QSerialPort::NoFlowControl
    );


    if(!serial->open(QIODevice::ReadWrite))
    {
        QMessageBox::critical(
            this,
            "Serial",
            serial->errorString()
        );

        return;
    }


    connectButton->setText(
        "Disconnect"
    );

    connectionLabel->setText(
        "Connected"
    );

    connectionLabel->setStyleSheet(
        "color: #008000;"
        "font-weight: bold;"
    );


    motorButton->setEnabled(true);

    forwardButton->setEnabled(true);

    reverseButton->setEnabled(true);


    appendLog(
        QString("Connected: %1 @ 115200 baud")
            .arg(serial->portName())
    );


    // STM32 현재 상태 요청
    sendCommand("STATUS\n");
}


// ============================================================
// Motor ON / OFF
// ============================================================

void MainWindow::toggleMotor()
{
    sendCommand(
        motorOn ? "OFF\n" : "ON\n"
    );
}


// ============================================================
// Speed
// ============================================================

void MainWindow::changeSpeed(int value)
{
    speedLabel->setText(
        QString::number(value) + " %"
    );

    hairDryerWidget->setSpeed(value);


    if(serial->isOpen())
    {
        sendCommand(
            QString("SPEED:%1\n")
                .arg(value)
        );
    }
}


// ============================================================
// Forward / Reverse
// ============================================================

void MainWindow::setForward()
{
    sendCommand("FORWARD\n");
}


void MainWindow::setReverse()
{
    sendCommand("REVERSE\n");
}


// ============================================================
// Send
// ============================================================

void MainWindow::sendCommand(const QString &command)
{
    if(!serial->isOpen())
    {
        appendLog("Not connected");
        return;
    }


    serial->write(
        command.toUtf8()
    );


    appendLog(
        "TX  " + command.trimmed()
    );
}


// ============================================================
// Receive
// ============================================================

void MainWindow::readData()
{
    rxBuffer += serial->readAll();


    while(rxBuffer.contains('\n'))
    {
        const int end =
            rxBuffer.indexOf('\n');


        const QByteArray line =
            rxBuffer.left(end).trimmed();


        rxBuffer.remove(
            0,
            end + 1
        );


        if(!line.isEmpty())
        {
            processMessage(line);
        }
    }
}


// ============================================================
// STM32 Message
// ============================================================

void MainWindow::processMessage(const QByteArray &message)
{
    const QString text =
        QString::fromUtf8(message);


    appendLog(
        "RX  " + text
    );


    // MOTOR
    if(text == "MOTOR:ON")
    {
        setMotorState(true);
    }

    else if(text == "MOTOR:OFF")
    {
        setMotorState(false);
    }


    // Direction
    else if(text == "DIR:FORWARD")
    {
        directionLabel->setText(
            "FORWARD"
        );
    }

    else if(text == "DIR:REVERSE")
    {
        directionLabel->setText(
            "REVERSE"
        );
    }


    // Speed
    else if(text.startsWith("SPEED:"))
    {
        bool ok = false;


        const int speed =
            text.mid(6).toInt(&ok);


        if(ok &&
           speed >= 0 &&
           speed <= 100)
        {
            // STM32에서 들어온 값으로 Slider 변경 시
            // 다시 SPEED 명령이 송신되는 것을 방지
            speedSlider->blockSignals(true);

            speedSlider->setValue(speed);

            speedSlider->blockSignals(false);


            speedLabel->setText(
                QString::number(speed) + " %"
            );


            hairDryerWidget->setSpeed(speed);
        }
    }


    // PA0 Button
    else if(text == "BUTTON:PRESS")
    {
        appendLog(
            "STM32 PA0 button pressed"
        );
    }
}


// ============================================================
// UI Motor State
// ============================================================

void MainWindow::setMotorState(bool on)
{
    motorOn = on;


    hairDryerWidget->setMotorOn(on);


    motorStateLabel->setText(
        on ? "ON" : "OFF"
    );


    motorStateLabel->setStyleSheet(
        on
            ? "color: #008000; font-size: 18px; font-weight: bold;"
            : "color: #b00020; font-size: 18px; font-weight: bold;"
    );


    motorButton->setText(
        on ? "Motor OFF" : "Motor ON"
    );
}


// ============================================================
// Log
// ============================================================

void MainWindow::appendLog(const QString &message)
{
    logEdit->appendPlainText(message);
}
