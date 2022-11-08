// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef STATES_H
#define STATES_H

//Qt
#include <QKeyEventTransition>
#include <QSignalTransition>
#include <QState>

class GraphicsScene;
QT_BEGIN_NAMESPACE
class QStateMachine;
QT_END_NAMESPACE

class PlayState : public QState
{
public:
    explicit PlayState(GraphicsScene *scene, QState *parent = nullptr);
    ~PlayState();

 protected:
    void onEntry(QEvent *) override;

private :
    GraphicsScene *scene;
    QStateMachine *machine;
    int currentLevel;
    int score;

    friend class UpdateScoreState;
    friend class UpdateScoreTransition;
    friend class WinTransition;
    friend class CustomSpaceTransition;
    friend class WinState;
    friend class LostState;
    friend class LevelState;
};

class LevelState : public QState
{
public:
    LevelState(GraphicsScene *scene, PlayState *game, QState *parent = nullptr);
protected:
    void onEntry(QEvent *) override;
private :
    void initializeLevel();
    GraphicsScene *scene;
    PlayState *game;
};

class PauseState : public QState
{
public:
    explicit PauseState(GraphicsScene *scene, QState *parent = nullptr);

protected:
    void onEntry(QEvent *) override;
    void onExit(QEvent *) override;
private :
    GraphicsScene *scene;
};

class LostState : public QState
{
public:
    LostState(GraphicsScene *scene, PlayState *game, QState *parent = nullptr);

protected:
    void onEntry(QEvent *) override;
    void onExit(QEvent *) override;
private :
    GraphicsScene *scene;
    PlayState *game;
};

class WinState : public QState
{
public:
    WinState(GraphicsScene *scene, PlayState *game, QState *parent = nullptr);

protected:
    void onEntry(QEvent *) override;
    void onExit(QEvent *) override;
private :
    GraphicsScene *scene;
    PlayState *game;
};

class UpdateScoreState : public QState
{
public:
    UpdateScoreState(QState *parent);
private:
};

//This transition is used to update the score
class UpdateScoreTransition : public QSignalTransition
{
public:
    UpdateScoreTransition(GraphicsScene *scene, PlayState *game, QAbstractState *target);
protected:
    bool eventTest(QEvent *event) override;
private:
    PlayState *game;
    GraphicsScene *scene;
};

//This transition tests if we have won the game
class WinTransition : public QSignalTransition
{
public:
    WinTransition(GraphicsScene *scene, PlayState *game, QAbstractState *target);
protected:
    bool eventTest(QEvent *event) override;
private:
    PlayState *game;
    GraphicsScene *scene;
};

//This transition is true if one level has been completed and the player wants to continue
 class CustomSpaceTransition : public QKeyEventTransition
{
public:
    CustomSpaceTransition(QWidget *widget, PlayState *game, QEvent::Type type, int key);
protected:
    bool eventTest(QEvent *event) override;
private:
    PlayState *game;
};

#endif // STATES_H
