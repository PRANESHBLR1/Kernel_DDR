#include "mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QApplication>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <cstring>
#include <cstdint>

// Must match ddr.c
#define DDR_IOC_MAGIC  'k'
#define DDR_READ        _IOWR(DDR_IOC_MAGIC, 1, struct ddr_rw_args)
#define DDR_WRITE       _IOW(DDR_IOC_MAGIC,  2, struct ddr_rw_args)
#define DDR_READ_RANGE  _IOWR(DDR_IOC_MAGIC, 3, struct ddr_range_args)
#define DDR_WRITE_RANGE _IOW(DDR_IOC_MAGIC,  4, struct ddr_range_args)

struct ddr_rw_args {
    unsigned long addr;
    uint32_t value;
};

struct ddr_range_args {
    unsigned long addr;
    uint32_t values[256];
    int count;
};

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), fd(-1)
{
    QLabel *addrLabel = new QLabel("Address (hex):");
    QLabel *valueLabel = new QLabel("Value (hex):");
    QLabel *countLabel = new QLabel("Count:");
    QLabel *rangeLabel = new QLabel("Range Values (hex, space-separated):");

    addrEdit = new QLineEdit(this);
    valueEdit = new QLineEdit(this);
    countEdit = new QLineEdit(this);

    rangeEdit = new QTextEdit(this);
    rangeEdit->setMinimumHeight(200);  // 👉 bigger
    rangeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    readButton = new QPushButton("Read", this);
    writeButton = new QPushButton("Write", this);
    readRangeButton = new QPushButton("Read Range", this);
    writeRangeButton = new QPushButton("Write Range", this);

    // 👉 keyboard shortcuts
    readButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    writeButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));

    // 👉 make Read default button
    readButton->setDefault(true);
    readButton->setAutoDefault(true);

    // --- Menubar setup ---
    menuBar = new QMenuBar(this);
    fileMenu = new QMenu("File", this);
    menuBar->addMenu(fileMenu);

    openAct = new QAction("Open...", this);
    openAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    fileMenu->addAction(openAct);

    exitAct = new QAction("Exit", this);
    exitAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    fileMenu->addAction(exitAct);

    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenTriggered);
    connect(exitAct, &QAction::triggered, qApp, &QApplication::quit);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMenuBar(menuBar);

    QHBoxLayout *addrLayout = new QHBoxLayout;
    addrLayout->addWidget(addrLabel);
    addrLayout->addWidget(addrEdit);

    QHBoxLayout *valLayout = new QHBoxLayout;
    valLayout->addWidget(valueLabel);
    valLayout->addWidget(valueEdit);

    QHBoxLayout *cntLayout = new QHBoxLayout;
    cntLayout->addWidget(countLabel);
    cntLayout->addWidget(countEdit);

    mainLayout->addLayout(addrLayout);
    mainLayout->addLayout(valLayout);
    mainLayout->addLayout(cntLayout);
    mainLayout->addWidget(rangeLabel);
    mainLayout->addWidget(rangeEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(readButton);
    btnLayout->addWidget(writeButton);
    btnLayout->addWidget(readRangeButton);
    btnLayout->addWidget(writeRangeButton);

    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);
    setWindowTitle("DDR Register Tool");

    // --- Button connections ---
    connect(readButton, &QPushButton::clicked, this, &MainWindow::onReadClicked);
    connect(writeButton, &QPushButton::clicked, this, &MainWindow::onWriteClicked);
    connect(readRangeButton, &QPushButton::clicked, this, &MainWindow::onReadRangeClicked);
    connect(writeRangeButton, &QPushButton::clicked, this, &MainWindow::onWriteRangeClicked);

    // --- Enter key smart behavior ---
    connect(addrEdit, &QLineEdit::returnPressed, this, &MainWindow::onReadClicked);
    connect(valueEdit, &QLineEdit::returnPressed, this, &MainWindow::onWriteClicked);
    connect(countEdit, &QLineEdit::returnPressed, this, &MainWindow::onReadRangeClicked);

    // open device
    fd = open("/dev/ddr", O_RDWR);
    if (fd < 0) {
        QMessageBox::critical(this, "Error",
            QString("Failed to open /dev/ddr: %1").arg(strerror(errno)));
    }

    // 👉 set initial size
    resize(700, 1000);
}

void MainWindow::onReadClicked() {
    if (fd < 0) return;
    bool ok;
    unsigned long addr = addrEdit->text().toULong(&ok, 16);
    if (!ok) { QMessageBox::warning(this,"Input Error","Invalid address!"); return; }
    ddr_rw_args req{addr,0};
    if (ioctl(fd, DDR_READ, &req) == -1) {
        QMessageBox::warning(this,"Read Failed",strerror(errno)); return;
    }
    valueEdit->setText(QString::number(req.value,16).toUpper());
}

void MainWindow::onWriteClicked() {
    if (fd < 0) return;
    bool ok1, ok2;
    unsigned long addr = addrEdit->text().toULong(&ok1, 16);
    unsigned int val = valueEdit->text().toUInt(&ok2, 16);
    if (!ok1 || !ok2) { QMessageBox::warning(this,"Input Error","Invalid addr/value!"); return; }

    ddr_rw_args req{addr,val};
    int ret = ioctl(fd, DDR_WRITE, &req);
    if (ret == -1) {
        if (errno == EEXIST) {
            // <<-- only change requested: unified message
            QMessageBox::warning(this, "Write Failed",
                QString("Values cannot be overwritten"));
        } else {
            QMessageBox::warning(this,"Write Failed",strerror(errno));
        }
        return;
    }

    QMessageBox::information(this,"Success","Value written successfully!");
}

void MainWindow::onReadRangeClicked() {
    if (fd < 0) return;
    bool ok1, ok2;
    unsigned long addr = addrEdit->text().toULong(&ok1, 16);
    int count = countEdit->text().toInt(&ok2);
    if (!ok1 || !ok2 || count<=0 || count>256) {
        QMessageBox::warning(this,"Input Error","Invalid addr/count!"); return;
    }
    ddr_range_args req;
    req.addr=addr; req.count=count;
    if (ioctl(fd, DDR_READ_RANGE, &req) == -1) {
        QMessageBox::warning(this,"ReadRange Failed",strerror(errno)); return;
    }
    QString result;
    for(int i=0;i<count;i++)
        result += QString("0x%1 ").arg(req.values[i],0,16).toUpper();
    rangeEdit->setText(result.trimmed());
}

void MainWindow::onWriteRangeClicked() {
    if (fd < 0) return;
    bool ok;
    unsigned long addr = addrEdit->text().toULong(&ok, 16);
    if (!ok) { QMessageBox::warning(this,"Input Error","Invalid address!"); return; }
    QStringList tokens = rangeEdit->toPlainText().split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    if (tokens.isEmpty()) { QMessageBox::warning(this,"Input Error","No values!"); return; }
    ddr_range_args req;
    req.addr=addr; req.count=tokens.size();
    if (req.count>256) { QMessageBox::warning(this,"Input Error","Too many values!"); return; }
    for(int i=0;i<req.count;i++) req.values[i]=tokens[i].toUInt(&ok,16);

    int ret = ioctl(fd, DDR_WRITE_RANGE, &req);
    if (ret == -1) {
        if (errno == EEXIST) {
            // <<-- only change requested: unified message
            QMessageBox::warning(this,"WriteRange Failed",
                QString("Values cannot be overwritten"));
        } else {
            QMessageBox::warning(this,"WriteRange Failed",strerror(errno));
        }
        return;
    }

    QMessageBox::information(this,"Success","Range written successfully!");
}

void MainWindow::onOpenTriggered() {
    QString path = QFileDialog::getOpenFileName(
        this,
        "Open Values File",
        QString(),
        "Text Files (*.txt);;All Files (*)"
    );
    if (path.isEmpty()) return;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Failed", "Could not open the file.");
        return;
    }
    QString content = QString::fromUtf8(f.readAll());
    f.close();

    content.replace(",", " ");
    content.replace("\n", " ");
    content.replace("\r", " ");
    content = content.simplified();

    if (rangeEdit) {
        rangeEdit->setPlainText(content.toUpper());
    }
}
