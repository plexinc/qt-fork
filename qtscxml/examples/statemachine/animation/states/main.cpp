// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtStateMachine>
#include <QtWidgets>

class Pixmap : public QGraphicsObject
{
    Q_OBJECT
public:
    Pixmap(const QPixmap &pix) : QGraphicsObject(), p(pix)
    {
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        painter->drawPixmap(QPointF(), p);
    }

    QRectF boundingRect() const override
    {
        return QRectF( QPointF(0, 0), p.size());
    }

private:
    QPixmap p;
};

class GraphicsView : public QGraphicsView
{
public:
    GraphicsView(QGraphicsScene *scene) : QGraphicsView(scene)
    {
    }

    void resizeEvent(QResizeEvent *) override
    {
        fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
};

//![0]
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(states);

    QApplication app(argc, argv);

    // Text edit and button
    QTextEdit *edit = new QTextEdit;
    edit->setText("asdf lkjha yuoiqwe asd iuaysd u iasyd uiy "
                  "asdf lkjha yuoiqwe asd iuaysd u iasyd uiy "
                  "asdf lkjha yuoiqwe asd iuaysd u iasyd uiy "
                  "asdf lkjha yuoiqwe asd iuaysd u iasyd uiy!");

    QPushButton *button = new QPushButton;
    QGraphicsProxyWidget *buttonProxy = new QGraphicsProxyWidget;
    buttonProxy->setWidget(button);
    QGraphicsProxyWidget *editProxy = new QGraphicsProxyWidget;
    editProxy->setWidget(edit);

    QGroupBox *box = new QGroupBox;
    box->setFlat(true);
    box->setTitle("Options");

    QVBoxLayout *layout2 = new QVBoxLayout;
    box->setLayout(layout2);
    layout2->addWidget(new QRadioButton("Herring"));
    layout2->addWidget(new QRadioButton("Blue Parrot"));
    layout2->addWidget(new QRadioButton("Petunias"));
    layout2->addStretch();

    QGraphicsProxyWidget *boxProxy = new QGraphicsProxyWidget;
    boxProxy->setWidget(box);

    // Parent widget
    QGraphicsWidget *widget = new QGraphicsWidget;
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical, widget);
    layout->addItem(editProxy);
    layout->addItem(buttonProxy);
    widget->setLayout(layout);

    Pixmap *p1 = new Pixmap(QPixmap(":/digikam.png"));
    Pixmap *p2 = new Pixmap(QPixmap(":/akregator.png"));
    Pixmap *p3 = new Pixmap(QPixmap(":/accessories-dictionary.png"));
    Pixmap *p4 = new Pixmap(QPixmap(":/k3b.png"));
    Pixmap *p5 = new Pixmap(QPixmap(":/help-browser.png"));
    Pixmap *p6 = new Pixmap(QPixmap(":/kchart.png"));

    QGraphicsScene scene(0, 0, 400, 300);
    scene.setBackgroundBrush(scene.palette().window());
    scene.addItem(widget);
    scene.addItem(boxProxy);
    scene.addItem(p1);
    scene.addItem(p2);
    scene.addItem(p3);
    scene.addItem(p4);
    scene.addItem(p5);
    scene.addItem(p6);
//![0]

//![1]
    QStateMachine machine;
    QState *state1 = new QState(&machine);
    QState *state2 = new QState(&machine);
    QState *state3 = new QState(&machine);
    machine.setInitialState(state1);
//![1]

//![2]
    // State 1
    state1->assignProperty(button, "text", "Switch to state 2");
    state1->assignProperty(widget, "geometry", QRectF(0, 0, 400, 150));
    state1->assignProperty(box, "geometry", QRect(-200, 150, 200, 150));
    state1->assignProperty(p1, "pos", QPointF(68, 200)); // 185));
    state1->assignProperty(p2, "pos", QPointF(168, 200)); // 185));
    state1->assignProperty(p3, "pos", QPointF(268, 200)); // 185));
    state1->assignProperty(p4, "pos", QPointF(68 - 150, 48 - 150));
    state1->assignProperty(p5, "pos", QPointF(168, 48 - 150));
    state1->assignProperty(p6, "pos", QPointF(268 + 150, 48 - 150));
    state1->assignProperty(p1, "rotation", qreal(0));
    state1->assignProperty(p2, "rotation", qreal(0));
    state1->assignProperty(p3, "rotation", qreal(0));
    state1->assignProperty(p4, "rotation", qreal(-270));
    state1->assignProperty(p5, "rotation", qreal(-90));
    state1->assignProperty(p6, "rotation", qreal(270));
    state1->assignProperty(boxProxy, "opacity", qreal(0));
    state1->assignProperty(p1, "opacity", qreal(1));
    state1->assignProperty(p2, "opacity", qreal(1));
    state1->assignProperty(p3, "opacity", qreal(1));
    state1->assignProperty(p4, "opacity", qreal(0));
    state1->assignProperty(p5, "opacity", qreal(0));
    state1->assignProperty(p6, "opacity", qreal(0));
//![2]

    // State 2
    state2->assignProperty(button, "text", "Switch to state 3");
    state2->assignProperty(widget, "geometry", QRectF(200, 150, 200, 150));
    state2->assignProperty(box, "geometry", QRect(9, 150, 190, 150));
    state2->assignProperty(p1, "pos", QPointF(68 - 150, 185 + 150));
    state2->assignProperty(p2, "pos", QPointF(168, 185 + 150));
    state2->assignProperty(p3, "pos", QPointF(268 + 150, 185 + 150));
    state2->assignProperty(p4, "pos", QPointF(64, 48));
    state2->assignProperty(p5, "pos", QPointF(168, 48));
    state2->assignProperty(p6, "pos", QPointF(268, 48));
    state2->assignProperty(p1, "rotation", qreal(-270));
    state2->assignProperty(p2, "rotation", qreal(90));
    state2->assignProperty(p3, "rotation", qreal(270));
    state2->assignProperty(p4, "rotation", qreal(0));
    state2->assignProperty(p5, "rotation", qreal(0));
    state2->assignProperty(p6, "rotation", qreal(0));
    state2->assignProperty(boxProxy, "opacity", qreal(1));
    state2->assignProperty(p1, "opacity", qreal(0));
    state2->assignProperty(p2, "opacity", qreal(0));
    state2->assignProperty(p3, "opacity", qreal(0));
    state2->assignProperty(p4, "opacity", qreal(1));
    state2->assignProperty(p5, "opacity", qreal(1));
    state2->assignProperty(p6, "opacity", qreal(1));

    // State 3
    state3->assignProperty(button, "text", "Switch to state 1");
    state3->assignProperty(p1, "pos", QPointF(0, 5));
    state3->assignProperty(p2, "pos", QPointF(0, 5 + 64 + 5));
    state3->assignProperty(p3, "pos", QPointF(5, 5 + (64 + 5) + 64));
    state3->assignProperty(p4, "pos", QPointF(5 + 64 + 5, 5));
    state3->assignProperty(p5, "pos", QPointF(5 + 64 + 5, 5 + 64 + 5));
    state3->assignProperty(p6, "pos", QPointF(5 + 64 + 5, 5 + (64 + 5) + 64));
    state3->assignProperty(widget, "geometry", QRectF(138, 5, 400 - 138, 200));
    state3->assignProperty(box, "geometry", QRect(5, 205, 400, 90));
    state3->assignProperty(p1, "opacity", qreal(1));
    state3->assignProperty(p2, "opacity", qreal(1));
    state3->assignProperty(p3, "opacity", qreal(1));
    state3->assignProperty(p4, "opacity", qreal(1));
    state3->assignProperty(p5, "opacity", qreal(1));
    state3->assignProperty(p6, "opacity", qreal(1));

//![3]
    QAbstractTransition *t1 = state1->addTransition(button, &QAbstractButton::clicked, state2);
    QSequentialAnimationGroup *animation1SubGroup = new QSequentialAnimationGroup;
    animation1SubGroup->addPause(250);
    animation1SubGroup->addAnimation(new QPropertyAnimation(box, "geometry"));
    t1->addAnimation(animation1SubGroup);
    t1->addAnimation(new QPropertyAnimation(widget, "geometry"));
    t1->addAnimation(new QPropertyAnimation(p1, "pos"));
    t1->addAnimation(new QPropertyAnimation(p2, "pos"));
    t1->addAnimation(new QPropertyAnimation(p3, "pos"));
    t1->addAnimation(new QPropertyAnimation(p4, "pos"));
    t1->addAnimation(new QPropertyAnimation(p5, "pos"));
    t1->addAnimation(new QPropertyAnimation(p6, "pos"));
    t1->addAnimation(new QPropertyAnimation(p1, "rotation"));
    t1->addAnimation(new QPropertyAnimation(p2, "rotation"));
    t1->addAnimation(new QPropertyAnimation(p3, "rotation"));
    t1->addAnimation(new QPropertyAnimation(p4, "rotation"));
    t1->addAnimation(new QPropertyAnimation(p5, "rotation"));
    t1->addAnimation(new QPropertyAnimation(p6, "rotation"));
    t1->addAnimation(new QPropertyAnimation(p1, "opacity"));
    t1->addAnimation(new QPropertyAnimation(p2, "opacity"));
    t1->addAnimation(new QPropertyAnimation(p3, "opacity"));
    t1->addAnimation(new QPropertyAnimation(p4, "opacity"));
    t1->addAnimation(new QPropertyAnimation(p5, "opacity"));
    t1->addAnimation(new QPropertyAnimation(p6, "opacity"));
//![3]

    QAbstractTransition *t2 = state2->addTransition(button, &QAbstractButton::clicked, state3);
    t2->addAnimation(new QPropertyAnimation(box, "geometry"));
    t2->addAnimation(new QPropertyAnimation(widget, "geometry"));
    t2->addAnimation(new QPropertyAnimation(p1, "pos"));
    t2->addAnimation(new QPropertyAnimation(p2, "pos"));
    t2->addAnimation(new QPropertyAnimation(p3, "pos"));
    t2->addAnimation(new QPropertyAnimation(p4, "pos"));
    t2->addAnimation(new QPropertyAnimation(p5, "pos"));
    t2->addAnimation(new QPropertyAnimation(p6, "pos"));
    t2->addAnimation(new QPropertyAnimation(p1, "rotation"));
    t2->addAnimation(new QPropertyAnimation(p2, "rotation"));
    t2->addAnimation(new QPropertyAnimation(p3, "rotation"));
    t2->addAnimation(new QPropertyAnimation(p4, "rotation"));
    t2->addAnimation(new QPropertyAnimation(p5, "rotation"));
    t2->addAnimation(new QPropertyAnimation(p6, "rotation"));
    t2->addAnimation(new QPropertyAnimation(p1, "opacity"));
    t2->addAnimation(new QPropertyAnimation(p2, "opacity"));
    t2->addAnimation(new QPropertyAnimation(p3, "opacity"));
    t2->addAnimation(new QPropertyAnimation(p4, "opacity"));
    t2->addAnimation(new QPropertyAnimation(p5, "opacity"));
    t2->addAnimation(new QPropertyAnimation(p6, "opacity"));

    QAbstractTransition *t3 = state3->addTransition(button, &QAbstractButton::clicked, state1);
    t3->addAnimation(new QPropertyAnimation(box, "geometry"));
    t3->addAnimation(new QPropertyAnimation(widget, "geometry"));
    t3->addAnimation(new QPropertyAnimation(p1, "pos"));
    t3->addAnimation(new QPropertyAnimation(p2, "pos"));
    t3->addAnimation(new QPropertyAnimation(p3, "pos"));
    t3->addAnimation(new QPropertyAnimation(p4, "pos"));
    t3->addAnimation(new QPropertyAnimation(p5, "pos"));
    t3->addAnimation(new QPropertyAnimation(p6, "pos"));
    t3->addAnimation(new QPropertyAnimation(p1, "rotation"));
    t3->addAnimation(new QPropertyAnimation(p2, "rotation"));
    t3->addAnimation(new QPropertyAnimation(p3, "rotation"));
    t3->addAnimation(new QPropertyAnimation(p4, "rotation"));
    t3->addAnimation(new QPropertyAnimation(p5, "rotation"));
    t3->addAnimation(new QPropertyAnimation(p6, "rotation"));
    t3->addAnimation(new QPropertyAnimation(p1, "opacity"));
    t3->addAnimation(new QPropertyAnimation(p2, "opacity"));
    t3->addAnimation(new QPropertyAnimation(p3, "opacity"));
    t3->addAnimation(new QPropertyAnimation(p4, "opacity"));
    t3->addAnimation(new QPropertyAnimation(p5, "opacity"));
    t3->addAnimation(new QPropertyAnimation(p6, "opacity"));

//![4]
    machine.start();

    GraphicsView view(&scene);

    view.show();

    return app.exec();
//![4]
}

#include "main.moc"
