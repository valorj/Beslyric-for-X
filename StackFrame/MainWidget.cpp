﻿#include "MainWidget.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include "MusicPlayer/musicPlayer.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setMouseTracking(true);

    initEntity();
    initLayout();
    initConnection();
}

MainWidget::~MainWidget()
{

}

void MainWidget::initEntity()
{
}

void MainWidget::initLayout()
{
    topWidget = new TopWidget(this);
    middleWidget = new MiddleWidget(this);
    bottomWidget = new BottomWidget(this);
    topWidget->setObjectName("topWidget");
    middleWidget->setObjectName("middleWidget");
    bottomWidget->setObjectName("bottomWidget");

    topWidget->setMinimumHeight(65);
    topWidget->setMaximumHeight(65);
    topWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    bottomWidget->setMinimumHeight(65);
    bottomWidget->setMaximumHeight(65);
    bottomWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addWidget(topWidget);
    vLayout->addWidget(middleWidget);
    vLayout->addWidget(bottomWidget);
}

void MainWidget::initConnection()
{
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onReloadMusic(QString)),
                                                 this, SLOT(onUnloadLyricFromPreviewPage()));

    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onReloadMusic(QString)),
                                                 middleWidget->pagePreviewLyric, SLOT(setToDefaultAlbumImage()));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onReloadMusic(QString)),
                                                 middleWidget->pagePreviewLyric->lyricViewer, SLOT(setMusicPath(QString)));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onReloadMusic(QString)),
                                                 middleWidget->pageMain->boxPagePreviewLyric, SLOT(setToDefaultPic()));


    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onReloadMusic(QString)),
                                                        bottomWidget, SLOT(reloadMusic(QString)));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onStartMaking()),
                                                        bottomWidget, SLOT(playFromBegin()));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onSeekBackward(quint64)),
                                                        bottomWidget, SLOT(seekBackward(quint64)));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onPlayOrPauseMusic()),
                                                        bottomWidget, SLOT(autoPlayOrPause()));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onStopMusic()),
                                                        bottomWidget, SLOT(stop()));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(loadLrcLyricAndSwitchToPreview(QString))
                                                        ,this, SLOT(onLoadLrcLyricAndSwitchToPreview(QString)));

    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onEnterMakingMode())
                                                        ,bottomWidget, SLOT(enterMakingMode()));
    connect(middleWidget->pageMain->subPageMaking,SIGNAL(onExitMakingMode())
                                                        ,bottomWidget, SLOT(exitMakingMode()));

    connect(bottomWidget->musicPlayer, SIGNAL(positionChanged(int)),this, SLOT(musicPositionChanged(int)));

    connect(bottomWidget->musicPlayer, SIGNAL(titleFound(QString)),
                                            middleWidget->pagePreviewLyric->lyricViewer, SLOT(setMusicTitle(QString)));
    connect(bottomWidget->musicPlayer, SIGNAL(pictureFound(QPixmap)),
                                            middleWidget->pageMain->boxPagePreviewLyric, SLOT(changePic(QPixmap)));
    connect(bottomWidget->musicPlayer, SIGNAL(pictureFound(QPixmap)),
                                            middleWidget->pagePreviewLyric, SLOT(AlbumImageChanged(QPixmap)));
    connect(bottomWidget->musicPlayer, SIGNAL(audioPlay()),
                                            middleWidget->pagePreviewLyric, SLOT(playPhonagraph()));
    connect(bottomWidget->musicPlayer, SIGNAL(audioPause()),
                                            middleWidget->pagePreviewLyric, SLOT(stopPhonagraph()));
    connect(bottomWidget->musicPlayer, SIGNAL(audioFinish()),
                                            middleWidget->pagePreviewLyric, SLOT(stopPhonagraph()));

    connect(bottomWidget->musicPlayer, SIGNAL(audioFinish()),
                                            middleWidget->pageMain->subPageMaking, SLOT(finishMaking()));

}


bool MainWidget::keyPress(QKeyEvent  *event)
{
    if( middleWidget->currentPage == 0 &&                    //在制作页面，且在制作歌词过程
            middleWidget->pageMain->subPageMaking->isMaking) //才响应以下按键
    {
        if(event->key() == Qt::Key_Up)  //推上一行
        {
            middleWidget->pageMain->subPageMaking->markOneLine();
        }
        else if(event->key() == Qt::Key_Right) //空出一行
        {
            middleWidget->pageMain->subPageMaking->markEmptyLine();
        }
        else if(event->key() == Qt::Key_B) //回退 5 秒
        {
            middleWidget->pageMain->subPageMaking->backBy5Second();
        }
        else if(event->key() == Qt::Key_Space) //暂停
        {
            middleWidget->pageMain->subPageMaking->playOrPause();
        }
        else if(event->key() == Qt::Key_Return) //结束制作
        {
            middleWidget->pageMain->subPageMaking->finishMaking();
        }
        return true;
    }

    return false;
}

//载入lrc歌词，并且播放当前歌曲来预览
void MainWidget::onLoadLrcLyricAndSwitchToPreview(QString lycLyricPath)
{
    //为了防止逻辑上的不明确，必须等到没有歌曲在播放的时候，才响应歌词的载入与预览
    if(bottomWidget->musicPlayer->state() != MusicPlayer::StoppedState)
    {
        //没有有效歌词时尝试载入歌词
        if(!middleWidget->pagePreviewLyric->lyricViewer->isLyricValid())
            middleWidget->pagePreviewLyric->lyricViewer->setLrcLyricPath(lycLyricPath);

        //切换页面(从主页面 0，到预览歌词页面1 切换 )
        middleWidget->switchPage(0,1);  //已经有歌词在播放，直接切换页面
        return ;
    }

    //载入歌词
    middleWidget->pagePreviewLyric->lyricViewer->setLrcLyricPath(lycLyricPath);

    //切换页面(从主页面 0，到预览歌词页面1 切换 )
    middleWidget->switchPage(0,1);

    //播放歌曲
    bottomWidget->play();
}


//从预览页面卸下歌词
void MainWidget::onUnloadLyricFromPreviewPage()
{
     middleWidget->pagePreviewLyric->lyricViewer->setEmptyLyric();
}

//音乐位置发生改变
void MainWidget::musicPositionChanged(int pos)
{
    //改变下方音乐条
    bottomWidget->positionChanged(pos);

    //改变制作歌词时的音乐时间
    if(middleWidget->pageMain->subPageMaking->isMaking
            && middleWidget->currentPage == 0)
        middleWidget->pageMain->subPageMaking->updatePos(pos);

    //改变预览歌词时的滚动状态
    if( middleWidget->pagePreviewLyric->lyricViewer->isLyricValid()
            && middleWidget->currentPage == 1)
        middleWidget->pagePreviewLyric->lyricViewer->scrollLyricPanel->lyricPanel->higthLineLyricAtPos(pos);
}

