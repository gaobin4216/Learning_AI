#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QScrollBar>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_currentIndex(-1)
    , m_sliderDragging(false)
{
    setupUI();

    // Initialize media player
    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(0.5);
    m_player = new QMediaPlayer(this);
    m_player->setAudioOutput(m_audioOutput);

    // Initialize network manager
    m_networkManager = new QNetworkAccessManager(this);

    // Initialize lyrics timer
    m_lyricsTimer = new QTimer(this);
    m_lyricsTimer->setInterval(100);

    // Connect signals
    connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearchClicked);
    connect(m_resultList, &QListWidget::itemDoubleClicked, this, &MainWindow::onSongDoubleClicked);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onNetworkReplyFinished);
    connect(m_playBtn, &QPushButton::clicked, this, &MainWindow::onPlayPauseClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_nextBtn, &QPushButton::clicked, this, &MainWindow::onNextClicked);
    connect(m_prevBtn, &QPushButton::clicked, this, &MainWindow::onPrevClicked);
    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MainWindow::onPlaybackStateChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::onMediaStatusChanged);
    connect(m_progressSlider, &QSlider::sliderPressed, this, &MainWindow::onSliderPressed);
    connect(m_progressSlider, &QSlider::sliderReleased, this, &MainWindow::onSliderReleased);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);
    connect(m_lyricsTimer, &QTimer::timeout, this, &MainWindow::updateLyrics);

    // Handle playback errors
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &errorString) {
        m_nowPlayingLabel->setText("Playback error: " + errorString);
        m_playBtn->setText(QString::fromUtf8("\xe2\x96\xb6"));
        m_lyricsTimer->stop();
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("Music Player");
    setMinimumSize(900, 700);

    // Dark theme stylesheet
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1a1a2e;
        }
        QLabel {
            color: #eee;
            font-size: 14px;
        }
        QLineEdit {
            background-color: #16213e;
            color: white;
            border: 2px solid #0f3460;
            border-radius: 8px;
            padding: 8px 12px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #e94560;
        }
        QPushButton {
            background-color: #e94560;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #c81e45;
        }
        QPushButton:pressed {
            background-color: #a01838;
        }
        QListWidget {
            background-color: #16213e;
            color: #ddd;
            border: 2px solid #0f3460;
            border-radius: 8px;
            padding: 4px;
            font-size: 13px;
        }
        QListWidget::item {
            padding: 8px;
            border-radius: 4px;
        }
        QListWidget::item:selected {
            background-color: #e94560;
            color: white;
        }
        QListWidget::item:hover {
            background-color: #0f3460;
        }
        QTextBrowser {
            background-color: #16213e;
            color: #ddd;
            border: 2px solid #0f3460;
            border-radius: 8px;
            padding: 8px;
            font-size: 16px;
        }
        QSlider::groove:horizontal {
            background: #0f3460;
            height: 8px;
            border-radius: 4px;
        }
        QSlider::handle:horizontal {
            background: #e94560;
            width: 18px;
            height: 18px;
            margin: -5px 0;
            border-radius: 9px;
        }
        QSlider::sub-page:horizontal {
            background: #e94560;
            border-radius: 4px;
        }
    )");

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Search bar
    QHBoxLayout *searchLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(QString::fromUtf8("\xf0\x9f\x94\x8d") + " Search for songs...");
    m_searchBtn = new QPushButton("Search");
    m_searchBtn->setFixedWidth(100);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);
    mainLayout->addLayout(searchLayout);

    // Main content area (results + lyrics)
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setStyleSheet("QSplitter::handle { background-color: #0f3460; width: 2px; }");

    // Search results
    m_resultList = new QListWidget;
    m_resultList->setMinimumWidth(300);
    splitter->addWidget(m_resultList);

    // Lyrics display
    m_lyricsBrowser = new QTextBrowser;
    m_lyricsBrowser->setOpenExternalLinks(false);
    m_lyricsBrowser->setPlaceholderText("Lyrics will appear here...");
    splitter->addWidget(m_lyricsBrowser);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    mainLayout->addWidget(splitter);

    // Now playing label
    m_nowPlayingLabel = new QLabel("No song playing");
    m_nowPlayingLabel->setAlignment(Qt::AlignCenter);
    m_nowPlayingLabel->setStyleSheet("font-size: 16px; color: #e94560; font-weight: bold;");
    mainLayout->addWidget(m_nowPlayingLabel);

    // Playback controls
    QHBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setAlignment(Qt::AlignCenter);

    m_prevBtn = new QPushButton(QString::fromUtf8("\xe2\x8f\xae"));
    m_prevBtn->setFixedSize(50, 50);
    m_playBtn = new QPushButton(QString::fromUtf8("\xe2\x96\xb6"));
    m_playBtn->setFixedSize(60, 60);
    m_stopBtn = new QPushButton(QString::fromUtf8("\xe2\x8f\xb9"));
    m_stopBtn->setFixedSize(50, 50);
    m_nextBtn = new QPushButton(QString::fromUtf8("\xe2\x8f\xaf"));
    m_nextBtn->setFixedSize(50, 50);

    controlLayout->addWidget(m_prevBtn);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(m_playBtn);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(m_stopBtn);
    controlLayout->addSpacing(10);
    controlLayout->addWidget(m_nextBtn);
    mainLayout->addLayout(controlLayout);

    // Progress bar
    QHBoxLayout *progressLayout = new QHBoxLayout;
    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setRange(0, 0);
    m_timeLabel = new QLabel("00:00 / 00:00");
    m_timeLabel->setFixedWidth(120);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    progressLayout->addWidget(m_progressSlider);
    progressLayout->addWidget(m_timeLabel);
    mainLayout->addLayout(progressLayout);

    // Volume control
    QHBoxLayout *volumeLayout = new QHBoxLayout;
    QLabel *volumeIcon = new QLabel(QString::fromUtf8("\xf0\x9f\x94\x8a"));
    volumeIcon->setFixedWidth(30);
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);
    m_volumeSlider->setFixedWidth(200);
    volumeLayout->addStretch();
    volumeLayout->addWidget(volumeIcon);
    volumeLayout->addWidget(m_volumeSlider);
    volumeLayout->addStretch();
    mainLayout->addLayout(volumeLayout);

    setCentralWidget(centralWidget);

    // Show welcome message in lyrics area
    m_lyricsBrowser->setHtml(
        "<html><body style='background:#16213e;color:#888;text-align:center;padding:80px;'>"
        "<p style='font-size:48px;margin-bottom:20px;'>" + QString::fromUtf8("\xf0\x9f\x8e\xb5") + "</p>"
        "<p style='font-size:20px;color:#aaa;'>Music Player</p>"
        "<p style='font-size:14px;color:#666;margin-top:20px;'>Search for a song and double-click to play</p>"
        "</body></html>"
    );
}

void MainWindow::onSearchClicked()
{
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) return;

    m_resultList->clear();
    m_resultList->addItem("Searching...");
    searchSongs(keyword);
}

void MainWindow::searchSongs(const QString &keyword)
{
    QNetworkRequest request{QUrl("https://music.163.com/api/search/get/web")};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Referer", "https://music.163.com/");

    QByteArray postData;
    postData.append("s=" + QUrl::toPercentEncoding(keyword) + "&type=1&limit=30&offset=0");

    m_networkManager->post(request, postData);
}

void MainWindow::onNetworkReplyFinished(QNetworkReply *reply)
{
    QString url = reply->url().toString();

    if (url.contains("song/lyric")) {
        // Handle lyrics response
        if (reply->error() != QNetworkReply::NoError) {
            m_lyricsBrowser->setHtml(
                "<html><body style='background:#16213e;color:#666;text-align:center;padding:50px;'>"
                "<p>Failed to load lyrics</p></body></html>"
            );
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        QJsonObject lrc = root["lrc"].toObject();
        QString lyricText = lrc["lyric"].toString();

        if (lyricText.isEmpty()) {
            m_lyricsBrowser->setHtml(
                "<html><body style='background:#16213e;color:#666;text-align:center;padding:50px;'>"
                "<p>No lyrics available for this song</p></body></html>"
            );
        } else {
            parseLyrics(lyricText);
        }

        reply->deleteLater();
        return;
    }

    // Handle search response
    if (reply->error() != QNetworkReply::NoError) {
        m_resultList->clear();
        m_resultList->addItem("Search failed: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject root = doc.object();
    QJsonObject result = root["result"].toObject();
    QJsonArray songs = result["songs"].toArray();

    m_songs.clear();
    m_resultList->clear();

    if (songs.isEmpty()) {
        m_resultList->addItem("No results found");
        reply->deleteLater();
        return;
    }

    for (const QJsonValue &val : songs) {
        QJsonObject song = val.toObject();
        SongInfo info;
        info.id = song["id"].toInteger();
        info.name = song["name"].toString();

        QJsonArray artists = song["artists"].toArray();
        QStringList artistNames;
        for (const QJsonValue &a : artists) {
            artistNames.append(a.toObject()["name"].toString());
        }
        info.artist = artistNames.join(", ");
        info.album = song["album"].toObject()["name"].toString();

        m_songs.append(info);
        m_resultList->addItem(info.name + " - " + info.artist + "  [" + info.album + "]");
    }

    reply->deleteLater();
}

void MainWindow::onSongDoubleClicked(QListWidgetItem *item)
{
    int index = m_resultList->row(item);
    if (index >= 0 && index < m_songs.size()) {
        playSong(index);
    }
}

void MainWindow::playSong(int index)
{
    if (index < 0 || index >= m_songs.size()) return;

    m_currentIndex = index;
    const SongInfo &song = m_songs[index];

    // Set the song URL
    QString url = QString("https://music.163.com/song/media/outer/url?id=%1.mp3").arg(song.id);
    m_player->setSource(QUrl(url));

    // Update UI
    m_nowPlayingLabel->setText(QString::fromUtf8("\xe2\x99\xaa ") + song.name + " - " + song.artist);
    m_playBtn->setText(QString::fromUtf8("\xe2\x8f\xb8"));

    // Highlight current song in list
    m_resultList->setCurrentRow(m_currentIndex);

    // Clear lyrics
    m_lyrics.clear();
    m_lyricsBrowser->setHtml(
        "<html><body style='background:#16213e;color:#666;text-align:center;padding:50px;'>"
        "<p>Loading lyrics...</p></body></html>"
    );

    // Fetch lyrics
    fetchLyrics(song.id);

    // Start playing
    m_player->play();
    m_lyricsTimer->start();
}

void MainWindow::fetchLyrics(qint64 songId)
{
    QString url = QString("https://music.163.com/api/song/lyric?id=%1&lv=1&tv=1").arg(songId);
    QNetworkRequest request{QUrl(url)};
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36");
    request.setRawHeader("Referer", "https://music.163.com/");

    m_networkManager->get(request);
}

void MainWindow::parseLyrics(const QString &lrcText)
{
    m_lyrics.clear();

    QStringList lines = lrcText.split('\n');
    QRegularExpression regex(R"(\[(\d{2}):(\d{2})\.(\d{2,3})\](.*))");

    for (const QString &line : lines) {
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            int minutes = match.captured(1).toInt();
            int seconds = match.captured(2).toInt();
            int ms = match.captured(3).toInt();
            if (match.captured(3).length() == 2) ms *= 10;

            qint64 timestamp = minutes * 60000 + seconds * 1000 + ms;
            QString text = match.captured(4).trimmed();

            if (!text.isEmpty()) {
                m_lyrics.append(qMakePair(timestamp, text));
            }
        }
    }

    std::sort(m_lyrics.begin(), m_lyrics.end());
    updateLyrics();
}

void MainWindow::updateLyrics()
{
    if (m_lyrics.isEmpty()) return;

    qint64 position = m_player->position();
    int currentIndex = -1;

    for (int i = m_lyrics.size() - 1; i >= 0; i--) {
        if (position >= m_lyrics[i].first) {
            currentIndex = i;
            break;
        }
    }

    QString html = "<html><body style='background:#16213e;padding:20px;'>";
    html += "<div style='text-align:center;'>";

    // Add some spacing at the top
    html += "<br><br>";

    int start = qMax(0, currentIndex - 5);
    int end = qMin(m_lyrics.size(), currentIndex + 6);

    for (int i = start; i < end; i++) {
        if (i == currentIndex) {
            html += QString("<p id='current' style='font-size:24px;color:#e94560;font-weight:bold;margin:12px 0;'>%1</p>")
                    .arg(m_lyrics[i].second.toHtmlEscaped());
        } else {
            int distance = qAbs(i - currentIndex);
            int alpha = qMax(80, 200 - distance * 30);
            html += QString("<p style='font-size:16px;color:rgba(200,200,200,%1);margin:8px 0;'>%2</p>")
                    .arg(alpha)
                    .arg(m_lyrics[i].second.toHtmlEscaped());
        }
    }

    html += "<br><br></div></body></html>";
    m_lyricsBrowser->setHtml(html);

    // Scroll to current line
    m_lyricsBrowser->scrollToAnchor("current");
}

void MainWindow::onPlayPauseClicked()
{
    if (m_songs.isEmpty()) return;

    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
        m_playBtn->setText(QString::fromUtf8("\xe2\x96\xb6"));
        m_lyricsTimer->stop();
    } else {
        m_player->play();
        m_playBtn->setText(QString::fromUtf8("\xe2\x8f\xb8"));
        m_lyricsTimer->start();
    }
}

void MainWindow::onStopClicked()
{
    m_player->stop();
    m_playBtn->setText(QString::fromUtf8("\xe2\x96\xb6"));
    m_lyricsTimer->stop();
}

void MainWindow::onNextClicked()
{
    if (m_currentIndex < m_songs.size() - 1) {
        playSong(m_currentIndex + 1);
    }
}

void MainWindow::onPrevClicked()
{
    if (m_currentIndex > 0) {
        playSong(m_currentIndex - 1);
    }
}

void MainWindow::onPositionChanged(qint64 position)
{
    if (!m_sliderDragging) {
        m_progressSlider->setValue(static_cast<int>(position));
    }
    m_timeLabel->setText(formatTime(position) + " / " + formatTime(m_player->duration()));
}

void MainWindow::onDurationChanged(qint64 duration)
{
    m_progressSlider->setRange(0, static_cast<int>(duration));
}

void MainWindow::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::StoppedState) {
        m_playBtn->setText(QString::fromUtf8("\xe2\x96\xb6"));
        m_lyricsTimer->stop();
    }
}

void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        // Auto play next song
        if (m_currentIndex < m_songs.size() - 1) {
            playSong(m_currentIndex + 1);
        }
    }
}

void MainWindow::onSliderPressed()
{
    m_sliderDragging = true;
}

void MainWindow::onSliderReleased()
{
    m_sliderDragging = false;
    m_player->setPosition(m_progressSlider->value());
}

void MainWindow::onVolumeChanged(int value)
{
    m_audioOutput->setVolume(value / 100.0);
}

QString MainWindow::formatTime(qint64 ms)
{
    int seconds = static_cast<int>(ms / 1000);
    int minutes = seconds / 60;
    seconds = seconds % 60;
    return QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
}
