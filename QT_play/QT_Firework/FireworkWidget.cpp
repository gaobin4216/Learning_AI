#include "FireworkWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QLinearGradient>
#include <QCoreApplication>

static constexpr float GRAVITY = 98.0f;
static constexpr int EXPLOSION_PARTICLES = 60;
static constexpr float DT = 0.016f;

FireworkWidget::FireworkWidget(QWidget *parent)
    : QWidget(parent), m_autoLaunchCounter(0), m_time(0.0f), m_bgCached(false)
{
    resize(900, 700);
    setMinimumSize(400, 300);
    setWindowTitle("烟花模拟 - 点击鼠标发射烟花");
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);

    regenerateStars();
    m_frameBuffer = QPixmap(size());
    m_bgCached = false;

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &FireworkWidget::updateSimulation);
    m_timer->start(16);
}

void FireworkWidget::regenerateStars()
{
    auto *rng = QRandomGenerator::global();
    m_stars.clear();
    m_stars.reserve(80);
    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = rng->bounded(width());
        s.y = rng->bounded(height() * 7 / 10);
        s.brightness = 0.3f + rng->bounded(70) / 100.0f;
        s.twinkleSpeed = 0.5f + rng->bounded(200) / 100.0f;
        m_stars.append(s);
    }
}

void FireworkWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_frameBuffer = QPixmap(size());
    m_bgCached = false;
    regenerateStars();
}

void FireworkWidget::launchFirework(QPointF target)
{
    auto *rng = QRandomGenerator::global();
    Firework fw;
    fw.phase = Firework::Launch;
    fw.launchStart = QPointF(target.x(), height());
    fw.targetPos = target;
    fw.launchProgress = 0.0f;

    int hue = rng->bounded(360);
    fw.baseColor = QColor::fromHsv(hue, 220 + rng->bounded(35), 255).rgb();

    Particle p;
    p.pos = fw.launchStart;
    p.vel = QPointF(0, 0);
    p.color = fw.baseColor;
    p.alpha = 1.0f;
    p.size = 3.0f;
    p.life = 1.0f;
    p.maxLife = 1.0f;
    fw.particles.append(p);

    m_fireworks.append(fw);
}

void FireworkWidget::explodeFirework(Firework &fw)
{
    auto *rng = QRandomGenerator::global();
    fw.phase = Firework::Exploding;

    QPointF center = fw.particles.first().pos;
    fw.particles.clear();

    int shapes = rng->bounded(3);
    QColor base(fw.baseColor);
    float baseHue = base.hueF();

    fw.particles.reserve(EXPLOSION_PARTICLES);
    for (int i = 0; i < EXPLOSION_PARTICLES; ++i) {
        Particle p;
        p.pos = center;

        float angle, speed;
        if (shapes == 0) {
            angle = rng->bounded(3600) * M_PI / 1800.0;
            speed = 80.0f + rng->bounded(1800) / 10.0f;
        } else if (shapes == 1) {
            angle = i * 2.0f * M_PI / EXPLOSION_PARTICLES + rng->bounded(300) / 1000.0f;
            speed = 120.0f + rng->bounded(1500) / 10.0f;
        } else {
            angle = rng->bounded(3600) * M_PI / 1800.0;
            float ring = (rng->bounded(3) == 0) ? 100.0f : 200.0f;
            speed = ring + rng->bounded(800) / 10.0f;
        }

        p.vel = QPointF(cos(angle) * speed, sin(angle) * speed);
        float hueShift = (rng->bounded(600) - 300) / 1000.0f;
        float h = fmod(baseHue + hueShift + 1.0f, 1.0f);
        p.color = QColor::fromHsvF(h, 0.6f + rng->bounded(40) / 100.0f, 1.0f).rgb();
        p.alpha = 1.0f;
        p.size = 1.5f + rng->bounded(200) / 100.0f;
        p.life = 0.8f + rng->bounded(700) / 1000.0f;
        p.maxLife = p.life;
        p.isEmber = (rng->bounded(5) == 0);
        p.trailHead = 0;
        p.trailCount = 0;
        fw.particles.append(p);
    }
}

void FireworkWidget::updateSimulation()
{
    m_time += DT;
    auto *rng = QRandomGenerator::global();

    m_autoLaunchCounter++;
    if (m_autoLaunchCounter >= 50 + rng->bounded(50)) {
        m_autoLaunchCounter = 0;
        spawnAutoFirework();
    }

    const int wh = height();

    for (int f = m_fireworks.size() - 1; f >= 0; --f) {
        Firework &fw = m_fireworks[f];

        if (fw.phase == Firework::Launch) {
            fw.launchProgress += 0.035f;
            float t = fw.launchProgress;
            float easeT = t * t * (3.0f - 2.0f * t);
            Particle &lp = fw.particles[0];
            lp.pos = QPointF(
                fw.launchStart.x() + (fw.targetPos.x() - fw.launchStart.x()) * easeT,
                fw.launchStart.y() + (fw.targetPos.y() - fw.launchStart.y()) * easeT
            );

            int idx = lp.trailHead;
            lp.trail[idx] = lp.pos;
            lp.trailAlpha[idx] = 1.0f;
            lp.trailHead = (idx + 1) % MAX_TRAIL;
            if (lp.trailCount < MAX_TRAIL) lp.trailCount++;

            if (fw.launchProgress >= 1.0f)
                explodeFirework(fw);
            continue;
        }

        int sz = fw.particles.size();
        int writeIdx = 0;
        for (int i = 0; i < sz; ++i) {
            Particle &p = fw.particles[i];

            int idx = p.trailHead;
            p.trail[idx] = p.pos;
            p.trailAlpha[idx] = p.alpha;
            p.trailHead = (idx + 1) % MAX_TRAIL;
            if (p.trailCount < MAX_TRAIL) p.trailCount++;

            p.vel.setY(p.vel.y() + GRAVITY * DT);
            p.vel.setX(p.vel.x() * 0.985f);
            p.vel.setY(p.vel.y() * 0.985f);
            p.pos.setX(p.pos.x() + p.vel.x() * DT);
            p.pos.setY(p.pos.y() + p.vel.y() * DT);

            p.life -= DT * (p.isEmber ? 0.6f : 0.4f);
            p.alpha = qBound(0.0f, p.life / p.maxLife, 1.0f);
            p.size *= 0.998f;

            for (int j = 0; j < MAX_TRAIL; ++j)
                p.trailAlpha[j] *= 0.9f;

            bool alive = p.life > 0 && p.pos.y() <= wh + 20;
            if (alive) {
                if (writeIdx != i)
                    fw.particles[writeIdx] = fw.particles[i];
                writeIdx++;
            }
        }
        if (writeIdx < sz)
            fw.particles.resize(writeIdx);

        if (fw.particles.isEmpty())
            m_fireworks.removeAt(f);
    }

    renderFrame();
    update();
}

void FireworkWidget::spawnAutoFirework()
{
    auto *rng = QRandomGenerator::global();
    float x = 100.0f + rng->bounded((int)(width() - 200));
    float y = 80.0f + rng->bounded((int)(height() * 0.4f));
    launchFirework(QPointF(x, y));
}

void FireworkWidget::renderFrame()
{
    QPainter painter(&m_frameBuffer);

    // Background: cached gradient, stars redrawn each frame (cheap with drawPoint)
    if (!m_bgCached) {
        m_bgPixmap = QPixmap(size());
        QPainter bgPainter(&m_bgPixmap);
        QLinearGradient skyGrad(0, 0, 0, height());
        skyGrad.setColorAt(0.0, QColor(5, 5, 25));
        skyGrad.setColorAt(0.6, QColor(10, 10, 40));
        skyGrad.setColorAt(1.0, QColor(15, 15, 50));
        bgPainter.fillRect(rect(), skyGrad);
        m_bgCached = true;
    }

    painter.drawPixmap(0, 0, m_bgPixmap);

    // Stars - drawPoint is much faster than drawEllipse
    painter.setPen(QPen(QColor(255, 255, 240), 1));
    for (const auto &s : m_stars) {
        float twinkle = 0.5f + 0.5f * sin(m_time * s.twinkleSpeed);
        int alpha = (int)(s.brightness * twinkle * 255);
        painter.setPen(QPen(QColor(255, 255, 240, alpha), 1));
        painter.drawPoint(s.x, s.y);
    }

    // Particles - no antialiasing, minimize state changes
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);

    for (const auto &fw : m_fireworks) {
        if (fw.phase == Firework::Launch) {
            const Particle &lp = fw.particles.first();
            int a = (int)(lp.alpha * 255);

            // Trail
            for (int i = 0; i < lp.trailCount; ++i) {
                int realIdx = (lp.trailHead - lp.trailCount + i + MAX_TRAIL) % MAX_TRAIL;
                float t = (float)(i + 1) / lp.trailCount;
                int ta = (int)(t * a * 0.6f);
                int sz = (int)(2.0f * t + 0.5f);
                painter.setBrush(QColor(255, 200, 100, ta));
                painter.drawEllipse(lp.trail[realIdx].toPoint(), sz, sz);
            }

            // Head
            painter.setBrush(QColor(255, 255, 200, a));
            painter.drawEllipse(lp.pos.toPoint(), 3, 3);
            continue;
        }

        for (const auto &p : fw.particles) {
            if (p.alpha < 0.01f) continue;

            int pa = (int)(p.alpha * 255);
            int r = qRed(p.color), g = qGreen(p.color), b = qBlue(p.color);

            // Trail - draw as small points
            for (int i = 0; i < p.trailCount; ++i) {
                int realIdx = (p.trailHead - p.trailCount + i + MAX_TRAIL) % MAX_TRAIL;
                float t = (float)(i + 1) / p.trailCount;
                float a = p.trailAlpha[realIdx] * t * 0.4f;
                if (a < 0.02f) continue;
                int sz = qMax(1, (int)(p.size * t * 0.5f));
                painter.setBrush(QColor(r, g, b, (int)(a * 255)));
                painter.drawEllipse(p.trail[realIdx].toPoint(), sz, sz);
            }

            // Body
            int sz = qMax(1, (int)p.size);
            painter.setBrush(QColor(r, g, b, pa));
            painter.drawEllipse(p.pos.toPoint(), sz, sz);
        }
    }

    painter.end();
}

void FireworkWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_frameBuffer);
}

void FireworkWidget::mousePressEvent(QMouseEvent *event)
{
    launchFirework(QPointF(event->pos()));
    event->accept();
}

void FireworkWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        auto *rng = QRandomGenerator::global();
        for (int i = 0; i < 5; ++i) {
            float x = 100.0f + rng->bounded((int)(width() - 200));
            float y = 80.0f + rng->bounded((int)(height() * 0.45f));
            launchFirework(QPointF(x, y));
        }
    } else if (event->key() == Qt::Key_Escape) {
        close();
    }
}
