#pragma once
#include "BSDF.h"
#include "Warp.h"
class DisneySubsurfaceScattering : public BSDF
{
public:
    DisneySubsurfaceScattering(const Vector3f &normal, const Vector3f &tangent, const Vector3f &bitangent, const Spectrum &baseColor, float subsurface, float roughness, float sheen, float sheenTint)
        : BSDF(normal, tangent, bitangent), baseColor(baseColor), subsurface(subsurface), roughness(roughness), sheen(sheen), sheenTint(sheenTint) {}

    // 把这段直接替换掉你现在 DisneySubsurfaceScattering::f(...)
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
    {
        Vector3f woL = toLocal(wo), wiL = toLocal(wi);
        if (wiL[1] <= 0.0f)
            return Spectrum(0.0f);

        // 1. 计算 Fresnel 近似项
        float FL = std::pow(1.0f - wiL[1], 5.0f);
        float FV = std::pow(1.0f - woL[1], 5.0f);

        // 2. 中间角度 θd
        Vector3f h = normalize(wiL + woL);
        float cosThetaD = dot(wiL, h);

        // 3. RR = 2 * roughness * cos²θd
        float RR = 2.0f * roughness * cosThetaD * cosThetaD;

        // 4. sub‐BRDF = baseColor/π * [ RR*(FL + FV + FL*FV*(RR - 1)) ]
        float B = FL + FV + FL * FV * (RR - 1.0f);
        Spectrum subs = (baseColor / M_PI) * (RR * B);

        // 5. 混合 strength = subsurface
        return subs * subsurface;
    }

    virtual BSDFSampleResult sample(const Vector3f &wo, const Vector2f &sample) const override
    {
        Vector3f wiLocal = squareToCosineHemisphere(sample);
        Vector3f wi = toWorld(wiLocal);
        float pdf = squareToCosineHemispherePdf(wiLocal);

        Spectrum f_val = f(wo, wi);
        Spectrum weight = (pdf > 0.0f) ? (f_val / pdf) : Spectrum(0.0f);

        return {weight, wi, pdf, BSDFType::DisneySubsurface};
    }

private:
    Spectrum baseColor;
    float subsurface, roughness, sheen, sheenTint;
};
