#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTextBrowser>
#include <QSlider>
#include <QLabel>
#include <QTimer>

struct SongInfo {
    qint64 id;
    QString name;
    QString artist;
    QString album;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSearchClicked();
    void onNetworkReplyFinished(QNetworkReply *reply);
    void onSongDoubleClicked(QListWidgetItem *item);
    void onPlayPauseClicked();
    void onStopClicked();
    void onNextClicked();
    void onPrevClicked();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onSliderPressed();
    void onSliderReleased();
    void onVolumeChanged(int value);
    void updateLyrics();

private:
    void setupUI();
    void searchSongs(const QString &keyword);
    void playSong(int index);
    void fetchLyrics(qint64 songId);
    void parseLyrics(const QString &lrcText);
    QString formatTime(qint64 ms);

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QNetworkAccessManager *m_networkManager;

    QLineEdit *m_searchEdit;
    QPushButton *m_searchBtn;
    QListWidget *m_resultList;
    QTextBrowser *m_lyricsBrowser;

    QPushButton *m_playBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_prevBtn;
    QSlider *m_progressSlider;
    QSlider *m_volumeSlider;
    QLabel *m_nowPlayingLabel;
    QLabel *m_timeLabel;

    QTimer *m_lyricsTimer;

    QList<SongInfo> m_songs;
    QList<QPair<qint64, QString>> m_lyrics;
    int m_currentIndex;
    bool m_sliderDragging;
};

#endif // MAINWINDOW_H
