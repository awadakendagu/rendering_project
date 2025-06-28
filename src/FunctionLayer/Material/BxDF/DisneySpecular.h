#pragma once
#include "BSDF.h"
#include "Warp.h"

class DisneySpecular : public BSDF
{
public:
    DisneySpecular(const Vector3f &_normal, const Vector3f &_tangent,
                   const Vector3f &_bitangent,
                   const Spectrum &_baseColor,
                   float _roughness, float _specular, float _specularTint)
        : BSDF(_normal, _tangent, _bitangent),
          baseColor(_baseColor), roughness(_roughness),
          specular(_specular), specularTint(_specularTint) {}

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
    {
        Vector3f woLocal = toLocal(wo);
        Vector3f wiLocal = toLocal(wi);
        if (woLocal[1] <= 0 || wiLocal[1] <= 0)
            return Spectrum(0.0f);

        Vector3f h = normalize(woLocal + wiLocal);
        float cosThetaH = std::max(h[1], 0.0f);
        float cosThetaI = std::max(wiLocal[1], 0.0f);
        float cosThetaO = std::max(woLocal[1], 0.0f);

        float D = D_GTR2(h, roughness);
        float rlin = 0.5f + 0.5f * roughness; // [0,1]→[0.5,1]
        float alphaG = rlin * rlin;           // 再平方
        float G = smithG_GGX(wiLocal, alphaG) * smithG_GGX(woLocal, alphaG);

        Spectrum dielectricF0(0.04f);
        Spectrum tintedF0 = specularColor();
        // 2) 根据 specular 参数插值
        Spectrum F0 = lerp(dielectricF0, tintedF0, specular);

        // 3) Schlick Fresnel
        float cosTheta = dot(wiLocal, h);
        Spectrum F = F0 + (Spectrum(1.0f) - F0) * pow(1 - cosTheta, 5);

        return D * G * F / (4 * cosThetaI * cosThetaO + 1e-4f);
    }

    virtual BSDFSampleResult sample(const Vector3f &wo, const Vector2f &sample) const override
    {
        Vector3f woLocal = toLocal(wo);
        if (woLocal[1] <= 0.0f)
            return {Spectrum(0.0f), Vector3f(), 0.0f, BSDFType::DisneySpecular};

        Vector3f h = squareToGGX(sample, roughness);
        Vector3f wiLocal = reflect(-woLocal, h);

        if (wiLocal[1] <= 0.0f)
            return {Spectrum(0.0f), Vector3f(), 0.0f, BSDFType::DisneySpecular};

        float pdf = ggxPdf(woLocal, h, roughness) / (4.0f * dot(woLocal, h));
        Vector3f wi = toWorld(wiLocal);
        Spectrum weight = f(wo, wi) * std::max(wiLocal[1], 0.0f) / pdf;

        return {weight, wi, pdf, BSDFType::DisneySpecular};
    }

private:
    Spectrum baseColor;
    float roughness, specular, specularTint;
    inline Vector3f reflect(const Vector3f &v, const Vector3f &n) const
    {
        return v - 2.0f * dot(v, n) * n;
    }

    inline Vector3f squareToGGX(const Vector2f &sample, float roughness) const
    {
        float alpha = roughness * roughness;

        float phi = 2.0f * M_PI * sample[0];
        float cosTheta = std::sqrt((1.0f - sample[1]) / (1.0f + (alpha * alpha - 1.0f) * sample[1]));
        float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));

        return Vector3f(
            sinTheta * std::cos(phi),
            cosTheta,
            sinTheta * std::sin(phi));
    }

    float D_GTR2(const Vector3f &h, float alpha) const
    {
        float cosThetaH = std::max(h[1], 0.0f);
        float alpha2 = alpha * alpha;
        float denom = cosThetaH * cosThetaH * (alpha2 - 1.0f) + 1.0f;
        return alpha2 / (M_PI * denom * denom + 1e-6f);
    }

    float smithG_GGX(const Vector3f &v, float alpha) const
    {
        float tanTheta = std::sqrt(std::max(1.0f - v[1] * v[1], 0.0f)) / v[1];
        float a = 1.0f / (alpha * tanTheta + 1e-5f);
        if (a >= 1.6f)
            return 1.0f;
        return (3.535f * a + 2.181f * a * a) / (1.0f + 2.276f * a + 2.577f * a * a);
    }

    Spectrum schlickFresnel(float cosTheta) const
    {
        Spectrum F0 = specularColor();
        return F0 + (Spectrum(1.0f) - F0) * std::pow(1.0f - cosTheta, 5.0f);
    }

    Spectrum specularColor() const
    {
        float luminance = 0.3f * baseColor[0] + 0.6f * baseColor[1] + 0.1f * baseColor[2];
        Spectrum tint = (luminance > 0.0f) ? (baseColor / luminance) : Spectrum(1.0f);
        return Spectrum(1.0f - specularTint) + tint * specularTint;
    }

    float ggxPdf(const Vector3f &wo, const Vector3f &h, float alpha) const
    {
        float D = D_GTR2(h, alpha);
        return D * h[1] / (4.0f * dot(wo, h));
    }

    inline Spectrum lerp(const Spectrum &a, const Spectrum &b, float t) const
    {
        return a * (1.0f - t) + b * t;
    }
};
