#pragma once
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>

struct Keypoint {
    float time = 0.0f;
    float value = 0.0f;
    float envelope = 0.0f;
};

class NumberSequence {
public:
    std::vector<Keypoint> keypoints;

    // Вычисляет значение кривой в момент времени t (от 0.0 до 1.0)
    float Evaluate(float t) const {
        if (keypoints.empty()) return 0.0f;
        if (keypoints.size() == 1) return keypoints[0].value;

        t = std::clamp(t, 0.0f, 1.0f);

        if (t <= keypoints.front().time) return keypoints.front().value;
        if (t >= keypoints.back().time) return keypoints.back().value;

        for (size_t i = 0; i < keypoints.size() - 1; ++i) {
            const auto& p0 = keypoints[i];
            const auto& p1 = keypoints[i + 1];

            if (t >= p0.time && t <= p1.time) {
                float factor = (t - p0.time) / (p1.time - p0.time);
                return p0.value + factor * (p1.value - p0.value);
            }
        }
        return keypoints.back().value;
    }

    // Парсер строки из Роблокса: "0 0 0  0.5 0.75 0.75  1 0.5 0"
    static NumberSequence Parse(const std::string& raw) {
        NumberSequence seq;
        std::istringstream iss(raw);
        float time, val, env;
        while (iss >> time >> val >> env) {
            seq.keypoints.push_back({ time, val, env });
        }
        // Сортируем по времени на всякий случай
        std::sort(seq.keypoints.begin(), seq.keypoints.end(), [](const Keypoint& a, const Keypoint& b) {
            return a.time < b.time;
        });
        return seq;
    }
};