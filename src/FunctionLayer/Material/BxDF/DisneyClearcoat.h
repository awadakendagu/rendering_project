#pragma once
#include "BSDF.h"
#include "Warp.h"

class DisneyClearcoat : public BSDF
{
public:
    DisneyClearcoat(const Vector3f &normal, const Vector3f &tangent, const Vector3f &bitangent,
                    float clearcoat, float clearcoatRoughness)
        : BSDF(normal, tangent, bitangent), clearcoat(clearcoat), clearcoatRoughness(clearcoatRoughness) {}

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
    {
        Vector3f woL = toLocal(wo);
        Vector3f wiL = toLocal(wi);
        if (woL[1] <= 0 || wiL[1] <= 0)
            return Spectrum(0.0f);

        Vector3f h = normalize(wiL + woL);
        float D = D_GTR1(h, clearcoatRoughness);
        float G = smithG_GGX(wiL, 0.25f) * smithG_GGX(woL, 0.25f);
        float F = schlickFresnel(dot(wiL, h), 0.04f);

        float denom = 4.0f * wiL[1] * woL[1] + 1e-5f;
        return Spectrum(clearcoat * 0.25f * D * G * F / denom);
    }

    virtual BSDFSampleResult sample(const Vector3f &wo, const Vector2f &sample) const override
    {
        Vector3f woL = toLocal(wo);
        Vector3f h = squareToGTR1(sample, clearcoatRoughness);
        Vector3f wiL = reflect(-woL, h);

        if (wiL[1] <= 0)
            return {Spectrum(0.0f), Vector3f(), 0.0f, BSDFType::DisneyClearcoat};

        float pdf = D_GTR1(h, clearcoatRoughness) * h[1] / (4.0f * dot(woL, h));
        Vector3f wi = toWorld(wiL);
        Spectrum f_val = f(wo, wi);
        Spectrum weight = (pdf > 0.0f) ? f_val * wiL[1] / pdf : Spectrum(0.0f);

        return {weight, wi, pdf, BSDFType::DisneyClearcoat};
    }

private:
    float clearcoat, clearcoatRoughness;

    float D_GTR1(const Vector3f &h, float alpha) const
    {
        float cos2h = h[1] * h[1];
        float alpha2 = alpha * alpha;
        float inv = 1.0f + (alpha2 - 1.0f) * cos2h;
        float c = (alpha - 1.0f) / (2.0f * M_PI * std::log(alpha));
        return c * (alpha2 / (alpha2 - 1.0f)) / inv;
    }

    float smithG_GGX(const Vector3f &v, float alpha) const
    {
        float tanTheta2 = (1.0f - v[1] * v[1]) / (v[1] * v[1]);
        return 2.0f / (1.0f + std::sqrt(1.0f + alpha * alpha * tanTheta2));
    }

    float schlickFresnel(float cosTheta, float F0) const
    {
        return F0 + (1.0f - F0) * std::pow(1.0f - cosTheta, 5.0f);
    }

    Vector3f reflect(const Vector3f &v, const Vector3f &n) const
    {
        return v - 2.0f * dot(v, n) * n;
    }

    Vector3f squareToGTR1(const Vector2f &sample, float alpha) const
    {
        float phi = 2.0f * M_PI * sample[0];
        float cosTheta = std::sqrt((1.0f - std::pow(alpha, 1.0f - sample[1])) / (1.0f - alpha));
        float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
        return Vector3f(std::cos(phi) * sinTheta, cosTheta, std::sin(phi) * sinTheta);
    }

    inline float disneyRd(float r, float d)
    {
        // d = scatterDistance 参数
        // 防止 r=0 跳零
        if (r <= 0.0f)
            return 0.0f;
        return (std::exp(-r / d) + std::exp(-r / (3.0f * d))) / (8.0f * M_PI * d * r);
    }
};
