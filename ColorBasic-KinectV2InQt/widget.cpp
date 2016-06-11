#include "widget.h"
#include "ui_widget.h"
#include "inc/Kinect.h"
#include <QTimer>
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
    if (pInterfaceToRelease != NULL)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = NULL;
    }
}
static const int        cColorWidth  = 1920;
static const int        cColorHeight = 1080;
IKinectSensor*          m_pKinectSensor;// Current Kinect
IColorFrameReader*      m_pColorFrameReader;// Color reader
RGBQUAD*                m_pColorRGBX;
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_pKinectSensor = NULL;
    m_pColorFrameReader = NULL;
    m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];// create heap storage for color pixel data in RGBX format
    init();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateKinectData()));
    timer->start(33);
}

Widget::~Widget()
{
    delete ui;
    if (m_pColorRGBX)
    {
        delete [] m_pColorRGBX;
        m_pColorRGBX = NULL;
    }

    SafeRelease(m_pColorFrameReader);// done with color frame reader

    if (m_pKinectSensor)
    {
        m_pKinectSensor->Close();// close the Kinect Sensor
    }
    SafeRelease(m_pKinectSensor);
}

void Widget::updateKinectData()
{
    if (!m_pColorFrameReader)
    {
        return;
    }

    IColorFrame* pColorFrame = NULL;

    HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

    if (SUCCEEDED(hr))
    {
        IFrameDescription* pFrameDescription = NULL;
        int nWidth = 0;
        int nHeight = 0;
        ColorImageFormat imageFormat = ColorImageFormat_None;
        UINT nBufferSize = 0;
        RGBQUAD *pBuffer = NULL;

        if (SUCCEEDED(hr))
        {
            hr = pColorFrame->get_FrameDescription(&pFrameDescription);
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameDescription->get_Width(&nWidth);
        }

        if (SUCCEEDED(hr))
        {
            hr = pFrameDescription->get_Height(&nHeight);
        }

        if (SUCCEEDED(hr))
        {
            hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
        }

        if (SUCCEEDED(hr))
        {
            if (imageFormat == ColorImageFormat_Bgra)
            {
                hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
            }
            else if (m_pColorRGBX)
            {
                pBuffer = m_pColorRGBX;
                nBufferSize = cColorWidth * cColorHeight * sizeof(RGBQUAD);
                hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
            }
            else
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
            ui->label->setPixmap(QPixmap::fromImage(QImage((uchar*)pBuffer, nWidth , nHeight, QImage::Format_RGB32)).scaledToWidth(800));
            //ProcessColor(pBuffer, nWidth, nHeight);
        }

        SafeRelease(pFrameDescription);
    }

    SafeRelease(pColorFrame);
}

void Widget::init()
{
    HRESULT hr;

    hr = GetDefaultKinectSensor(&m_pKinectSensor);
    if (FAILED(hr))
    {
        return;
    }
    if (m_pKinectSensor)
    {
        // Initialize the Kinect and get the color reader
        IColorFrameSource* pColorFrameSource = NULL;
        hr = m_pKinectSensor->Open();
        if (SUCCEEDED(hr))
        {
            hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
        }
        if (SUCCEEDED(hr))
        {
            hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
        }
        SafeRelease(pColorFrameSource);
    }

    if (!m_pKinectSensor || FAILED(hr))
    {
        printf("No ready Kinect found! \n");
        return;
    }
}
