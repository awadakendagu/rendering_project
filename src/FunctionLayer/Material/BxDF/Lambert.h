#pragma once
#include "BSDF.h"
#include "Warp.h"
class LambertReflection : public BSDF
{
public:
  LambertReflection(const Vector3f &_normal, const Vector3f &_tangent,
                    const Vector3f &_bitangent, Spectrum _albedo) // 漫反射BSDF
      : BSDF(_normal, _tangent, _bitangent), albedo(_albedo)
  {
  }

  virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
  {
    Vector3f woLocal = toLocal(wo), wiLocal = toLocal(wi);
    // if (woLocal[1] <= .0f || wiLocal[1] <= .0f)
    //   return Spectrum(0.f);
    return albedo * INV_PI * std::max(wiLocal[1], 0.0f);
  } // 给定出射方向wo和入射方向wi，返回f值

  virtual BSDFSampleResult sample(const Vector3f &wo,
                                  const Vector2f &sample) const override
  {
    Vector3f wi = squareToCosineHemisphere(sample);
    float pdf = squareToCosineHemispherePdf(wi);
    return {albedo, toWorld(wi), pdf, BSDFType::Diffuse};
  }

private:
  Spectrum albedo; // 漫反射的反射率
};