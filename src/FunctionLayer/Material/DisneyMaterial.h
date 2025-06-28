#pragma once

#include "Material.h"

class DisneyMaterial : public Material
{
public:
    DisneyMaterial();
    DisneyMaterial(const Json &json);

    virtual std::shared_ptr<BSDF>
    computeBSDF(const Intersection &intersection) const override;

private:
    std::shared_ptr<Texture<Spectrum>> baseColor;
    float subsurface, metallic;
    float specular, specularTint;
    float roughness, anisotropic;
    float sheen, sheenTint;
    float clearcoat, clearcoatRoughness;
    float ior, transmission, transmissionRoughness;
};
