#pragma once
#include "Particle.h"
#include "../config/Settings.h"
#include <vector>

class ParticleSystem {
public:
    explicit ParticleSystem(const Settings& settings);

    void Update(float dt);
    void SpawnTrail(float fromX, float fromY, float toX, float toY);
    void UpdateSettings(const Settings& settings);

    const std::vector<Particle>& GetParticles() const { return m_particles; }

private:
    Settings m_settings;
    std::vector<Particle> m_particles;
};