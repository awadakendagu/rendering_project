#pragma once
#include <CoreLayer/ColorSpace/Spectrum.h>
#include <FunctionLayer/Shape/Intersection.h>

enum class BSDFType
{
  Diffuse,
  Specular,
  DisneyDiffuse,
  DisneySpecular,
  DisneySubsurface,
  DisneyClearcoat
};

struct BSDFSampleResult
{
  Spectrum weight;
  Vector3f wi;   // 采样得到的入射方向
  float pdf;     // 采样该方向的概率密度
  BSDFType type; // 采样得到的类型
};

class BSDF
{
public:
  BSDF(const Vector3f &_normal, const Vector3f &_tangent,
       const Vector3f &_bitangent) // 法线，切线，副切线，构成局部坐标系
  {
    normal = _normal;
    tangent = _tangent;
    bitangent = _bitangent;
  }
  virtual Spectrum f(const Vector3f &wo, const Vector3f &wi) const = 0;
  // 给定出射方向wo和入射方向wi，返回BSDF值（即反射/透射的比率）
  virtual BSDFSampleResult sample(const Vector3f &wo,
                                  const Vector2f &sample) const = 0;
  // 给定出射方向wo和采样点sample，返回采样结果

public:
  Vector3f normal, tangent, bitangent; // 构成局部坐标系
protected:
  Vector3f toLocal(Vector3f world) const
  {
    return Vector3f{dot(tangent, world), dot(normal, world),
                    dot(bitangent, world)};
  } // 将世界坐标系下的向量转换为局部坐标系下的向量
  Vector3f toWorld(Vector3f local) const
  {
    return local[0] * tangent + local[1] * normal + local[2] * bitangent;
  } // 将局部坐标系下的向量转换为世界坐标系下的向量
};