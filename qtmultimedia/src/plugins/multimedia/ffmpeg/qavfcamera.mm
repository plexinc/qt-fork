// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include <qavfcamera_p.h>
#include <qpointer.h>
#include <qmediacapturesession.h>
#include <private/qplatformmediacapture_p.h>
#include "avfcamerautility_p.h"
#include "qavfhelpers_p.h"
#include <qvideosink.h>
#include <private/qrhi_p.h>
#define AVMediaType XAVMediaType
#include "qffmpegvideobuffer_p.h"
#include "qffmpegvideosink_p.h"
extern "C" {
#include <libavutil/hwcontext_videotoolbox.h>
#include <libavutil/hwcontext.h>
}
#undef AVMediaType



#import <AVFoundation/AVFoundation.h>
#include <CoreVideo/CoreVideo.h>

static void releaseHwFrame(void */*opaque*/, uint8_t *data)
{
    CVPixelBufferRelease(CVPixelBufferRef(data));
}

// Make sure this is compatible with the layout used in ffmpeg's hwcontext_videotoolbox
static AVFrame *allocHWFrame(AVBufferRef *hwContext, const CVPixelBufferRef &pixbuf)
{
    AVHWFramesContext *ctx = (AVHWFramesContext*)hwContext->data;
    AVFrame *frame = av_frame_alloc();
    frame->hw_frames_ctx = av_buffer_ref(hwContext);
    frame->extended_data = frame->data;

    frame->buf[0] = av_buffer_create((uint8_t *)pixbuf, 1, releaseHwFrame, NULL, 0);
    frame->data[3] = (uint8_t *)pixbuf;
    CVPixelBufferRetain(pixbuf);
    frame->width  = ctx->width;
    frame->height = ctx->height;
    frame->format = AV_PIX_FMT_VIDEOTOOLBOX;
    if (frame->width != (int)CVPixelBufferGetWidth(pixbuf) ||
        frame->height != (int)CVPixelBufferGetHeight(pixbuf)) {
        // This can happen while changing camera format
        av_frame_free(&frame);
        return nullptr;
    }
    return frame;
}

static AVAuthorizationStatus m_cameraAuthorizationStatus = AVAuthorizationStatusNotDetermined;

@interface QAVFSampleBufferDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

- (QAVFSampleBufferDelegate *) initWithCamera:(QAVFCamera *)renderer;

- (void) captureOutput:(AVCaptureOutput *)captureOutput
         didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection *)connection;

- (void) setHWAccel:(std::unique_ptr<QFFmpeg::HWAccel> &&)accel;

@end

@implementation QAVFSampleBufferDelegate
{
@private
    QAVFCamera *m_camera;
    AVBufferRef *hwFramesContext;
    std::unique_ptr<QFFmpeg::HWAccel> m_accel;
    qint64 startTime;
    qint64 baseTime;
}

- (QAVFSampleBufferDelegate *) initWithCamera:(QAVFCamera *)renderer
{
    if (!(self = [super init]))
        return nil;

    m_camera = renderer;
    hwFramesContext = nullptr;
    startTime = 0;
    baseTime = 0;
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput
         didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
         fromConnection:(AVCaptureConnection *)connection
{
    Q_UNUSED(connection);
    Q_UNUSED(captureOutput);

    // NB: on iOS captureOutput/connection can be nil (when recording a video -
    // avfmediaassetwriter).

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);

    CMTime time = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
    qint64 frameTime = time.timescale ? time.value*1000/time.timescale : 0;
    if (baseTime == 0) {
        // drop the first frame to get a valid frame start time
        baseTime = frameTime;
        startTime = 0;
        return;
    }

    if (!m_accel)
        return;

    AVFrame *avFrame = allocHWFrame(m_accel->hwFramesContextAsBuffer(), imageBuffer);
    if (!avFrame)
        return;

#ifdef USE_SW_FRAMES
    auto *swFrame = av_frame_alloc();
    /* retrieve data from GPU to CPU */
    int ret = av_hwframe_transfer_data(swFrame, avFrame, 0);
    if (ret < 0) {
        qWarning() << "Error transferring the data to system memory\n";
        av_frame_unref(swFrame);
    } else {
        av_frame_unref(avFrame);
        avFrame = swFrame;
    }
#endif

    QVideoFrameFormat format = QAVFHelpers::videoFormatForImageBuffer(imageBuffer);
    if (!format.isValid()) {
        av_frame_unref(avFrame);
        return;
    }

    avFrame->pts = startTime;

    QFFmpegVideoBuffer *buffer = new QFFmpegVideoBuffer(avFrame);
    QVideoFrame frame(buffer, format);
    frame.setStartTime(startTime);
    frame.setEndTime(frameTime);
    startTime = frameTime;

    m_camera->syncHandleFrame(frame);
}

- (void) setHWAccel:(std::unique_ptr<QFFmpeg::HWAccel> &&)accel
{
    m_accel = std::move(accel);
}

@end

QT_BEGIN_NAMESPACE

QAVFCamera::QAVFCamera(QCamera *parent)
    : QAVFCameraBase(parent)
{
    m_captureSession = [[AVCaptureSession alloc] init];
    m_sampleBufferDelegate = [[QAVFSampleBufferDelegate alloc] initWithCamera:this];
}

QAVFCamera::~QAVFCamera()
{
    [m_sampleBufferDelegate release];
    [m_videoInput release];
    [m_videoDataOutput release];
    [m_captureSession release];
}

void QAVFCamera::requestCameraPermissionIfNeeded()
{
    if (m_cameraAuthorizationStatus == AVAuthorizationStatusAuthorized)
        return;

    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo])
    {
        case AVAuthorizationStatusAuthorized:
        {
            m_cameraAuthorizationStatus = AVAuthorizationStatusAuthorized;
            break;
        }
        case AVAuthorizationStatusNotDetermined:
        {
            m_cameraAuthorizationStatus = AVAuthorizationStatusNotDetermined;
            QPointer<QAVFCamera> guard(this);
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (guard)
                        cameraAuthorizationChanged(granted);
                });
            }];
            break;
        }
        case AVAuthorizationStatusDenied:
        case AVAuthorizationStatusRestricted:
        {
            m_cameraAuthorizationStatus = AVAuthorizationStatusDenied;
            return;
        }
    }
}

void QAVFCamera::cameraAuthorizationChanged(bool authorized)
{
    if (authorized) {
        m_cameraAuthorizationStatus = AVAuthorizationStatusAuthorized;
    } else {
        m_cameraAuthorizationStatus = AVAuthorizationStatusDenied;
        qWarning() << "User has denied access to camera";
    }
}

void QAVFCamera::updateVideoInput()
{
    requestCameraPermissionIfNeeded();
    if (m_cameraAuthorizationStatus != AVAuthorizationStatusAuthorized)
        return;

    [m_captureSession beginConfiguration];

    attachVideoInputDevice();

    if (!m_videoDataOutput) {
        m_videoDataOutput = [[[AVCaptureVideoDataOutput alloc] init] autorelease];

        // Configure video output
        m_delegateQueue = dispatch_queue_create("vf_queue", nullptr);
        [m_videoDataOutput
                setSampleBufferDelegate:m_sampleBufferDelegate
                queue:m_delegateQueue];

        [m_captureSession addOutput:m_videoDataOutput];
    }
    [m_captureSession commitConfiguration];
    deviceOrientationChanged();
}

void QAVFCamera::deviceOrientationChanged(int angle)
{
    AVCaptureConnection *connection = [m_videoDataOutput connectionWithMediaType:AVMediaTypeVideo];
    if (connection == nil || !m_videoDataOutput)
        return;

    if (!connection.supportsVideoOrientation)
        return;

    if (angle < 0)
        angle = m_orientationHandler.currentOrientation();

    AVCaptureVideoOrientation orientation = AVCaptureVideoOrientationPortrait;
    switch (angle) {
    default:
        break;
    case 90:
        orientation = AVCaptureVideoOrientationLandscapeRight;
        break;
    case 180:
        // this keeps the last orientation, don't do anything
        return;
    case 270:
        orientation = AVCaptureVideoOrientationLandscapeLeft;
        break;
    }

    connection.videoOrientation = orientation;
}

void QAVFCamera::attachVideoInputDevice()
{
    if (m_videoInput) {
        [m_captureSession removeInput:m_videoInput];
        [m_videoInput release];
        m_videoInput = nullptr;
    }

    QByteArray deviceId = m_cameraDevice.id();
    if (deviceId.isEmpty())
        return;

    AVCaptureDevice *videoDevice = [AVCaptureDevice deviceWithUniqueID:
                    [NSString stringWithUTF8String: deviceId.constData()]];

    if (!videoDevice)
        return;

    m_videoInput = [AVCaptureDeviceInput
                    deviceInputWithDevice:videoDevice
                    error:nil];
    if (m_videoInput && [m_captureSession canAddInput:m_videoInput]) {
        [m_videoInput retain];
        [m_captureSession addInput:m_videoInput];
    } else {
        qWarning() << "Failed to create video device input";
    }
}

AVCaptureDevice *QAVFCamera::device() const
{
    return m_videoInput ? m_videoInput.device : nullptr;
}

bool QAVFCamera::isActive() const
{
    return m_active;
}

void QAVFCamera::setActive(bool active)
{
    if (m_active == active)
        return;
    requestCameraPermissionIfNeeded();
    if (m_cameraAuthorizationStatus != AVAuthorizationStatusAuthorized)
        return;

    m_active = active;

    if (active) {
        // According to the doc, the capture device must be locked before
        // startRunning to prevent the format we set to be overridden by the
        // session preset.
        [m_videoInput.device lockForConfiguration:nil];
        [m_captureSession startRunning];
        [m_videoInput.device unlockForConfiguration];
    } else {
        [m_captureSession stopRunning];
    }

    emit activeChanged(active);
}

void QAVFCamera::setCaptureSession(QPlatformMediaCaptureSession *session)
{
    m_session = session ? session->captureSession() : nullptr;
}

void QAVFCamera::setCamera(const QCameraDevice &camera)
{
    if (m_cameraDevice == camera)
        return;

    m_cameraDevice = camera;

    requestCameraPermissionIfNeeded();
    if (m_cameraAuthorizationStatus == AVAuthorizationStatusAuthorized)
        updateVideoInput();
    setCameraFormat({});
}

bool QAVFCamera::setCameraFormat(const QCameraFormat &format)
{
    if (m_cameraFormat == format && !format.isNull())
        return true;

    QAVFCameraBase::setCameraFormat(format);
    updateCameraFormat();
    return true;
}

void QAVFCamera::updateCameraFormat()
{
    AVCaptureDevice *captureDevice = device();
    if (!captureDevice)
        return;

    uint avPixelFormat = 0;
    AVCaptureDeviceFormat *newFormat = qt_convert_to_capture_device_format(captureDevice, m_cameraFormat);
    if (newFormat) {
        qt_set_active_format(captureDevice, newFormat, false);
        avPixelFormat = setPixelFormat(m_cameraFormat.pixelFormat());
    }

    auto hwAccel = QFFmpeg::HWAccel::create(AV_HWDEVICE_TYPE_VIDEOTOOLBOX);
    if (hwAccel) {
        hwAccel->createFramesContext(av_map_videotoolbox_format_to_pixfmt(avPixelFormat),
                                     m_cameraFormat.resolution());
        hwPixelFormat = hwAccel->hwFormat();
    } else {
        hwPixelFormat = AV_PIX_FMT_NONE;
    }
    [m_sampleBufferDelegate setHWAccel:std::move(hwAccel)];
}

uint QAVFCamera::setPixelFormat(const QVideoFrameFormat::PixelFormat pixelFormat)
{
    // Default to 32BGRA pixel formats on the viewfinder, in case the requested
    // format can't be used (shouldn't happen unless the developers sets a wrong camera
    // format on the camera).
    unsigned avPixelFormat = kCVPixelFormatType_32BGRA;
    if (!QAVFHelpers::toCVPixelFormat(pixelFormat, avPixelFormat))
        qWarning() << "QCamera::setCameraFormat: couldn't convert requested pixel format, using ARGB32";

    bool isSupported = false;
    NSArray *supportedPixelFormats = m_videoDataOutput.availableVideoCVPixelFormatTypes;
    for (NSNumber *currentPixelFormat in supportedPixelFormats)
    {
        if ([currentPixelFormat unsignedIntValue] == avPixelFormat) {
            isSupported = true;
            break;
        }
    }

    if (isSupported) {
        NSDictionary* outputSettings = @{
            (NSString *)kCVPixelBufferPixelFormatTypeKey: [NSNumber numberWithUnsignedInt:avPixelFormat],
            (NSString *)kCVPixelBufferMetalCompatibilityKey: @true
        };
        m_videoDataOutput.videoSettings = outputSettings;
    } else {
        qWarning() << "QCamera::setCameraFormat: requested pixel format not supported. Did you use a camera format from another camera?";
    }
    return avPixelFormat;
}

void QAVFCamera::syncHandleFrame(const QVideoFrame &frame)
{
    Q_EMIT newVideoFrame(frame);
}

QT_END_NAMESPACE

#include "moc_qavfcamera_p.cpp"
