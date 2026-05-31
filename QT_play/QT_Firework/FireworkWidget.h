#ifndef FIREWORKWIDGET_H
#define FIREWORKWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QRandomGenerator>
#include <QPixmap>
#include <QtMath>

static const int MAX_TRAIL = 5;

struct Particle {
    QPointF pos;
    QPointF vel;
    QRgb color;
    float alpha;
    float size;
    float life;
    float maxLife;
    QPointF trail[MAX_TRAIL];
    float trailAlpha[MAX_TRAIL];
    int trailHead;
    int trailCount;
    bool isEmber;

    Particle()
        : alpha(1.0f), size(2.0f), life(1.0f), maxLife(1.0f),
          trailHead(0), trailCount(0), isEmber(false)
    {
        for (int i = 0; i < MAX_TRAIL; ++i) trailAlpha[i] = 0.0f;
    }
};

struct Firework {
    enum Phase { Launch, Exploding };

    Phase phase;
    QVector<Particle> particles;
    QPointF launchStart;
    QPointF targetPos;
    QRgb baseColor;
    float launchProgress;

    Firework() : phase(Launch), launchProgress(0.0f) {}
};

class FireworkWidget : public QWidget {
    Q_OBJECT

public:
    explicit FireworkWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateSimulation();

private:
    void launchFirework(QPointF target);
    void explodeFirework(Firework &fw);
    void spawnAutoFirework();
    void renderFrame();
    void regenerateStars();

    QVector<Firework> m_fireworks;
    QTimer *m_timer;
    int m_autoLaunchCounter;
    float m_time;

    struct Star {
        int x, y;
        float brightness;
        float twinkleSpeed;
    };
    QVector<Star> m_stars;

    QPixmap m_frameBuffer;
    QPixmap m_bgPixmap;
    bool m_bgCached;
};

#endif // FIREWORKWIDGET_H
