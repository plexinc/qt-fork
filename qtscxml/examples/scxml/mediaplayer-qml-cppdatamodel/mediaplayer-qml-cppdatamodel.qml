// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import MediaPlayerStateMachine 1.0
import MediaPlayerDataModel 1.0

Mediaplayer {
    MediaPlayerDataModel {
        id: model
    }

    stateMachine: MediaPlayerStateMachine {
        onDataModelChanged: start()
        dataModel: model
    }
}
