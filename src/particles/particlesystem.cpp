#include "ParticleSystem.h"
#include <cmath>

ParticleSystem::ParticleSystem(const Settings& settings)
    : m_settings(settings) {
    m_particles.reserve(2000); // Резервируем память, чтобы избежать лишних аллокаций
}

void ParticleSystem::UpdateSettings(const Settings& settings) {
    m_settings = settings;
}

void ParticleSystem::SpawnTrail(float fromX, float fromY, float toX, float toY) {
    if (!m_settings.trailEnabled) return;

    float dx = toX - fromX;
    float dy = toY - fromY;
    float dist = std::sqrt(dx * dx + dy * dy);

    float step = (m_settings.stepDistance > 1.0f) ? m_settings.stepDistance : 1.0f;
    int count = static_cast<int>(dist / step);
    if (count == 0) count = 1;

    for (int i = 0; i <= count; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(count);
        Particle p;
        p.x = fromX + dx * t;
        p.y = fromY + dy * t;
        p.maxLife = m_settings.particleLife;
        p.life = p.maxLife;
        p.startRadius = m_settings.startRadius;
        p.endRadius = m_settings.endRadius;
        p.startColor = m_settings.startColor;
        p.endColor = m_settings.endColor;

        m_particles.push_back(p);
    }
}

void ParticleSystem::Update(float dt) {
    for (size_t i = 0; i < m_particles.size();) {
        m_particles[i].life -= dt;
        m_particles[i].animTime += dt; // Увеличиваем таймер анимации

        if (m_particles[i].life <= 0.0f) {
            m_particles[i] = m_particles.back();
            m_particles.pop_back();
        } else {
            ++i;
        }
    }
}