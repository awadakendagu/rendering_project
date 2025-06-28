#pragma once

#include "BSDF.h"
#include "Warp.h"
#include "DisneyDiffuse.h"
#include "DisneySpecular.h"
#include "DisneySubsurfaceScattering.h"
#include "DisneyClearcoat.h" // 假设你之后实现的清漆BSDF模块头文件

class DisneyReflection : public BSDF
{
public:
    DisneyReflection(const Vector3f &normal, const Vector3f &tangent, const Vector3f &bitangent,
                     const Spectrum &baseColor,
                     float subsurface, float metallic,
                     float specular, float specularTint,
                     float roughness, float anisotropic,
                     float sheen, float sheenTint,
                     float clearcoat, float clearcoatRoughness,
                     float ior, float transmission,
                     float transmissionRoughness)
        : BSDF(normal, tangent, bitangent),
          baseColor(baseColor), subsurface(subsurface), metallic(metallic),
          specular(specular), specularTint(specularTint), roughness(roughness),
          anisotropic(anisotropic), sheen(sheen), sheenTint(sheenTint),
          clearcoat(clearcoat), clearcoatRoughness(clearcoatRoughness),
          ior(ior), transmission(transmission), transmissionRoughness(transmissionRoughness),
          diffuse(normal, tangent, bitangent, baseColor, roughness),
          specularPart(normal, tangent, bitangent, baseColor, roughness, specular, specularTint),
          subsurfacePart(normal, tangent, bitangent, baseColor, subsurface, roughness, sheen, sheenTint),
          clearcoatPart(normal, tangent, bitangent, clearcoat, clearcoatRoughness)
    {
    }

    // BSDF 接口实现
    virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const override
    {
        Spectrum result(0.0f);

        if (metallic < 1.0f)
        {
            // 非金属部分才有 diffuse 和 subsurface
            result += (1.0f - metallic) * (1.0f - subsurface) * diffuse.f(wo, wi);
            result += (1.0f - metallic) * subsurface * subsurfacePart.f(wo, wi);
        }

        // 金属和非金属都可能有镜面
        result += specularPart.f(wo, wi);

        // 清漆层（单独 GTR1 高光）
        result += clearcoatPart.f(wo, wi);

        return result;
    }

    virtual BSDFSampleResult sample(const Vector3f &wo, const Vector2f &u) const override
    {
        // 简单多项分支采样（可改为更合理的采样策略）
        float r = u[0];
        Vector2f uRemapped(u[0], u[1]);

        float w_diff = (1.0f - metallic) * (1.0f - subsurface);
        float w_subs = (1.0f - metallic) * subsurface;
        float w_spec = (1.0f - metallic) * specular;
        float w_clear = clearcoat;

        float w_sum = w_diff + w_subs + w_spec + w_clear; // (+ w_trans)
        w_diff /= w_sum;
        w_subs /= w_sum;
        w_spec /= w_sum;
        w_clear /= w_sum;

        BSDFSampleResult res;

        if (r < w_diff)
        {
            float newR = r / w_diff;
            res = diffuse.sample(wo, Vector2f(newR, u[1]));
        }
        else if (r < w_diff + w_subs)
        {
            float newR = (r - w_diff) / w_subs;
            res = subsurfacePart.sample(wo, Vector2f(newR, u[1]));
        }
        else if (r < w_diff + w_subs + w_spec)
        {
            float newR = (r - w_diff - w_subs) / w_spec;
            res = specularPart.sample(wo, Vector2f(newR, u[1]));
        }
        else
        {
            float newR = (r - w_diff - w_subs - w_spec) / w_clear;
            res = clearcoatPart.sample(wo, Vector2f(newR, u[1]));
        }

        return res;
    }

private:
    // 参数
    Spectrum baseColor;
    float subsurface, metallic;
    float specular, specularTint;
    float roughness, anisotropic;
    float sheen, sheenTint;
    float clearcoat, clearcoatRoughness;
    float ior, transmission, transmissionRoughness;

    // 模块
    DisneyDiffuse diffuse;
    DisneySpecular specularPart;
    DisneySubsurfaceScattering subsurfacePart;
    DisneyClearcoat clearcoatPart;
};
