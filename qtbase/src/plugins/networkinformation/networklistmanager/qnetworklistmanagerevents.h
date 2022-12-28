// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtNetwork/private/qtnetworkglobal_p.h>

#include <QtNetwork/qnetworkinformation.h>

#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include <QtCore/qloggingcategory.h>

#include <objbase.h>
#include <netlistmgr.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <comdef.h>

#if QT_CONFIG(cpp_winrt) && !defined(Q_CC_CLANG)
#define SUPPORTS_WINRT 1
#endif

#ifdef SUPPORTS_WINRT
#include <winrt/base.h>
#include <QtCore/private/qfactorycacheregistration_p.h>
#endif

using namespace Microsoft::WRL;

QT_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(lcNetInfoNLM)

inline QString errorStringFromHResult(HRESULT hr)
{
    _com_error error(hr);
    return QString::fromWCharArray(error.ErrorMessage());
}

class QNetworkListManagerEvents : public QObject, public INetworkListManagerEvents
{
    Q_OBJECT
public:
    QNetworkListManagerEvents();
    virtual ~QNetworkListManagerEvents();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;

    ULONG STDMETHODCALLTYPE AddRef() override { return ++ref; }
    ULONG STDMETHODCALLTYPE Release() override
    {
        if (--ref == 0) {
            delete this;
            return 0;
        }
        return ref;
    }

    HRESULT STDMETHODCALLTYPE ConnectivityChanged(NLM_CONNECTIVITY newConnectivity) override;

    [[nodiscard]] bool start();
    bool stop();

    [[nodiscard]] bool checkBehindCaptivePortal();

signals:
    void connectivityChanged(NLM_CONNECTIVITY);
    void transportMediumChanged(QNetworkInformation::TransportMedium);
    void isMeteredChanged(bool);

private:
    ComPtr<INetworkListManager> networkListManager = nullptr;
    ComPtr<IConnectionPoint> connectionPoint = nullptr;

#ifdef SUPPORTS_WINRT
    void emitWinRTUpdates();

    winrt::event_token token;
#endif

    QAtomicInteger<ULONG> ref = 0;
    DWORD cookie = 0;
};

QT_END_NAMESPACE
