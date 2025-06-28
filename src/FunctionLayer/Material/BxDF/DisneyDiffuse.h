#pragma once
#include "BSDF.h"
#include "Warp.h"

class DisneyDiffuse : public BSDF
{
public:
    DisneyDiffuse(const Vector3f &_normal, const Vector3f &_tangent,
                  const Vector3f &_bitangent, Spectrum _baseColor, float _roughness)
        : BSDF(_normal, _tangent, _bitangent), baseColor(_baseColor), roughness(_roughness) {}

    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
    {
        Vector3f woLocal = toLocal(wo);
        Vector3f wiLocal = toLocal(wi);

        // if (woLocal[1] <= 0.0f || wiLocal[1] <= 0.0f)
        //     return Spectrum(0.0f);

        float cosThetaI = wiLocal[1];
        float cosThetaO = woLocal[1];

        // 计算中间角度 cos(theta_d)
        Vector3f h = normalize(wiLocal + woLocal);
        float cosThetaD = dot(wiLocal, h);

        float FD90 = 0.5f + 2.0f * roughness * cosThetaD * cosThetaD;

        float Fi = 1.0f + (FD90 - 1.0f) * std::pow(1.0f - cosThetaI, 5);
        float Fo = 1.0f + (FD90 - 1.0f) * std::pow(1.0f - cosThetaO, 5);

        Spectrum diffuse = baseColor * INV_PI * Fi * Fo * std::max(cosThetaI, 0.0f);

        return diffuse;
    }

    virtual BSDFSampleResult sample(const Vector3f &wo, const Vector2f &sample) const override
    {
        Vector3f wiLocal = squareToCosineHemisphere(sample);
        Vector3f wi = toWorld(wiLocal);
        float pdf = squareToCosineHemispherePdf(wiLocal);
        Spectrum f_val = f(wo, wi);
        float cosTheta = std::max(toLocal(wi)[1], 0.0f); // cosine term
        Spectrum weight = (pdf > 0) ? (f_val / pdf) : Spectrum(0.0f);
        return {weight, wi, pdf, BSDFType::DisneyDiffuse};
    }

private:
    Spectrum baseColor;
    float roughness;
};
