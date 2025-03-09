// cmake -S . -B build && cmake --build build && ./build/mqtt-qt

#include <QApplication>
#include <QtMqtt/qmqttclient.h>
#include <QtWidgets>

#include <QThread>
#include <thread>

#include <sstream>
#include <fstream>
// #include <unistd.h>
#include <picojson.h>

#include <qcustomplot.h>
#include <qcgaugewidget.h>

#include "gui.h"

using std::thread;

QString PASSWORD = "tetx";
QString CLIENT_PASSWORD = "cp";

namespace json = picojson;

#define DEBUG false

#define NUM_LED 30
#define NUM_NAV 3
#define NUM_GIS 4
#define NUM_BARS 4
#define NUM_TAS_BARS 6
#define NUM_GAUGES 6
#define NUM_PLOTS 27
#define NUM_PLOT_OPTIONS 56
#define NUM_REBOOT 5
#define NUM_DIO_RLYS 20
#define NUM_U3_RLYS 16

//IPMI boot requiest
#define IPMI_BOOT "IPMI/manual_boot"
#define BOOT_SIGNAL_DURATION_MIN 15

//DIO registers
#define FANS_ONOFF "DIO/control/fans"
#define EXFAN1_ONOFF "DIO/control/exfan1"
#define EXFAN2_ONOFF "DIO/control/exfan2" 
#define MCCRST_ONOFF "DIO/control/mccrst"
#define BCCRST_ONOFF "DIO/control/bccrst"
#define MCCHB_ONOFF "DIO/control/mcchb"
#define BCCHB_ONOFF "DIO/control/bcchb"
#define RLY1_ONOFF "DIO/control/rly1"
#define RLY2_ONOFF "DIO/control/rly2"
#define RLY3_ONOFF "DIO/control/rly3"
#define RLY4_ONOFF "DIO/control/rly4"
#define RLY5_ONOFF "DIO/control/rly5"
#define RLY6_ONOFF "DIO/control/rly6"
#define RLY7_ONOFF "DIO/control/rly7"
#define RLY8_ONOFF "DIO/control/rly8" // 2U power pin
#define RLY9_ONOFF "DIO/control/rly9"
#define RLY10_ONOFF "DIO/control/rly10"

#define L1_ONOFF "2U_DIO/SSR3_L1"
#define L2_ONOFF "2U_DIO/SSR4_L2"
#define DMA_ONOFF "2U_DIO/SSR5_AUX1" //DMA
#define APLX_ONOFF "2U_DIO/SSR6_AUX2" //APLX
#define BLWR_ONOFF "2U_DIO/SSR2_BLWR" //compressor - air steam to all sesors to block debris
#define VARIPHASE_ONOFF "2U_DIO/variphase_an"

//the sleep time used when allwing the enclosured to close or open. this value can be adjusted by the GUI itself.
int IMG_action_duration_time = 20;
int LIDAR_action_duration_time = 10;


std::string ledID[NUM_LED] = {
    "health",
    "server",
    "hvac",
    "cell",
    "man",
    "sliding",
    "smokea",
    "smokeb",
    "fssalert",
    "fsstbl",
    "cam_man",
    "cam_out_man",
    "cam_panel",
    "cam_outa",
    "cam_outb",
    "gen",
    "fana",
    "fanb",
    "sim",
    "compress",
    "l1",
    "l2",
    "dma",
    "aplx",
    "d3_stat",
    "li_stat",
    "nas_stat",
    "img_stat",
    "rw_stat",
    "dma_stat"
};

std::string navID[NUM_NAV] = {
    "Lat",
    "Long",
    "Speed"
};

std::string gisID[NUM_GIS] = {
    "runid",
    "division",
    "subdivision",
    "line"
};

std::string rebootID[NUM_REBOOT] = {
    "d3_reboots",
    "li_reboots",
    "nas_reboots",
    "img_reboots",
    "rw_reboots"
};

std::string barID[NUM_BARS] = {
    "ProgBar1",
    "ProgBar2",
    "ProgBar3",
    "ProgBar4"
};
std::string TASbarID[NUM_TAS_BARS] = {
    "ProgBar5",
    "ProgBar6",
    "ProgBar7",
    "ProgBar8",
    "ProgBar9",
    "ProgBar10"
};

std::string plotID[NUM_PLOT_OPTIONS] = {
    "plot1",
    "plot2",
    "plot5",
    "plot3",
    "plot6",
    "plot4",
    "plot7",
    "plot8",
    "plot12",
    "plot9",
    "plot13",
    "plot10",
    "plot14",
    "plot11",
    "plot15",
    "plot16",
    "plot17",
    "plot18",
    "plot23",
    "plot19",
    "plot24",
    "plot20",
    "plot25",
    "plot21",
    "plot26",
    "plot22",
    "plot27",
    "plot28",
    "plot29",
    "plot30",
    "plot31",
    "plot32",
    "plot33",
    "plot34",
    "plot35",
    "plot36",
    "plot37",
    "plot38",
    "plot39",
    "plot40",
    "plot41",
    "plot42",
    "plot43",
    "plot44",
    "plot45",
    "plot46",
    "plot47",
    "plot48",
    "plot49",
    "plot50",
    "plot51",
    "plot52",
    "plot53",
    "plot54",
    "plot55",
    "plot56"
};

std::string gaugeID[NUM_GAUGES] = {
    "batt",
    "load",
    "rpm",
    "solar",
    "charge",
    "fuel"
};

std::string dioRlyID[NUM_DIO_RLYS] = {
    "dio_rly1",
    "dio_rly1",
    "dio_rly2",
    "dio_rly2",
    "dio_rly3",
    "dio_rly3",
    "dio_rly4",
    "dio_rly4",
    "dio_rly5",
    "dio_rly5",
    "dio_rly6",
    "dio_rly6",
    "dio_rly7",
    "dio_rly7",
    "dio_rly8",
    "dio_rly8",
    "dio_rly9",
    "dio_rly9",
    "dio_rly10",
    "dio_rly10"
};

std::string U3RlyID[NUM_U3_RLYS] = {
    "U3_APLX",
    "U3_APLX",
    "U3_DMA",
    "U3_DMA",
    "U3_L1",
    "U3_L1",
    "U3_L2",
    "U3_L2",
    "U3_BLWR",
    "U3_BLWR",
    "U3_VARIPHASE",
    "U3_VARIPHASE",
    "dio_rly8",
    "dio_rly8",
    "dio_rly8",
    "dio_rly8"
};


/* Mqtt Endpoint config */
struct MqttEndpoint {
    QString host;
    qint16 port;
    QString topic;

    static MqttEndpoint fromJson(json::object &object) {
        MqttEndpoint result;
        result.host = QString::fromStdString(object["host"].get<std::string>());
        result.port = static_cast<qint16>(object["port"].get<double>());
        result.topic = QString::fromStdString(object["topic"].get<std::string>());

        return result;
    }

    operator QString() const {
        return QString("%1:%2[%3]").arg(host).arg(port).arg(topic);
    }
};

/* Global app config */
struct MqttConfig {
    public:
        MqttEndpoint plotEndpoint[NUM_PLOT_OPTIONS];
        MqttEndpoint gaugeEndpoint[NUM_GAUGES];
        MqttEndpoint barEndpoint[NUM_BARS];
        MqttEndpoint navEndpoint[NUM_NAV];
        MqttEndpoint gisEndpoint[NUM_GIS];
        MqttEndpoint testEndpoint;
        MqttEndpoint LedEndpoint[NUM_LED];
        MqttEndpoint TASbarEndpoint[NUM_TAS_BARS];
        MqttEndpoint rebootEndpoint[NUM_REBOOT];
        MqttEndpoint dioRlyEndpoint[NUM_DIO_RLYS];
        MqttEndpoint U3RlyEndpoint[NUM_U3_RLYS];

    static MqttConfig fromJson(json::object &object) {
        MqttConfig result;
        result.testEndpoint = MqttEndpoint::fromJson(object["test"].get<json::object>());

        for (int i = 0; i < NUM_LED; ++i){
            // qDebug() << QString("parsing json LED %1").arg(i);
            result.LedEndpoint[i] = MqttEndpoint::fromJson(object[ledID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_NAV; ++i){
            result.navEndpoint[i] = MqttEndpoint::fromJson(object[navID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_GIS; ++i){
            result.gisEndpoint[i] = MqttEndpoint::fromJson(object[gisID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_BARS; ++i){
            result.barEndpoint[i] = MqttEndpoint::fromJson(object[barID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_TAS_BARS; ++i){
            result.TASbarEndpoint[i] = MqttEndpoint::fromJson(object[TASbarID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_PLOT_OPTIONS; ++i){
            result.plotEndpoint[i] = MqttEndpoint::fromJson(object[plotID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_GAUGES; ++i){
            result.gaugeEndpoint[i] = MqttEndpoint::fromJson(object[gaugeID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_REBOOT; ++i){
            result.rebootEndpoint[i] = MqttEndpoint::fromJson(object[rebootID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_DIO_RLYS; ++i){
            result.dioRlyEndpoint[i] = MqttEndpoint::fromJson(object[dioRlyID[i].c_str()].get<json::object>());
        }

        for (int i = 0; i < NUM_U3_RLYS; ++i){
            result.U3RlyEndpoint[i] = MqttEndpoint::fromJson(object[U3RlyID[i].c_str()].get<json::object>());
        }

        return result;
    }

    static MqttConfig fromFile(const QString &filepath) {
        std::ifstream file(filepath.toStdString());
        if (!file) {
            qCritical() << QString("Failed to open %1 file").arg(filepath);
            return {};
        }

        json::value root;
        QString err = QString::fromStdString(picojson::parse(root, file));
        if (!err.isEmpty()) {
            qCritical() << QString("Error parsing file %1 JSON: %1").arg(filepath).arg(err);
            return {};
        }

        return fromJson(root.get<json::object>());
    }
};

class MqttClient : public QObject {
    Q_OBJECT
    public:
    // MqttClient(const MqttEndpoint &config, std::function<void(double)> payloadDoubleCallback, QObject *parent = nullptr)
    MqttClient(const MqttEndpoint &config, std::function<void(void*)> payloadCallback, QObject *parent = nullptr)
        : QObject(parent), config(config), mqttClient(this), payloadCallback(payloadCallback), shutdown(false) {
        connect(&mqttClient, &QMqttClient::stateChanged, this, &MqttClient::handleStateChanged);
        connect(&mqttClient, &QMqttClient::messageReceived, this, &MqttClient::handleMessageReceived);
    }

    virtual ~MqttClient() {
        shutdown = true;
    }

    void start() {
        mqttClient.setHostname(config.host);
        mqttClient.setPort(config.port);
        mqttClient.connectToHost();
    }

    void stop() {
        mqttClient.unsubscribe(config.topic);
        mqttClient.disconnectFromHost();
        shutdown = true;
    }

    qint32 publish(const QMqttTopicName &topic, const QByteArray &message = QByteArray(), quint8 qos = 0, bool retain = false) {
        return mqttClient.publish(topic, message, qos, retain);
    }

    private slots:
    void handleStateChanged(QMqttClient::ClientState state) {
        if (state == QMqttClient::Connected) {
            mqttClient.subscribe(config.topic, QOS);
            if(DEBUG)
		qDebug() << QString("Connected to %1:%2 broker and subscribed to %3").arg(config.host).arg(config.port).arg(config.topic);
        } else if (state == QMqttClient::Disconnected && !shutdown) {
            qWarning() << QString("Connection to %1:%2 failed or lost. Reconnecting...").arg(config.host).arg(config.port);
            QTimer::singleShot(RECONNECT_DELAY, this, &MqttClient::start);
        }
    }

    void handleMessageReceived(const QByteArray &message, const QMqttTopicName &topicName) {
        // double receivedValue = QString::fromUtf8(message).toDouble();

        bool ok;

        // qDebug() << QString("got Message. Length: %1").arg(QString::fromUtf8(message).length());

        int i = QString::fromUtf8(message).toInt(&ok, 10);
        if(ok){//received int
	    if(DEBUG)
            qDebug() << QString("Received int data (%1) on a topic: %2").arg(i).arg(topicName.name());
            payloadCallback(&i);
            return;
        }
        double dbl = QString::fromUtf8(message).toDouble(&ok);
        if(ok){//received double
            if(DEBUG)
	    qDebug() << QString("Received Double data (%1) on a topic: %2").arg(dbl).arg(topicName.name());
            payloadCallback(&dbl);
            return;
        }
        float f = QString::fromUtf8(message).toFloat(&ok);
        if(ok){//received float
            if(DEBUG)
	    qDebug() << QString("Received float data (%1) on a topic: %2").arg(f).arg(topicName.name());
            payloadCallback(&f);
            return;
        }


        if(QString::fromUtf8(message).length() == 1) {

            QString str(message.toStdString().c_str());
            if(DEBUG)
	    qDebug() << QString("got bool string %1").arg(str);
            bool ON = true, OFF = false;
            char ch1 = ON;
            QString str2(ch1);
            QString str3("");

            if(str.compare(str2) == 0){
                if(DEBUG)
		qDebug() << QString("Received bool data TRUE on a topic: %1").arg(topicName.name());
                payloadCallback(&ON);
                return;
            }
            else if(str.compare(str3) == 0) {
                if(DEBUG)
		qDebug() << QString("Received bool data FALSE on a topic: %1").arg(topicName.name());
                payloadCallback(&OFF);
                return;
            }
        }

        //ASSUMING the data is string
        std::string str = QString::fromUtf8(message).toStdString();
        if(DEBUG)
	qDebug() << QString("Received text data (%1) on a topic: %2").arg(QString::fromUtf8(message)).arg(topicName.name());
        payloadCallback(&str);
        return;

        // qDebug() << QString("Received invalid data on topic: %1").arg(topicName.name());
    }

    private:
    static constexpr size_t RECONNECT_DELAY = 5000;
    static constexpr size_t QOS = 0;
    const MqttEndpoint &config;
    QMqttClient mqttClient;
    std::function<void(void*)> payloadCallback;
    bool shutdown;
};

QcNeedleItem *setupBasicGaugeWidget(QcGaugeWidget& gaugeWidget, QPair<int, int> valuesRange, int denumerator, std::string paramLabel) {
    const QPair<int, int> gaugeRange = { valuesRange.first / denumerator, valuesRange.second / denumerator };

    // Create the gauge widget
    // auto BasicGauge = new QcGaugeWidget;

    // Add a 225 degrees arc
    gaugeWidget.addArc(55);

    // Add tics from 0 to 100 (by default one tic each 10) and set the range between 0 and 100
    gaugeWidget.addDegrees(65)->setValueRange(0,100);

    // Add a color band
    auto clrBand = gaugeWidget.addColorBand(50);

    // Set colors between 0 and 100
    clrBand->setValueRange(0,100);

    // Add values and set the range between 0 and 100 (by default one each 10)
    gaugeWidget.addValues(80)->setValueRange(gaugeRange.first, gaugeRange.second);

    // Add a label and set the text
    gaugeWidget.addLabel(70)->setText(QString("X%1 %2").arg(denumerator).arg(paramLabel.c_str()));

    // Add a label (to be connected to the needle)
    auto lab = gaugeWidget.addLabel(40);

    // Set the label as 0
    lab->setText("0");

    // Add a new needle
    auto mAirspeedNeedle = gaugeWidget.addNeedle(60);

    // Assign the label to the needle
    mAirspeedNeedle->setLabel(lab);

    // Set the needle color
    mAirspeedNeedle->setColor(Qt::blue);

    // Set the needle values' range
    mAirspeedNeedle->setValueRange(gaugeRange.first, gaugeRange.second);

    return mAirspeedNeedle;
}

QcNeedleItem *setupGaugeWidget(QcGaugeWidget& gaugeWidget, QPair<int, int> valuesRange, int denumerator) {
    const QPair<int, int> gaugeRange = { valuesRange.first / denumerator, valuesRange.second / denumerator };

    gaugeWidget.addBackground(99);
    auto background1 = gaugeWidget.addBackground(92);
    background1->clearrColors();
    background1->addColor(0.1, Qt::black);
    background1->addColor(1.0, Qt::white);

    auto background2 = gaugeWidget.addBackground(88);
    background2->clearrColors();
    background2->addColor(0.1, Qt::gray);
    background2->addColor(1.0, Qt::darkGray);

    gaugeWidget.addArc(55);
    gaugeWidget.addDegrees(65)->setValueRange(gaugeRange.first, gaugeRange.second);
    // gaugeWidget.addColorBand(50,0,0,0);
    gaugeWidget.addColorBand(50);

    gaugeWidget.addValues(80)->setValueRange(gaugeRange.first, gaugeRange.second);

    gaugeWidget.addLabel(70)->setText(QString("X %1 RPM").arg(denumerator));

    QcLabelItem *lab = gaugeWidget.addLabel(40);
    lab->setText("0");

    auto gaugeNeedleItem = gaugeWidget.addNeedle(60);
    gaugeNeedleItem->setLabel(lab);
    gaugeNeedleItem->setColor(Qt::white);
    gaugeNeedleItem->setValueRange(gaugeRange.first, gaugeRange.second);
    gaugeWidget.addBackground(7);
    gaugeWidget.addGlass(88);

    return gaugeNeedleItem;
}

void setupPlotWidget(QCustomPlot &plotWidget, QPair<int, int> valuesRange, int denumerator, std::string paramLabel) {

    plotWidget.setInteraction(QCP::iRangeDrag, true);
    plotWidget.setInteraction(QCP::iRangeZoom, true);
    plotWidget.setInteraction(QCP::iSelectPlottables, true);

    plotWidget.setBackground(QBrush(Qt::white));
    plotWidget.addGraph();
    plotWidget.graph(0)->setPen(QPen(Qt::blue));
    plotWidget.graph(0)->setBrush(QBrush(QColor(0, 0, 255, 10)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    plotWidget.xAxis->setTicker(timeTicker);
    plotWidget.axisRect()->setupFullAxesBox();

    plotWidget.yAxis->setRange(valuesRange.first / denumerator - 1, valuesRange.second / denumerator + 1);

    // plotWidget.xAxis->setLabel("Time");
    plotWidget.yAxis->setLabel(QString("X %1 %2").arg(denumerator).arg(paramLabel.c_str()));
}








//GLOBALS
MqttClient* plotMqttClientArr[NUM_PLOTS];

int main(int argc, char *argv[]) {


    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Health Dashboard");

    gui* g = new gui();
    g->setupUi(&mainWindow);
    g->setWindowTitle("my app");

    g->pushButton_56->setDisabled(true); //disabling NAS soft shutdown btn

    //##### Disable Tabs to force password entry #####
    QTabWidget* tabWidget_ptr = g->tabWidget;
    for (int i = 1; i < 7; ++i){
        tabWidget_ptr->setTabEnabled(i, false);
    }

    const QString configPath = (argc > 1) ? argv[1] : "./config.json";

    if (!QFile(configPath).exists()) {
        qCritical() << QString("%1 doesn't exists. Exit.").arg(configPath);
        return 1;
    }





    // const auto [plotEndpoint, gaugeEndpoint, barEndpoint, navEndpoint, gisEndpoint] = MqttConfig::fromFile(configPath);
    const auto [plotEndpoint, gaugeEndpoint, barEndpoint, navEndpoint, gisEndpoint, testEndpoint, LedEndpoint, TASbarEndpoint, rebootEndpoint, dioRlyEndpoint, U3RlyEndpoint] = MqttConfig::fromFile(configPath);


    MqttClient* testClient = new MqttClient(
        testEndpoint, [](void* payload) {
            if(DEBUG)
		qDebug() << "publishing data";
        }, &mainWindow
    );
    testClient->start();



    //######### HOME LEDS ############
    MqttClient* ledMqttClientArr[NUM_LED];

    QLabel* ledLabeles[NUM_LED] = {
        g->label_123, //home
        g->label_124,
        g->label_125,
        g->label_179,
        g->label_14, //sensor state
        g->label_22,
        g->label_23,
        g->label_24,
        g->label_25,
        g->label_26,
        g->label_28, //cam state
        g->label_34,
        g->label_35,
        g->label_36,
        g->label_37,
        g->label_126, //power state
        g->label_127,
        g->label_128,
        g->label_129,
        g->label_130,
        g->label_131,
        g->label_132,
        g->label_133,
        g->label_134,
        g->label_141, //TAS status
        g->label_142,
        g->label_143,
        g->label_144,
        g->label_145,
        g->label_146
    };


    for (int i = 0; i < NUM_LED; ++i){
        QLabel* LedLabel_ptr = ledLabeles[i];
        ledMqttClientArr[i] = new MqttClient(
            LedEndpoint[i], [LedLabel_ptr, i](void* payload) {

                bool led = *((bool*)payload);
                if(DEBUG)
		qDebug() << "Update LED label with new data: " << led;
                QPalette palette = LedLabel_ptr->palette();

                if(i <= 3){ //home LED
                    if(led){
                        LedLabel_ptr->setPixmap( QPixmap( "/home/cp/Control_GUI/icons/green_orig.png" ) );
                    } else LedLabel_ptr->setPixmap( QPixmap( "/home/cp/Control_GUI/icons/red_orig.png" ) );
                }
                else if(i>3 && i<=5){ //doors
                    if(led){
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::red);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("OPEN");
                    } else {
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::black);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("Closed");
                    }
                }
                else if(i>5 && i<=7){ //smoke
                    if(led){
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::red);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("SMOKE");
                    } else {
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::black);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("No Smoke");
                    }
                }
                else if(i==8){ //FSS alert
                    if(led){
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::red);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("FIRE!");
                    } else {
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::black);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("Off");
                    }
                }
                else if(i==9){ //FSS trouble
                    if(led){
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::black);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("Off");
                    } else {
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::red);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("TROUBLE!");
                    }
                }
                else if(i>9 && i<=14){ //CAM
                    if(led){
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::black);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("Online");
                    } else {
                        palette.setColor(LedLabel_ptr->foregroundRole(), Qt::red);
                        LedLabel_ptr->setPalette(palette);
                        LedLabel_ptr->setText("OFFLINE!");
                    }
                }
                //      power status        TAS power status
                else if((i>14 && i<=23) || (i>23 && i<=30)){
                    if(led){
                        LedLabel_ptr->setPixmap( QPixmap( "/home/cp/Control_GUI/icons/green.png" ) );
                    } else LedLabel_ptr->setPixmap( QPixmap( "/home/cp/Control_GUI/icons/red.png" ) );
                }

                // LedLabel_ptr->setText(led);
            }, &mainWindow
        );
        ledMqttClientArr[i]->start();
    }


    //#####################################
    //########### buttons #################
    //#####################################


    //######### Password entry #########

    QLineEdit* passLine = g->lineEdit;
    QPushButton* loginBtn = g->pushButton_2;
    QPushButton* logoutBtn = g->pushButton;

    QObject::connect(loginBtn, &QPushButton::clicked, [passLine, loginBtn, logoutBtn, tabWidget_ptr]() {
        //enabling tabs if password is valid
        if((passLine->text()).compare(PASSWORD) == 0 || (passLine->text()).compare(CLIENT_PASSWORD) == 0) {
            loginBtn->setDisabled(true);
            logoutBtn->setDisabled(false);

            for (int i = 1; i < 7; ++i){
                if((passLine->text()).compare(CLIENT_PASSWORD) == 0
                    && (i!=2)// && i!=3 && i!=6) //when not a tab with Plots/cams/log we don't enable tab
                ){
                    continue;
                }
                tabWidget_ptr->setTabEnabled(i, true);
            }
        }
        passLine->setText("");
    });

    QObject::connect(logoutBtn, &QPushButton::clicked, [passLine, loginBtn, logoutBtn, tabWidget_ptr]() {
        //disabling tabs
        passLine->setText("");
        loginBtn->setDisabled(false);
        logoutBtn->setDisabled(true);

        for (int i = 1; i < 7; ++i){
            tabWidget_ptr->setTabEnabled(i, false);
        }
    });


    //######### Minimize, exit buttons ##########

    QPushButton* miniBtn = g->pushButton_88;
    QPushButton* CloseBtn = g->pushButton_107;
    QMainWindow* mainWindowPtr = &mainWindow;

    QObject::connect(miniBtn, &QPushButton::clicked, [mainWindowPtr]() {
        mainWindowPtr->showMinimized();
    });

    QObject::connect(CloseBtn, &QPushButton::clicked, []() {
        exit(0);
    });


    //###### Disable/enable control software ########

    QPushButton* softwareBtn[8] = {
        g->pushButton_113, //pwr on
        g->pushButton_106, //off
        g->pushButton_111, //IPMI on
        g->pushButton_110, //off
        g->pushButton_114, //outback on
        g->pushButton_109, //off
        g->pushButton_115, //comap on
        g->pushButton_116  //off
    };

    QString softwareCommand[8] = {
        "expect /home/cp/Control_GUI/scripts/startPOWER.exp &",
        "expect /home/cp/Control_GUI/scripts/stopPOWER.exp &",
        "expect /home/cp/Control_GUI/scripts/startIPMI.exp &",
        "expect /home/cp/Control_GUI/scripts/stopIPMI.exp &",
        "expect /home/cp/Control_GUI/scripts/startOutback.exp &",
        "expect /home/cp/Control_GUI/scripts/stopOutback.exp &",
        "expect /home/cp/Control_GUI/scripts/startComap.exp &",
        "expect /home/cp/Control_GUI/scripts/stopComap.exp &"
    };

    for (int i = 0; i < 8; ++i) {
        QString* commandPtr = &softwareCommand[i];
        QObject::connect(softwareBtn[i], &QPushButton::clicked, [commandPtr]() { 
            system((*commandPtr).toStdString().c_str());
        });
    }



    //###### start/stop Gen ########

    QPushButton* genBtn[2] = {
        g->pushButton_18, //start
        g->pushButton_13  //stop
    };

    QString genCommand[2] = {
        "expect /home/cp/Control_GUI/scripts/startGen.exp &",
        "expect /home/cp/Control_GUI/scripts/stopGen.exp &"
    };

    for (int i = 0; i < 2; ++i) {
        QString* commandPtr = &genCommand[i];
        QObject::connect(genBtn[i], &QPushButton::clicked, [commandPtr]() { 
            system((*commandPtr).toStdString().c_str());
        });
    }


    //###### start/stop fans ########

    QPushButton* fanBtn[6] = {
        g->pushButton_19, //start fanA
        g->pushButton_14, //stop
        g->pushButton_20, //start fanB
        g->pushButton_15, //stop
        g->pushButton_101,//start health fans (PWM)
        g->pushButton_102 //stop
    };

    QString fanTopic[6] = {
        EXFAN1_ONOFF,
        EXFAN1_ONOFF,
        EXFAN2_ONOFF,
        EXFAN2_ONOFF,
        FANS_ONOFF,
        FANS_ONOFF
    };

    for (int i = 0; i < 6; ++i) {
        //QString* topicPtr = &fanTopic[i];
        QObject::connect(fanBtn[i], &QPushButton::clicked, [testClient, i, fanTopic]() {
            QString command, topic = fanTopic[i];
            if(i%2 == 0) {
                command = QString("./testInt %1 1 > /dev/null 2>&1 &").arg(topic);
            } else command = QString("./testInt %1 0 > /dev/null 2>&1 &").arg(topic);
            system(command.toStdString().c_str());

            //Controling 2nd Fan on A-end (AUX2)
            if(i<2) {
                topic = RLY9_ONOFF;
                //QMqttTopicName topic2 = QMqttTopicName(topicStr2);
                if(i%2 == 0) {
                command = QString("./testInt %1 1 > /dev/null 2>&1 &").arg(topic);
            } else command = QString("./testInt %1 0 > /dev/null 2>&1 &").arg(topic);
                system(command.toStdString().c_str());
            }
        });
    }



    //######## Auto TAS boot-up sequence #########

    QPushButton* AutoBootBtn = g->pushButton_6;
    QString bootSeqTopic = IPMI_BOOT;

    QObject::connect(AutoBootBtn, &QPushButton::clicked, [bootSeqTopic]() {
        QString command = QString("bash /home/cp/Control_GUI/scripts/auto_tas_boot/autoBoot.sh %1 %2 > /dev/null 2>&1 &").arg(bootSeqTopic).arg(BOOT_SIGNAL_DURATION_MIN);
        // QString command = QString("bash scripts/auto_tas_boot/autoBoot.sh %1 > /dev/null 2>&1 &").arg(bootSeqTopic); // this will make the script to default to 15min boot timeout

        system(command.toStdString().c_str());
        if(DEBUG)
	qDebug() << "### Started Boot sequence ###";
    });


    //######## 2U power ##########

    QPushButton* simBtn[2] = {
        g->pushButton_3,//start sim
        g->pushButton_52, //stop sim
    };

    QString simTopics[3] = {
        "USB_GPIO/voltage2",
        "2U_DIO/sim_force_en",
        "2U_DIO/sim_force_dis"
    };

    QObject::connect(simBtn[0], &QPushButton::clicked, [simTopics]() { 
        QString command = QString("./testBool %1 1 > /dev/null 2>&1 &").arg(simTopics[1]);
        system(command.toStdString().c_str());
        command = QString("./testBool %1 0 > /dev/null 2>&1 &").arg(simTopics[2]);
        system(command.toStdString().c_str());
        if(DEBUG)
	qDebug() << "### Started SIM ###";
    });

    QObject::connect(simBtn[1], &QPushButton::clicked, [simTopics]() { 
        QString command = QString("./testBool %1 0 > /dev/null 2>&1 &").arg(simTopics[1]);
        system(command.toStdString().c_str());
        command = QString("./testBool %1 1 > /dev/null 2>&1 &").arg(simTopics[2]);
        system(command.toStdString().c_str());
        if(DEBUG)
	qDebug() << "### Stopped SIM ###";
    });

    //######## 2U power ##########

    QPushButton* U2Btn[4] = {
        g->pushButton_21,//start 2U
        g->pushButton_16, //stop
        g->pushButton_8,//start
        g->pushButton_33//stop
    };

    // RLY8_ONOFF
    for (int i = 0; i < 4; ++i) {
        QObject::connect(U2Btn[i], &QPushButton::clicked, [testClient, i]() { 
            QString topic = RLY8_ONOFF, command;
           /* QByteArray data;
            bool temp = true;
            char ch;
            QMqttTopicName topic = QMqttTopicName(topicStr);
            if(i%2 == 0) {
                //publish int 1 to DIO topic
                // data.setNum(1, 10);
                ch = temp;
                data += ch;
            } else {
                //publish int 0 to DIO topic
                // data.setNum(0, 10);
                temp = false;
                ch = temp;
                data += ch;
            }
            testClient->publish(topic, data, 0, false);
	    */
            if(i%2 == 0) {
                    command = QString("./testInt %1 1 > /dev/null 2>&1 &").arg(topic);
            } else command = QString("./testInt %1 0 > /dev/null 2>&1 &").arg(topic);
	    system(command.toStdString().c_str());
        });
    }


    //###### system Power state buttons on 2U-DIO ######

    QPushButton* sysPowBtn[NUM_U3_RLYS] = {
        g->pushButton_7,  //APLX on
        g->pushButton_28, //off
        g->pushButton_9,  //DMA on
        g->pushButton_34, //off
        g->pushButton_10, //L1 on
        g->pushButton_35, //off
        g->pushButton_11, //L2 on
        g->pushButton_36, //off
        g->pushButton_45, //lidar blower on
        g->pushButton_46,  //off
        g->pushButton_68, //3d/rw blower on
        g->pushButton_74,  //off
        U2Btn[0], //3U on
        U2Btn[1], //off
        U2Btn[2], //3U on
        U2Btn[3], //off
    };

    QString sysPowTopics[NUM_U3_RLYS-4] = {
        APLX_ONOFF,
        APLX_ONOFF,
        DMA_ONOFF,
        DMA_ONOFF,
        L1_ONOFF,
        L1_ONOFF,
        L2_ONOFF,
        L2_ONOFF,
        BLWR_ONOFF,
        BLWR_ONOFF,
        VARIPHASE_ONOFF,
        VARIPHASE_ONOFF
    };

    for (int i = 0; i < NUM_U3_RLYS-4; ++i) {
        QObject::connect(sysPowBtn[i], &QPushButton::clicked, [testClient, i, sysPowTopics]() {
            QString topicStr = sysPowTopics[i];
            QByteArray data;
            bool temp = true;
            char ch;
            QMqttTopicName topic = QMqttTopicName(topicStr);

            if(i < 10) {
                if(i%2 == 0) {
                    //publish int 1 to DIO topic
                    // data.setNum(1, 10);
                    ch = temp;
                    data += ch;
                } else {
                    //publish int 0 to DIO topic
                    // data.setNum(0, 10);
                    temp = false;
                    ch = temp;
                    data += ch;
                }
                testClient->publish(topic, data, 0, false);
            }
            else {
                //publishInt
                QString command;
                if(i%2 == 0) {
                    command = QString("./testInt %1 1 > /dev/null 2>&1 &").arg(topicStr);
                } else command = QString("./testInt %1 0 > /dev/null 2>&1 &").arg(topicStr);
                system(command.toStdString().c_str());
            }
        });
    }


    MqttClient* U3RlyMqttClientArr[NUM_U3_RLYS];

    for (int i = 0; i < NUM_U3_RLYS; ++i){
        U3RlyMqttClientArr[i] = new MqttClient(
            U3RlyEndpoint[i], [sysPowBtn, i](void* payload) {
                bool active = *((bool*)payload);
                if(DEBUG)
		qDebug() << "Update LED label with new data: " << active;

                if(i%2 == 0){//ON button
                    if(active){ //green when high

			if(i>NUM_U3_RLYS-5){
				sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: none; }").arg(sysPowBtn[i]->objectName()));
			}
                        else sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: green; }").arg(sysPowBtn[i]->objectName()));
                    }
                    //grey when low
                    else {
			if(i>NUM_U3_RLYS-5){
				sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: green; }").arg(sysPowBtn[i]->objectName()));
			}
			else sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: none; }").arg(sysPowBtn[i]->objectName()));
		    }
                } else { //OFF button
                    if(active){ //grey when high
                        if(i>NUM_U3_RLYS-5){
				sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: red; }").arg(sysPowBtn[i]->objectName()));
			}
			else sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: none; }").arg(sysPowBtn[i]->objectName()));
                    }
                    //red when high
                    else {
			if(i>NUM_U3_RLYS-5){
				sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: none; }").arg(sysPowBtn[i]->objectName()));
			}
			else sysPowBtn[i]->setStyleSheet(QString("#%1 { background-color: red; }").arg(sysPowBtn[i]->objectName()));
		    }
                }

            } , &mainWindow
        );
        U3RlyMqttClientArr[i]->start();
    }


    //###### Linux controllers state change ######
    QString linuxControllerTopics[6] = {
        MCCRST_ONOFF,
        BCCRST_ONOFF,
        MCCHB_ONOFF,
        MCCHB_ONOFF,
        BCCHB_ONOFF,
        BCCHB_ONOFF
    };

    QPushButton* linuxControllerBtn[8] = {
        g->pushButton_47, //mcc auto dio reboot
        g->pushButton_69, //bcc auto dio reboot
        g->pushButton_49, //mcc en dio HB
        g->pushButton_71, //mcc dis dio HB
        g->pushButton_50, //bcc en dio HB
        g->pushButton_72, //bcc dis dio HB
        g->pushButton_48, //mcc shutdown
        g->pushButton_70  //bcc shutdown
    };

    QString controllerShutdownCommand[2] = {
        "expect /home/cp/Control_GUI/scripts/LinuxShutdown.exp 10.2.0.100 > /dev/null 2>&1 &",
        "expect /home/cp/Control_GUI/scripts/LinuxShutdown.exp 10.2.0.110 > /dev/null 2>&1 &"
    };

    //issuing automatic reboot of controllers
    for (int i = 0; i < 2; ++i) {
        QObject::connect(linuxControllerBtn[i], &QPushButton::clicked, [testClient, i, linuxControllerTopics]() {
            QString topicStr = linuxControllerTopics[i];
            QString command;
	    command = QString("./testInt %1 1 > /dev/null 2>&1 &").arg(topicStr);
            system(command.toStdString().c_str());
        });
    }

    //issuing DIO heartbeat en/dis
    for (int i = 2; i < 6; ++i) {
        QObject::connect(linuxControllerBtn[i], &QPushButton::clicked, [testClient, i, linuxControllerTopics]() {
            QString topicStr = linuxControllerTopics[i];
            QString command;
            if(i%2 == 0) {
                // publish int 1 to DIO topic
                command = QString("./testInt %1 1 > /dev/null 2>&1 &").arg(topicStr);
            } else {
                // publish int 0 to DIO topic
                command = QString("./testInt %1 0 > /dev/null 2>&1 &").arg(topicStr);
            }
            system(command.toStdString().c_str());
        });
    }

    //issuing linux shutdown via ssh
    for (int i = 0; i < 2; ++i) {
        QString* commandPtr = &controllerShutdownCommand[i];
        QObject::connect(linuxControllerBtn[6+i], &QPushButton::clicked, [commandPtr]() { 
            system((*commandPtr).toStdString().c_str());
        });
    }



    //issuing a new mate3 max charge voltage
    QSpinBox* chargeAmpSpinBtn = g->spinBox;
    QPushButton* publishAmpsBtn = g->pushButton_78;
    QString chargeAmpTopic = "MODBUS/inverter_new_max_amps";

    QObject::connect(publishAmpsBtn, &QPushButton::clicked, [testClient, chargeAmpTopic, chargeAmpSpinBtn]() {
        QString topicStr = chargeAmpTopic;
        QString command;
        command = QString("./testInt %1 %2 > /dev/null 2>&1 &").arg(topicStr).arg(chargeAmpSpinBtn->value());
        system(command.toStdString().c_str());
    });



    //#### shore-mode gen-mode ####
    QPushButton* powerModeBtn[2] = {
        g->pushButton_51,
        g->pushButton_73,
    };


    for (int i = 0; i < 8; ++i) {
        QString* commandPtr = &softwareCommand[i];
        QObject::connect(softwareBtn[i], &QPushButton::clicked, [commandPtr]() { 
            system((*commandPtr).toStdString().c_str());
        });
    }



    //shore power mode
    QObject::connect(powerModeBtn[0], &QPushButton::clicked, [testClient, chargeAmpTopic, softwareCommand]() {
        QString topicStr = chargeAmpTopic;
        QString command;

        //disable POWER control loop
        command = softwareCommand[1];
        system(command.toStdString().c_str());

        //set charge amps to 2A
        command = QString("./testInt %1 %2 > /dev/null 2>&1 &").arg(topicStr).arg(2);
        system(command.toStdString().c_str());
    });

    //Gen power mode
    QObject::connect(powerModeBtn[1], &QPushButton::clicked, [testClient, chargeAmpTopic, softwareCommand]() {
        QString topicStr = chargeAmpTopic;
        QString command;

        //enable POWER control loop
        command = softwareCommand[0];
        system(command.toStdString().c_str());

        //set charge amps to 4A
        command = QString("./testInt %1 %2 > /dev/null 2>&1 &").arg(topicStr).arg(4);
        system(command.toStdString().c_str());
    });



    //health DIO relay state control
    QPushButton* dioControllerBtn[20] = {
        g->pushButton_84,  //rly1 on
        g->pushButton_90,  //off
        g->pushButton_82,  //rly2 on
        g->pushButton_91,  //off
        g->pushButton_86,  //rly3 on
        g->pushButton_92,  //off
        g->pushButton_80,  //rly4 on 
        g->pushButton_93,  //off
        g->pushButton_81,  //rly5 on
        g->pushButton_94,  //off
        g->pushButton_85,  //rly6 on
        g->pushButton_95,  //off
        g->pushButton_83,  //rly7 on
        g->pushButton_96,  //off
        g->pushButton_79,  //rly8 on
        g->pushButton_97,  //off
        g->pushButton_87,  //rly9 on
        g->pushButton_98,  //rly off
        g->pushButton_99,  //rly10 on
        g->pushButton_100  //off
    };


    QString dioControllerTopics[20] = {
        RLY1_ONOFF,
        RLY1_ONOFF,
        RLY2_ONOFF,
        RLY2_ONOFF,
        RLY3_ONOFF,
        RLY3_ONOFF,
        RLY4_ONOFF,
        RLY4_ONOFF,
        RLY5_ONOFF,
        RLY5_ONOFF,
        RLY6_ONOFF,
        RLY6_ONOFF,
        RLY7_ONOFF,
        RLY7_ONOFF,
        RLY8_ONOFF,
        RLY8_ONOFF,
        RLY9_ONOFF,
        RLY9_ONOFF,
        RLY10_ONOFF,
        RLY10_ONOFF
    };


    for (int i = 0; i < NUM_DIO_RLYS; ++i) {
        QObject::connect(dioControllerBtn[i], &QPushButton::clicked, [testClient, i, dioControllerTopics]() {
            QString topicStr = dioControllerTopics[i];
            QString command;
            if(i%2 == 0) {
                // publish int 1 to DIO topic
                command = QString("./testInt %1 1 > /dev/null 2>&1 &").arg(topicStr);
            } else {
                // publish int 0 to DIO topic
                command = QString("./testInt %1 0 > /dev/null 2>&1 &").arg(topicStr);
            }
            system(command.toStdString().c_str());
        });
    }

    MqttClient* dioRlyMqttClientArr[NUM_DIO_RLYS];

    for (int i = 0; i < NUM_DIO_RLYS; ++i){
        dioRlyMqttClientArr[i] = new MqttClient(
            dioRlyEndpoint[i], [dioControllerBtn, i](void* payload) {
                bool active = *((bool*)payload);
                if(DEBUG)
		qDebug() << "Update LED label with new data: " << active;

                if(i%2 == 0){//ON button
                    if(active){ //green when high
                        dioControllerBtn[i]->setStyleSheet(QString("#%1 { background-color: green; }").arg(dioControllerBtn[i]->objectName()));
                    }
                    //grey when low
                    else dioControllerBtn[i]->setStyleSheet(QString("#%1 { background-color: none; }").arg(dioControllerBtn[i]->objectName()));

                } else { //OFF button
                    if(active){ //grey when high
                        dioControllerBtn[i]->setStyleSheet(QString("#%1 { background-color: none; }").arg(dioControllerBtn[i]->objectName()));
                    }
                    //red when high
                    else dioControllerBtn[i]->setStyleSheet(QString("#%1 { background-color: red; }").arg(dioControllerBtn[i]->objectName()));
                }

            } , &mainWindow
        );
        dioRlyMqttClientArr[i]->start();
    }




    //###### enclosure action duration adjust buttons ########

    QPushButton* enclAdjBtn[4] = {
        g->pushButton_26, // img +5
        g->pushButton_27, // img -5
        g->pushButton_32, // li +5
        g->pushButton_30  // li -5
    };

    for (int i = 0; i < 4; ++i) {
        QObject::connect(enclAdjBtn[i], &QPushButton::clicked, [i]() {
            switch(i) {
                case 0:
                    if(IMG_action_duration_time <= 25) {
                        IMG_action_duration_time += 5;
                        if(DEBUG)
			qDebug() << QString("Note: IMG enclosure actuation time adjusted to: %1 seconds.").arg(IMG_action_duration_time);
                    } else if(DEBUG)qDebug() << QString("Note: IMG enclosure actuation time can't go above: %1 seconds.").arg(IMG_action_duration_time);
                    break;
                case 1:
                    if(IMG_action_duration_time >= 15) {
                        IMG_action_duration_time -= 5;
                        if(DEBUG)
			qDebug() << QString("Note: IMG enclosure actuation time adjusted to: %1 seconds.").arg(IMG_action_duration_time);
                    } else if(DEBUG)qDebug() << QString("Note: IMG enclosure actuation time can't go lower than: %1 seconds.").arg(IMG_action_duration_time);
                    break;
                case 2:
                    if(LIDAR_action_duration_time <= 15) {
                        LIDAR_action_duration_time += 5;
                        if(DEBUG)
			qDebug() << QString("Note: LIDAR enclosure actuation time adjusted to: %1 seconds.").arg(LIDAR_action_duration_time);
                    } else if(DEBUG)qDebug() << QString("Note: LIDAR enclosure actuation time can't go above: %1 seconds.").arg(LIDAR_action_duration_time);
                    break;
                case 3:
                    if(LIDAR_action_duration_time >= 10) {
                        LIDAR_action_duration_time -= 5;
                        if(DEBUG)
			qDebug() << QString("Note: LIDAR enclosure actuation time adjusted to: %1 seconds.").arg(LIDAR_action_duration_time);
                    } else if(DEBUG)qDebug() << QString("Note: LIDAR enclosure actuation time can't go lower than: %1 seconds.").arg(LIDAR_action_duration_time);
                    break;
            }
            //TODO: implement backup mechanism
            // backupActionTimes(IMG_action_duration_time, LIDAR_action_duration_time);
        });
    }


    //###### enclosure buttons ########

    QPushButton* enclBtn[4] = {
        g->pushButton_17, // op li
        g->pushButton_22, // cl li
        g->pushButton_23, // op img
        g->pushButton_24  // cl img
    };
    QPushButton* enclBtn2[4] = {
        g->pushButton_29, // op li
        g->pushButton_31, // cl li
        g->pushButton_12, // op img
        g->pushButton_25  // cl img
    };

    QString liEnclTopics[4] = {
        "USB_GPIO/DIO2_state",
        "USB_GPIO/DIO3_state",
        "2U_DIO/lidar_open",
        "2U_DIO/lidar_close"
    };

    QString imgEnclTopics[4] = {
        "USB_GPIO/DIO6_state",
        "USB_GPIO/DIO7_state",
        "2U_DIO/cam_open",
        "2U_DIO/cam_close"
    };

    for (int i = 0; i < 4; ++i) {

        QObject::connect(enclBtn[i], &QPushButton::clicked, [i, liEnclTopics, imgEnclTopics]() {
            QString command;
            switch(i) {
                case 0:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 0 1 0 %5 > /dev/null 2>&1 &").arg(liEnclTopics[0]).arg(liEnclTopics[1]).arg(liEnclTopics[2]).arg(liEnclTopics[3]).arg(LIDAR_action_duration_time);
                    qDebug() << "Lidar OPEN start";
                    system(command.toStdString().c_str());
                    qDebug() << "Lidar OPEN complete";
                    break;
                }
                case 1:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 1 0 1 %5 > /dev/null 2>&1 &").arg(liEnclTopics[0]).arg(liEnclTopics[1]).arg(liEnclTopics[2]).arg(liEnclTopics[3]).arg(LIDAR_action_duration_time);
                    qDebug() << "Lidar CLOSE start";
                    system(command.toStdString().c_str());
                    qDebug() << "Lidar CLOSE complete";
                    break;
                }
                case 2:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 0 0 1 %5 > /dev/null 2>&1 &").arg(imgEnclTopics[0]).arg(imgEnclTopics[1]).arg(imgEnclTopics[2]).arg(imgEnclTopics[3]).arg(IMG_action_duration_time);
                    qDebug() << "IMG OPEN start";
                    system(command.toStdString().c_str());
                    qDebug() << "IMG OPEN complete";
                    break;
                }
                case 3:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 1 1 0 %5 > /dev/null 2>&1 &").arg(imgEnclTopics[0]).arg(imgEnclTopics[1]).arg(imgEnclTopics[2]).arg(imgEnclTopics[3]).arg(IMG_action_duration_time);
                    qDebug() << "IMG CLOSE start";
                    system(command.toStdString().c_str());
                    qDebug() << "IMG CLOSE complete";
                    break;
                }
            }
        });

        QObject::connect(enclBtn2[i], &QPushButton::clicked, [i, liEnclTopics, imgEnclTopics]() {
            QString command;
            switch(i) {
                case 0:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 0 1 0 %5 > /dev/null 2>&1 &").arg(liEnclTopics[0]).arg(liEnclTopics[1]).arg(liEnclTopics[2]).arg(liEnclTopics[3]).arg(LIDAR_action_duration_time);
                    qDebug() << "Lidar OPEN start";
                    system(command.toStdString().c_str());
                    qDebug() << "Lidar OPEN complete";
                    break;
                }
                case 1:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 1 0 1 %5 > /dev/null 2>&1 &").arg(liEnclTopics[0]).arg(liEnclTopics[1]).arg(liEnclTopics[2]).arg(liEnclTopics[3]).arg(LIDAR_action_duration_time);
                    qDebug() << "Lidar CLOSE start";
                    system(command.toStdString().c_str());
                    qDebug() << "Lidar CLOSE complete";
                    break;
                }
                case 2:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 0 0 1 %5 > /dev/null 2>&1 &").arg(imgEnclTopics[0]).arg(imgEnclTopics[1]).arg(imgEnclTopics[2]).arg(imgEnclTopics[3]).arg(IMG_action_duration_time);
                    qDebug() << "IMG OPEN start";
                    system(command.toStdString().c_str());
                    qDebug() << "IMG OPEN complete";
                    break;
                }
                case 3:
                {
                    command = QString("bash scripts/control_enclosure/controlEnc.sh %1 %2 %3 %4 0 1 1 0 %5 > /dev/null 2>&1 &").arg(imgEnclTopics[0]).arg(imgEnclTopics[1]).arg(imgEnclTopics[2]).arg(imgEnclTopics[3]).arg(IMG_action_duration_time);
                    qDebug() << "IMG CLOSE start";
                    system(command.toStdString().c_str());
                    qDebug() << "IMG CLOSE complete";
                    break;
                }
            }
        });
    }


    //###########################################################################################


    //######### Navigation #############
    MqttClient* NavMqttClientArr[NUM_NAV];

    QLabel* NavLabeles[NUM_NAV] = {
        g->label_57,
        g->label_60,
        g->label_192
    };

    for (int i = 0; i < NUM_NAV; ++i){
        QLabel* NavLabel_ptr = NavLabeles[i];
        NavMqttClientArr[i] = new MqttClient(
            navEndpoint[i], [NavLabel_ptr](void* payload) {

                if(DEBUG)
		qDebug() << "Update nav label with new data: " << *((double*)payload);
                QString labelStr = QString("%1").arg(*((double*)payload));
                NavLabel_ptr->setText(labelStr);
            }, &mainWindow
        );
        NavMqttClientArr[i]->start();
    }


    //######### GIS #############
    MqttClient* gisMqttClientArr[NUM_GIS];

    QLabel* gisLabeles[NUM_GIS] = {
        g->label_62,
        g->label_190,
        g->label_66,
        g->label_67
    };

    for (int i = 0; i < NUM_GIS; ++i){
        QLabel* gisLabel_ptr = gisLabeles[i];
        gisMqttClientArr[i] = new MqttClient(
            gisEndpoint[i], [gisLabel_ptr](void* payload) {

                // if( ((std::string*)payload) == NULL) {
                //     qDebug() << "Failed to parse string";
                //     return;
                // }
                QString labelStr = QString::fromStdString((*((std::string*)payload)).c_str());
                if(DEBUG)
		qDebug() << "Update gis label with new data: " << labelStr;
                gisLabel_ptr->setText(labelStr);

            }, &mainWindow
        );
        gisMqttClientArr[i]->start();
    }




    //######### Progress bars ##########

    MqttClient* BarMqttClientArr[NUM_BARS];

    QProgressBar* progressBars[NUM_BARS] = {
        g->progressBar,
        g->progressBar_9,
        g->progressBar_10,
        g->progressBar_8
    };

    for (int i = 0; i < NUM_BARS; ++i){
        QProgressBar* progressBar_ptr = progressBars[i];

        BarMqttClientArr[i] = new MqttClient(
            barEndpoint[i], [progressBar_ptr](void* payload) {
		int payload_parsed = 0;
		if((int*)payload == NULL && !std::isnan(*(double*)payload) && *(double*)payload < 1000){
		    payload_parsed = (int)(*((double*)payload));
		    if(DEBUG)
			qDebug() << "Failed to parser Int, parsing as Double";
                } else if((int*)payload == NULL && *(int*)payload != 0){
                    if(DEBUG)
			qDebug() << "Failed to parser Int";
                    return;
		    //payload_parsed = (int)(*((double*)payload));
                }
		else payload_parsed = *((int*)payload);
                if(DEBUG)
			qDebug() << QString("Update bar widget with new data: %1").arg(payload_parsed);
                progressBar_ptr->setValue(payload_parsed);
            }, &mainWindow
        );
        BarMqttClientArr[i]->start();
    }


    //####### TAS storage bars ########
    MqttClient* TASBarMqttClientArr[NUM_TAS_BARS];

    QLabel* tasStorageLabeles[NUM_TAS_BARS] = {
        g->label_152,
        g->label_149,
        g->label_161,
        g->label_155,
        g->label_158,
        g->label_164
    };

    QProgressBar* TASprogressBars[NUM_TAS_BARS] = {
        g->progressBar_2,
        g->progressBar_4,
        g->progressBar_6,
        g->progressBar_3,
        g->progressBar_5,
        g->progressBar_7
    };

    for (int i = 0; i < NUM_TAS_BARS; ++i){
        QProgressBar* progressBar_ptr = TASprogressBars[i];
        QLabel* tasStorageLabel_ptr = tasStorageLabeles[i];
        TASBarMqttClientArr[i] = new MqttClient(
            TASbarEndpoint[i], [progressBar_ptr, tasStorageLabel_ptr](void* payload) {

                if(DEBUG)
		qDebug() << "Update TAS storage bar widget with new data: " << *((double*)payload);
                progressBar_ptr->setValue(*((double*)payload) );
                QString str;
                str.setNum(*((double*)payload), 'f', 1);
                tasStorageLabel_ptr->setText(str);

            }, &mainWindow
        );
        TASBarMqttClientArr[i]->start();
    }


    //###### TAS num reboots #######

    MqttClient* TASrebootMqttClientArr[NUM_REBOOT];

    QLabel* tasRebootsLabeles[NUM_REBOOT] = {
        g->label_78,
        g->label_90,
        g->label_95,
        g->label_100,
        g->label_105
    };    

    for (int i = 0; i < NUM_REBOOT; ++i){
        QLabel* label_ptr = tasRebootsLabeles[i];
        TASrebootMqttClientArr[i] = new MqttClient(
            rebootEndpoint[i], [label_ptr](void* payload) {
                if((int*)payload == NULL && *(int*)payload != 0){
                    if(DEBUG)
			qDebug() << "Failed to parser Int";
                    return;
                }
                if(DEBUG)
		qDebug() << "Update tas reboot label with new data: " << *((int*)payload);
                QString str;
                str.setNum(*((int*)payload), 10);
                label_ptr->setText(str);
            }, &mainWindow
        );
        TASrebootMqttClientArr[i]->start();
    }


    //#### Controls to issue IPMI comaands ####
    QPushButton* powerServerBtn[5][4] = {
        {
            g->pushButton_37, // 3D On
            g->pushButton_40, //softOff
            g->pushButton_39, //hardOff
            g->pushButton_38  //cycle
        },
        {
            g->pushButton_41, //Lidar on
            g->pushButton_44, //
            g->pushButton_43, //
            g->pushButton_42  //
        },
        {
            g->pushButton_53, //NAS on
            g->pushButton_56, //soft (deactivated)
            g->pushButton_55, //hard
            g->pushButton_54  //cycle
        },
        {
            g->pushButton_57, //IMG on
            g->pushButton_60, //
            g->pushButton_59, //
            g->pushButton_58  //
        },
        {
            g->pushButton_61, //RW
            g->pushButton_64, //
            g->pushButton_63, //
            g->pushButton_62  //
        }
    };

    QString ServerStateCommand[5] = {
        "servOn.exp",
        "servOff.exp",
        "ServHard.exp",
        "servCycle.exp"
    };

    QString Server_id_param[5] = {
        "3d",
        "li",
        "nas",
        "img",
        "rw"
    };

    QString ServerStateCommandPath = "expect /home/cp/Control_GUI/scripts/";

    for (int i=0; i<5; ++i){
	    for (int j=0; j<4; ++j){
		//prevents the NAS soft-shutdown commands from running
		if(i==2 && j==1) {
		g->pushButton_56->setDisabled(true);
		continue;
	    }

            QObject::connect(powerServerBtn[i][j], &QPushButton::clicked, [i, j, Server_id_param, ServerStateCommand, ServerStateCommandPath](){
                QString command2;
		// /dev/null 2>&1 &
                command2 = QString("%1%2 %3 > /home/cp/IPMItest.txt &").arg(ServerStateCommandPath).arg(ServerStateCommand[j]).arg(Server_id_param[i]);
                system(command2.toStdString().c_str());
                qDebug() << "################### Sending IPMI command: " << command2 << "####################";
            });
        }
    }



    //########## GAUGES ############

    MqttClient* gaugeMqttClientArr[NUM_GAUGES];
    QcNeedleItem* needleItemArr[NUM_GAUGES];


    //###################################
    //### Define widget ID in ui file ###
    //###################################
    QcGaugeWidget* gaugeArr[NUM_GAUGES] = {
        new QcGaugeWidget(g->widget_32),
        new QcGaugeWidget(g->widget_33),
        new QcGaugeWidget(g->widget_4),
        new QcGaugeWidget(g->widget_5),
        new QcGaugeWidget(g->widget_6),
        new QcGaugeWidget(g->widget_7)
    };

    QHBoxLayout gaugeLayoutArr[NUM_GAUGES] = {
        QHBoxLayout(g->widget_32),
        QHBoxLayout(g->widget_33),
        QHBoxLayout(g->widget_4),
        QHBoxLayout(g->widget_5),
        QHBoxLayout(g->widget_6),
        QHBoxLayout(g->widget_7)
    };

    const std::string gaugeLabels[NUM_GAUGES] = {
        "Batt (V)",
        "Solar (W)",
        "RPM",
        "Fuel (%)",
        "Charge (KW)",
        "Load (KW)"
    };


    const QPair<int, int>* gaugeValuesRangeArr[NUM_GAUGES];

    // for( int i = 0; i<NUM_GAUGES; ++i) {
    //     gaugeValuesRangeArr[i] = new QPair<int, int>(0, 2000);
    // };
    gaugeValuesRangeArr[0] = new QPair<int, int>(0, 60); //batt
    gaugeValuesRangeArr[1] = new QPair<int, int>(0, 3000); //RPM/solar
    gaugeValuesRangeArr[2] = new QPair<int, int>(0, 3000); //RPM/solar
    gaugeValuesRangeArr[3] = new QPair<int, int>(0, 100); //fuel
    gaugeValuesRangeArr[4] = new QPair<int, int>(0, 20); //Inv charge KW
    gaugeValuesRangeArr[5] = new QPair<int, int>(0, 20); //Inv load KW


    const int gaugeDenumeratorArr[NUM_GAUGES] = {
        1, //batt/fuel
        1000, //rpm/solar
        1000, //rpm/solar
        1, //batt/fuel
        5, //charge/load
        5 //charge/load
    };


    for(int i = 0; i < NUM_GAUGES; ++i) {
        needleItemArr[i] = setupBasicGaugeWidget(*gaugeArr[i], *gaugeValuesRangeArr[i], gaugeDenumeratorArr[i], gaugeLabels[i]);
        gaugeLayoutArr[i].addWidget(gaugeArr[i], /* stretch factor */ 1);

        QcNeedleItem* needleItem_ptr = needleItemArr[i];
        const int* denum_ptr = &gaugeDenumeratorArr[i];

        gaugeMqttClientArr[i] = new MqttClient(
            gaugeEndpoint[i], [needleItem_ptr, denum_ptr](void* payload) {
                // if(dynamic_cast<double*>(payload) == NULL){
                //     qDebug() << "Failed to parser Double";
                //     return;
                // }

                if(DEBUG)
		qDebug() << "Update gauge widget with new data: " << *((double*)payload);
                needleItem_ptr->setCurrentValue(*((double*)payload) / *denum_ptr);
            }, &mainWindow
        );
        gaugeMqttClientArr[i]->start();
    }


    //################ PLOTS ###############

    QComboBox* comboBoxArr[NUM_PLOTS] = {
        g->comboBox_8,
        g->comboBox_9,
        g->comboBox_11,
        g->comboBox_12,
        g->comboBox_15,
        g->comboBox_14,
        g->comboBox_13,
        g->comboBox,
        g->comboBox_4,
        g->comboBox_10,
        g->comboBox_5,
        g->comboBox_2,
        g->comboBox_6,
        g->comboBox_3,
        g->comboBox_7,
        g->comboBox_16,
        g->comboBox_17,
        g->comboBox_20,
        g->comboBox_22,
        g->comboBox_19,
        g->comboBox_23,
        g->comboBox_21,
        g->comboBox_24,
        g->comboBox_18,
        g->comboBox_25,
        g->comboBox_26,
        g->comboBox_27
    };


    const std::string plotLabels[NUM_PLOT_OPTIONS] = {
        "Inv. Batt (V)",
        "Inv. Charge lim. (A)",
        "Bat. Monitor Output (KW)",
        "Bat. Monitor Input (KW)",
        "Master Inv mode",
        "Solar (W)",
        "Solar (V)",
        "Gen Bat. (V)",
        "Gen RPM",
        "Gen Load (KW)",
        "Fuel (%)",
        "Gen Total Runtime (Hrs)",
        "Gen Maint. (Hrs)",
        "Bat. Monitor temp (C)",
        "Car Lights",
        "Mandoor Prox state",
        "Slidingdoor Prox state",
        "FanA state",
        "FanB state",
        "Health Fans state",
        "Smoke-A",
        "Smoke-B",
        "FSS Fire",
        "FSS Trbl.",
        "Speed (KM/h)",
        "Int. AEnd temp (C)",
        "Int. AEnd hum (%)",
        "Int. BEnd temp (C)",
        "Int. BEnd hum (%)",
        "Ext. AEnd temp (C)",
        "Ext. AEnd hum (%)",
        "Ext. BEnd temp (C)",
        "Ext. BEnd hum (%)",
        "Weather temp (C)",
        "DIO Int. temp (C)",
        "Rack-In temp (C)",
        "Rack-In hum (%)",
        "Rack-Out temp (C)",
        "Rack-Out hum (%)",
        "Precip. inten. (mm/h)",
        "R-Lidar temp (C)",
        "R-Lidar hum (%)",
        "R-Lidar temp (C)",
        "R-Lidar hum (%)",
        "R-3DTAS temp (C)",
        "R-3DTAS hum (%)",
        "L-3DTAS temp (C)",
        "L-3DTAS hum (%)",
        "LF-RW temp (C)",
        "LF-RW hum (%)",
        "LG-RW temp (C)",
        "LG-RW hum (%)",
        "RF-RW temp (C)",
        "RF-RW hum (%)",
        "RG-RW temp (C)",
        "RG-RW hum (%)"
    };

    int plotDenumeratorArr[NUM_PLOT_OPTIONS] = {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    };

    QCustomPlot* plotsArr[NUM_PLOTS] = {
        new QCustomPlot(g->widget),
        new QCustomPlot(g->widget_24),
        new QCustomPlot(g->widget_25),
        new QCustomPlot(g->widget_3),
        new QCustomPlot(g->widget_22),
        new QCustomPlot(g->widget_2),
        new QCustomPlot(g->widget_27),
        new QCustomPlot(g->widget_8),
        new QCustomPlot(g->widget_12),
        new QCustomPlot(g->widget_9),
        new QCustomPlot(g->widget_13),
        new QCustomPlot(g->widget_10),
        new QCustomPlot(g->widget_14),
        new QCustomPlot(g->widget_16),
        new QCustomPlot(g->widget_15),
        new QCustomPlot(g->widget_28),
        new QCustomPlot(g->widget_29),
        new QCustomPlot(g->widget_11),
        new QCustomPlot(g->widget_20),
        new QCustomPlot(g->widget_17),
        new QCustomPlot(g->widget_23),
        new QCustomPlot(g->widget_18),
        new QCustomPlot(g->widget_26),
        new QCustomPlot(g->widget_19),
        new QCustomPlot(g->widget_21),
        new QCustomPlot(g->widget_30),
        new QCustomPlot(g->widget_31)
    };


    QHBoxLayout plotLayoutArr[NUM_PLOTS] = {
        QHBoxLayout(g->widget),
        QHBoxLayout(g->widget_24),
        QHBoxLayout(g->widget_25),
        QHBoxLayout(g->widget_3),
        QHBoxLayout(g->widget_22),
        QHBoxLayout(g->widget_2),
        QHBoxLayout(g->widget_27),
        QHBoxLayout(g->widget_8),
        QHBoxLayout(g->widget_12),
        QHBoxLayout(g->widget_9),
        QHBoxLayout(g->widget_13),
        QHBoxLayout(g->widget_10),
        QHBoxLayout(g->widget_14),
        QHBoxLayout(g->widget_16),
        QHBoxLayout(g->widget_15),
        QHBoxLayout(g->widget_28),
        QHBoxLayout(g->widget_29),
        QHBoxLayout(g->widget_11),
        QHBoxLayout(g->widget_20),
        QHBoxLayout(g->widget_17),
        QHBoxLayout(g->widget_23),
        QHBoxLayout(g->widget_18),
        QHBoxLayout(g->widget_26),
        QHBoxLayout(g->widget_19),
        QHBoxLayout(g->widget_21),
        QHBoxLayout(g->widget_30),
        QHBoxLayout(g->widget_31)
    };

    const QPair<int, int>* plotValuesRangeArr[NUM_PLOT_OPTIONS];

    //for( int i = 0; i < NUM_PLOT_OPTIONS; ++i) {
    //    plotValuesRangeArr[i] = new QPair<int, int>(0, 2000);
    //};

    //"Inv. Batt (V)",
    plotValuesRangeArr[0] = new QPair<int, int>(0, 60);
    //"Inv. Charge lim. (A)",
    plotValuesRangeArr[1] = new QPair<int, int>(0, 12);
    // "Bat. Monitor Output (KW)",
    plotValuesRangeArr[2] = new QPair<int, int>(0, 13);
    // "Bat. Monitor Input (KW)",
    plotValuesRangeArr[3] = new QPair<int, int>(0, 10);
    // "Master Inv mode",
    plotValuesRangeArr[4] = new QPair<int, int>(0, 15);
    // "Solar (W)",
    plotValuesRangeArr[5] = new QPair<int, int>(0, 5000);
    // "Solar (V)",
    plotValuesRangeArr[6] = new QPair<int, int>(0, 1000);
    // "Gen Bat. (V)",
    plotValuesRangeArr[7] = new QPair<int, int>(0, 24);
    // "Gen RPM",
    plotValuesRangeArr[8] = new QPair<int, int>(0, 3000);
    // "Gen Load (KW)",
    plotValuesRangeArr[9] = new QPair<int, int>(0, 13);
    // "Fuel (%)",
    plotValuesRangeArr[10] = new QPair<int, int>(0, 100);
    // "Batt (V)",
    plotValuesRangeArr[11] = new QPair<int, int>(0, 60);
    // "Gen Total Runtime (Hrs)",
    plotValuesRangeArr[12] = new QPair<int, int>(0, 65536);
    // "Gen Maint. (Hrs)",
    plotValuesRangeArr[13] = new QPair<int, int>(-700, 700);
    // "Bat. Monitor temp (C)",
    plotValuesRangeArr[14] = new QPair<int, int>(-40, 40);
    // "Car Lights",
    plotValuesRangeArr[15] = new QPair<int, int>(-1, 2);
    // "Mandoor Prox state",
    plotValuesRangeArr[16] = new QPair<int, int>(-1, 2);
    // "Slidingdoor Prox state",
    plotValuesRangeArr[17] = new QPair<int, int>(-1, 2);
    // "FanA state",
    plotValuesRangeArr[18] = new QPair<int, int>(-1, 2);
    // "FanB state",
    plotValuesRangeArr[19] = new QPair<int, int>(-1, 2);
    // "Health Fans state",
    plotValuesRangeArr[20] = new QPair<int, int>(-1, 2);
    // "Smoke-A",
    plotValuesRangeArr[21] = new QPair<int, int>(-1, 2);
    // "Smoke-B",
    plotValuesRangeArr[22] = new QPair<int, int>(-1, 2);
    // "FSS Fire",
    plotValuesRangeArr[23] = new QPair<int, int>(-1, 2);
    // "FSS Trbl.",
    plotValuesRangeArr[24] = new QPair<int, int>(-1, 2);
    // "Speed (KM/h)",
    plotValuesRangeArr[25] = new QPair<int, int>(0, 200);
    // "Int. AEnd temp (C)",
    plotValuesRangeArr[26] = new QPair<int, int>(-40, 40);
    // "Int. AEnd hum (%)",
    plotValuesRangeArr[27] = new QPair<int, int>(0, 100);
    // "Int. BEnd temp (C)",
    plotValuesRangeArr[28] = new QPair<int, int>(-40, 40);
    // "Int. BEnd hum (%)",
    plotValuesRangeArr[29] = new QPair<int, int>(0, 100);
    // "Ext. AEnd temp (C)",
    plotValuesRangeArr[30] = new QPair<int, int>(-40, 40);
    // "Ext. AEnd hum (%)",
    plotValuesRangeArr[31] = new QPair<int, int>(0, 100);
    // "Ext. BEnd temp (C)",
    plotValuesRangeArr[32] = new QPair<int, int>(-40, 40);
    // "Ext. BEnd hum (%)",
    plotValuesRangeArr[33] = new QPair<int, int>(0, 100);
    // "Weather temp (C)",
    plotValuesRangeArr[34] = new QPair<int, int>(-40, 40);
    // "DIO Int. temp (C)",
    plotValuesRangeArr[35] = new QPair<int, int>(-40, 40);
    // "Rack-In temp (C)",
    plotValuesRangeArr[36] = new QPair<int, int>(-40, 40);
    // "Rack-In hum (%)",
    plotValuesRangeArr[37] = new QPair<int, int>(0, 100);
    // "Rack-Out temp (C)",
    plotValuesRangeArr[38] = new QPair<int, int>(-40, 40);
    // "Rack-Out hum (%)",
    plotValuesRangeArr[39] = new QPair<int, int>(0, 100);
    // "Precip. inten. (mm/h)",
    plotValuesRangeArr[40] = new QPair<int, int>(0, 50);
    // "R-Lidar temp (C)",
    plotValuesRangeArr[41] = new QPair<int, int>(-40, 40);
    // "R-Lidar hum (%)",
    plotValuesRangeArr[42] = new QPair<int, int>(0, 100);
    // "R-Lidar temp (C)",
    plotValuesRangeArr[43] = new QPair<int, int>(-40, 40);
    // "R-Lidar hum (%)",
    plotValuesRangeArr[44] = new QPair<int, int>(0, 100);
    // "R-3DTAS temp (C)",
    plotValuesRangeArr[45] = new QPair<int, int>(-40, 40);
    // "R-3DTAS hum (%)",
    plotValuesRangeArr[46] = new QPair<int, int>(0, 100);
    // "L-3DTAS temp (C)",
    plotValuesRangeArr[47] = new QPair<int, int>(-40, 40);
    // "L-3DTAS hum (%)",
    plotValuesRangeArr[48] = new QPair<int, int>(0, 100);
    // "LF-RW temp (C)",
    plotValuesRangeArr[49] = new QPair<int, int>(-40, 40);
    // "LF-RW hum (%)",
    plotValuesRangeArr[50] = new QPair<int, int>(0, 100);
    // "LG-RW temp (C)",
    plotValuesRangeArr[51] = new QPair<int, int>(-40, 40);
    // "LG-RW hum (%)",
    plotValuesRangeArr[52] = new QPair<int, int>(0, 100);
    // "RF-RW temp (C)",
    plotValuesRangeArr[53] = new QPair<int, int>(-40, 40);
    // "RF-RW hum (%)",
    plotValuesRangeArr[54] = new QPair<int, int>(0, 100);
    // "RG-RW temp (C)",
    plotValuesRangeArr[55] = new QPair<int, int>(-40, 40);
    // "RG-RW hum (%)"
    plotValuesRangeArr[56] = new QPair<int, int>(0, 100);






    // QCustomPlot plotWidget(g->widget);
    // QHBoxLayout layout(g->widget);

    // setupPlotWidget(plotWidget, *gaugeValuesRangeArr[0], plotDenumeratorArr[0]);
    // layout.addWidget(&plotWidget, /* stretch factor */ 1);

    // MqttClient* plotMqttClientArr[NUM_PLOTS];


    const QTime timeStart = QTime::fromMSecsSinceStartOfDay(0); //QTime::currentTime();


    for(int i = 0; i < NUM_PLOTS; ++i) {

        setupPlotWidget(*plotsArr[i], *plotValuesRangeArr[i], plotDenumeratorArr[i], plotLabels[i]);
        plotLayoutArr[i].addWidget(plotsArr[i], /* stretch factor */ 1);

        QCustomPlot* plot_ptr = plotsArr[i];
        const int* denum_ptr = &plotDenumeratorArr[i];
        const QPair<int,int>* range_ptr = plotValuesRangeArr[i];

        plotMqttClientArr[i] = new MqttClient(
            // plotEndpoint[i], [plot_ptr, &timeStart, range_ptr, denum_ptr](double payload) {
            //     qDebug() << "Update graph widget with new data: " << payload;
            plotEndpoint[i], [plot_ptr, &timeStart, range_ptr, denum_ptr](void* payload) {
                // if(*(double*)payload == NULL){
                //     qDebug() << "Failed to parser Double";
                //     return;
                // }
                if(DEBUG)
		qDebug() << "Update graph widget with new data: " << *((double*)payload);

                static double maxY = range_ptr->second;
                const double key = timeStart.msecsTo(QTime::currentTime()) / 1000.0;

                // auto displayValue = payload / *denum_ptr;
                auto displayValue = *((double*)payload) / *denum_ptr;

                plot_ptr->graph(0)->addData(key, displayValue);
                plot_ptr->xAxis->setRange(key, 3, Qt::AlignRight);

                // if (payload > maxY) {
                //     maxY = payload * 1.1;
                if (*((double*)payload) > maxY) {
                    maxY = *((double*)payload) * 1.1;
                    plot_ptr->yAxis->setRange(0, maxY / *denum_ptr);
                }
                plot_ptr->replot();
            },
            &mainWindow
        );
        plotMqttClientArr[i]->start();


        // MqttClient plotClient(
        //     plotEndpoint, [&plotWidget, &timeStart, range_ptr, denum_ptr](double payload) {
        //         qDebug() << "Update graph widget with new data: " << payload;

        //         static double maxY = range_ptr->second;
        //         const double key = timeStart.msecsTo(QTime::currentTime()) / 1000.0;

        //         auto displayValue = payload / *denum_ptr;

        //         plotWidget.graph(0)->addData(key, displayValue);
        //         plotWidget.xAxis->setRange(key, 3, Qt::AlignRight);

        //         if (payload > maxY) {
        //             maxY = payload * 1.1;
        //             plotWidget.yAxis->setRange(0, maxY / *denum_ptr);
        //         }
        //         plotWidget.replot();
        //     },
        //     &mainWindow);
        // plotClient.start();
    }



    for(int i = 0; i < NUM_PLOTS; ++i) {

        QObject::connect(comboBoxArr[i], QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index) {
            if(DEBUG)
		qDebug() << "Updated comboBox[" << i << "] index to: " << index;

            // plotsArr[i]->replot();
            plotsArr[i]->clearGraphs();
            setupPlotWidget(*plotsArr[i], *plotValuesRangeArr[index], plotDenumeratorArr[index], plotLabels[index]);

            QCustomPlot* plot_ptr = plotsArr[i];
            const int* denum_ptr = &plotDenumeratorArr[index];
            const QPair<int,int>* range_ptr = plotValuesRangeArr[index];

            // plotMqttClientArr[i]->stop();
            delete plotMqttClientArr[i];

            plotMqttClientArr[i] = new MqttClient(
                // plotEndpoint[index], [plot_ptr, &timeStart, range_ptr, denum_ptr](double payload) {
                //     qDebug() << "Update graph widget with new client: " << payload;
                plotEndpoint[index], [plot_ptr, &timeStart, range_ptr, denum_ptr](void* payload) {
                    // if(*(double*)payload == NULL){
                    //     qDebug() << "Failed to parser Double";
                    //     return;
                    // }
                    if(DEBUG)
			qDebug() << "Update graph widget with new client: " << *((double*)payload);

                    static double maxY = range_ptr->second;
                    const double key = timeStart.msecsTo(QTime::currentTime()) / 1000.0;

                    // auto displayValue = payload / *denum_ptr;
                    auto displayValue = *((double*)payload) / *denum_ptr;

                    plot_ptr->graph(0)->addData(key, displayValue);
                    plot_ptr->xAxis->setRange(key, 3, Qt::AlignRight);

                    // if (payload > maxY) {
                    //     maxY = payload * 1.1;
                    if (*((double*)payload) > maxY) {
                        maxY = *((double*)payload) * 1.1;
                        plot_ptr->yAxis->setRange(0, maxY / *denum_ptr);
                    }
                    plot_ptr->replot();
                }
            );
            plotMqttClientArr[i]->start();

        });
    }


    for (int i = 0; i < NUM_BARS; ++i){
        progressBars[i]->setValue(0);
    }

    // slight waving of gauge arrow
    // QTimer timer;
    // timer.setInterval(50);
    // QObject::connect(&timer, &QTimer::timeout, [gaugeNeedleItem, &timeStart, &lastGaugeDisplayValue]() {
    //     const double key = timeStart.msecsTo(QTime::currentTime()) / 20.0;
    //     gaugeNeedleItem->setCurrentValue(lastGaugeDisplayValue + 0.1 * (qSin(key) + 1));
    // });
    // timer.start();
    // mainWindow.show();


    // mainWindow.setWindowState(Qt::WindowFullScreen);
    // mainWindow.setWindowState(Qt::WindowMaximized);
    // mainWindow.showMaximized();

    mainWindow.showFullScreen();
    mainWindow.show();

    return app.exec();
}

#include "main.moc"
