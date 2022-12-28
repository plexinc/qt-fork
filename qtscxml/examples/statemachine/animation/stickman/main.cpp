// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "animation.h"
#include "node.h"
#include "lifecycle.h"
#include "stickman.h"
#include "graphicsview.h"
#include "rectbutton.h"

#include <QtCore>
#include <QtWidgets>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(stickman);
    QApplication app(argc, argv);

    StickMan *stickMan = new StickMan;
    stickMan->setDrawSticks(false);

    QGraphicsTextItem *textItem = new QGraphicsTextItem();
    textItem->setHtml("<font color=\"white\"><b>Stickman</b>"
        "<p>"
        "Tell the stickman what to do!"
        "</p>"
        "<p><i>"
        "<li>Press <font color=\"purple\">J</font> to make the stickman jump.</li>"
        "<li>Press <font color=\"purple\">D</font> to make the stickman dance.</li>"
        "<li>Press <font color=\"purple\">C</font> to make him chill out.</li>"
        "<li>When you are done, press <font color=\"purple\">Escape</font>.</li>"
        "</i></p>"
        "<p>If he is unlucky, the stickman will get struck by lightning, and never jump, dance or chill out again."
        "</p></font>");
    qreal w = textItem->boundingRect().width();
    QRectF stickManBoundingRect = stickMan->mapToScene(stickMan->boundingRect()).boundingRect();
    textItem->setPos(-w / 2.0, stickManBoundingRect.bottom() + 25.0);

    QGraphicsScene scene;
    scene.addItem(stickMan);

    scene.addItem(textItem);
    scene.setBackgroundBrush(Qt::black);

    GraphicsView view;
    view.setRenderHints(QPainter::Antialiasing);
    view.setTransformationAnchor(QGraphicsView::NoAnchor);
    view.setScene(&scene);

    QRectF sceneRect = scene.sceneRect();
    // making enough room in the scene for stickman to jump and die
    view.resize(sceneRect.width() + 100, sceneRect.height() + 100);
    view.setSceneRect(sceneRect);

    view.show();
    view.setFocus();

    LifeCycle cycle(stickMan, &view);
    cycle.setDeathAnimation(":/animations/dead.bin");

    cycle.addActivity(":/animations/jumping.bin", Qt::Key_J);
    cycle.addActivity(":/animations/dancing.bin", Qt::Key_D);
    cycle.addActivity(":/animations/chilling.bin", Qt::Key_C);

    cycle.start();


    return app.exec();
}
