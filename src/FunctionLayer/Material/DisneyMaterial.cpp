#include "DisneyMaterial.h"
#include "./BxDF/DisneyReflection.h"
#include <FunctionLayer/Texture/ConstantTexture.h>

DisneyMaterial::DisneyMaterial()
{
    baseColor = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.5f));
    roughness = 0.5f;
    metallic = 0.0f;
    subsurface = 0.0f;
    specular = 0.5f;
    specularTint = 0.0f;
    anisotropic = 0.0f;
    sheen = 0.0f;
    sheenTint = 0.5f;
    clearcoat = 0.0f;
    clearcoatRoughness = 0.03f;
    ior = 1.5f;
    transmission = 0.0f;
    transmissionRoughness = 0.0f;
}

DisneyMaterial::DisneyMaterial(const Json &json) : Material(json)
{
    if (json.contains("baseColor"))
    {
        if (json["baseColor"].is_object())
        {
            baseColor = Factory::construct_class<Texture<Spectrum>>(json["baseColor"]);
        }
        else if (json["baseColor"].is_array())
        {
            Spectrum c = fetchRequired<Spectrum>(json, "baseColor");
            baseColor = std::make_shared<ConstantTexture<Spectrum>>(c);
        }
        else
        {
            std::cerr << "Error in baseColor format!\n";
            exit(1);
        }
    }
    else
    {
        baseColor = std::make_shared<ConstantTexture<Spectrum>>(Spectrum(0.5f));
    }

    roughness = json.value("roughness", 0.5f);
    metallic = json.value("metallic", 0.0f);
    subsurface = json.value("subsurface", 0.0f);
    specular = json.value("specular", 0.5f);
    specularTint = json.value("specularTint", 0.0f);
    anisotropic = json.value("anisotropic", 0.0f);
    sheen = json.value("sheen", 0.0f);
    sheenTint = json.value("sheenTint", 0.5f);
    clearcoat = json.value("clearcoat", 0.0f);
    clearcoatRoughness = json.value("clearcoatRoughness", 0.03f);
    ior = json.value("ior", 1.5f);
    transmission = json.value("transmission", 0.0f);
    transmissionRoughness = json.value("transmissionRoughness", 0.0f);
}

std::shared_ptr<BSDF>
DisneyMaterial::computeBSDF(const Intersection &intersection) const
{
    Vector3f normal, tangent, bitangent;
    computeShadingGeometry(intersection, &normal, &tangent, &bitangent);
    Spectrum color = baseColor->evaluate(intersection);

    return std::make_shared<DisneyReflection>(
        normal, tangent, bitangent,
        color,
        subsurface, metallic,
        specular, specularTint,
        roughness, anisotropic,
        sheen, sheenTint,
        clearcoat, clearcoatRoughness,
        ior, transmission, transmissionRoughness);
}

REGISTER_CLASS(DisneyMaterial, "disney")
