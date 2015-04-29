#include "mainwindow.h"
#include "ui_mainwindow.h"

// global variables
bool isStreaming;
QString greenTextBox = "background-color: green; border-style: outset; "
                       "border-width: 2px; border-radius: 10px; border-color: beige; "
                       "min-width: 10em; padding: 6px;";
QString redTextBox = "background-color: red; border-style: outset; "
                     "border-width: 2px; border-radius: 10px; border-color: beige; "
                     "min-width: 10em; padding: 6px;";
QString greyTextBox = "background-color: grey; border-style: outset; "
                      "border-width: 2px; border-radius: 10px; border-color: beige; "
                      "min-width: 10em; padding: 6px;";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // a global check for whether streaming is happening
    isStreaming = false;

    // setup the header text
    ui->header->setText("    Mac Side Connection:");
    ui->header->setStyleSheet("font: bold 16px;");

    // setup the instruction text
    ui->instructions->setText("All steps must be performed in order:\n\n"
                              "1. Check the connection, if no connection must fix\n    before continuing\n"
                              "2. Output the stimulation points chosen in\n    Brainsight\n"
                              "3. Begin streaming camera information from\n    Brainsight");

    // setup the points, streaming and connection labels and buttons
    ui->pointsPushButton->setEnabled(false);
    ui->streamingPushButton->setEnabled(false);
    ui->streamingLabel->setStyleSheet(greyTextBox);
    ui->pointsLabel->setStyleSheet(greyTextBox);
    ui->checkConnectionLabel->setStyleSheet(greyTextBox);
    ui->checkConnectionLabel->setText("Not Checked...");
    ui->pointsLabel->setText("Not Submitted...");
    ui->streamingLabel->setText("Not Streaming...");

}

MainWindow::~MainWindow()
{
    delete ui;
}

// responsible for stripping useful data and sending to the share for the windows computer
void MainWindow::streamData()
{
    // open the file to the desktop and allow writing/reading of information
    QString outputFilename = "/Users/as_763/Desktop/output.txt";
    QString inputFilename = "/Users/as_763/Desktop/Tool Locations Dump.txt";
    QFile f_out(outputFilename);
    QFile f_in(inputFilename);
    f_out.open(QIODevice::WriteOnly | QIODevice::Text);
    f_in.open(QIODevice::ReadOnly | QIODevice::Text);

    // if opened the writing file wrong then return
    if(!f_out.isOpen()){
        qDebug() << "- Error, unable to open" << outputFilename << "for output" <<"\n";
        return;
    }

    // if opened the reading file wrong then return
    if(!f_in.isOpen()){
        qDebug() << "- Error, unable to open" << inputFilename << "for input" <<"\n";
        return;
    }

    // point a text stream at the file to allow writing and reading
    QTextStream outStream(&f_out);
    QTextStream inStream(&f_in);

    // go to the end of the file, track the end
    int tracker = 0;
    while (!inStream.atEnd()) {
        inStream.readLine();
        tracker++;
    }

    // go through file again and get the last 8 lines in the file
    int position = 0;
    inStream.seek(0);
    while (!inStream.atEnd()) {
        // check for the 7th to last line
        if (tracker-8 == position) {
            outStream << inStream.readLine() << "\n";
        // check for the 6th to last line
        } else if (tracker-7 == position) {
            outStream << inStream.readLine() << "\n";
        // check for the 5th to last line
        } else if (tracker-6 == position) {
            outStream << inStream.readLine() << "\n";
        // check for the 4th to last line
        } else if (tracker-5 == position) {
            outStream << inStream.readLine() << "\n";
        // check for the 3rd to last line
        } else if (tracker-4 == position) {
            outStream << inStream.readLine() << "\n";
        // check for the 2nd to last line
        } else if (tracker-3 == position) {
            outStream << inStream.readLine() << "\n";
        // check for the 1st to last line
        } else if (tracker-2 == position) {
            outStream << inStream.readLine() << "\n";
        // check for the very last line
        } else if (tracker-1 == position) {
            outStream << inStream.readLine() << "\n";
        // increment the file if none of the above cases hold
        } else {
            inStream.readLine();
        }

        // increment the position counter
        position++;
    }

    // close the files
    f_out.close();
    f_in.close();
}

// control the streaming of the python script
void MainWindow::on_streamingPushButton_clicked()
{
    // depending on streaming conditions want to have different functionality
    if (!isStreaming) {
        ui->streamingLabel->setText("Streaming!");
        ui->streamingLabel->setStyleSheet(greenTextBox);
        ui->streamingPushButton->setText("End Streaming");
        isStreaming = true;
        streamData();
    } else {
        ui->streamingLabel->setText("Not Streaming...");
        ui->streamingLabel->setStyleSheet(redTextBox);
        ui->streamingPushButton->setText("Begin Streaming");
        isStreaming = false;
    }
}

// controls the point submission to the windows computer
void MainWindow::on_pointsPushButton_clicked()
{
    ui->pointsLabel->setText("Points Submitted!");
    ui->pointsLabel->setStyleSheet(greenTextBox);
    ui->pointsPushButton->setEnabled(false);

    // if points submitted then must setup streaming button for last step
    ui->streamingLabel->setStyleSheet(redTextBox);
    ui->streamingPushButton->setEnabled(true);
}

// checks the connection to the share between mac and windows machines
void MainWindow::on_checkConnectionPushButton_clicked()
{
    ui->checkConnectionLabel->setText("Connection Good!");
    ui->checkConnectionLabel->setStyleSheet(greenTextBox);
    ui->checkConnectionPushButton->setEnabled(false); // TODO: may need to be able to check connection many times so possibly adjust

    // if connection good then must set the points button for next step
    ui->pointsLabel->setStyleSheet(redTextBox);
    ui->pointsPushButton->setEnabled(true);
}
