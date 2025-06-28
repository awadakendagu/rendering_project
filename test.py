import os
import json
import time

# 路径设置
scene_dir = "scenes"
output_dir = "output"
os.makedirs(scene_dir, exist_ok=True)
os.makedirs(output_dir, exist_ok=True)

# 参数配置
parameters = [
    "subsurface", "metallic", "specular", "specularTint", "roughness",
    "anisotropic", "sheen", "sheenTint", "clearcoat", "clearcoatRoughness"
]
default = {
    "baseColor": [0.2, 0.5, 0.5],
    "subsurface": 0.0,
    "metallic": 0.0,
    "specular": 0.4,
    "specularTint": 0.2,
    "roughness": 0.3,
    "anisotropic": 0.0,
    "sheen": 0.0,
    "sheenTint": 0.0,
    "clearcoat": 0.0,
    "clearcoatRoughness": 0.0,
    "ior": 0.0,
    "transmission": 0.0,
    "transmissionRoughness": 0.0
}

# 渲染核心函数
def create_scene(param_name, value):
    material = default.copy()
    material[param_name] = value
    material["type"] = "disney"

    scene = {
        "output": {
            "filename": f"{param_name}_{value:.2f}.png"
        },
        "sampler": {
            "type": "independent",
            "xSamples": 8,
            "ySamples": 8
        },
        "camera": {
            "type": "pinhole",
            "transform": {
                "position": [0, 2, 5],
                "up": [0, 1, 0],
                "lookAt": [0, 0.5, 0]
            },
            "tNear": 0.1,
            "tFar": 10000,
            "verticalFov": 45,
            "timeStart": 0,
            "timeEnd": 0,
            "film": {
                "size": [512, 512]
            }
        },
        "integrator": {
            "type": "directSampleLight"
        },
        "scene": {
            "shapes": [
                {
                    "type": "parallelogram",
                    "base": [-10, 0, -10],
                    "edge1": [20, 0, 0],
                    "edge0": [0, 0, 20],
                    "material": {
                        "type": "matte",
                        "albedo": [0.5, 0.5, 0.5]
                    }
                },
                {
                    "type": "sphere",
                    "center": [0, 0.5, 0],
                    "radius": 0.49,
                    "material": material
                }
            ],
            "lights": [
                {
                    "type": "spotLight",
                    "position": [3, 2, 0],
                    "energy": [30, 30, 30]
                }
            ]
        }
    }

    folder_name = f"{param_name}_{value:.2f}"
    folder_path = os.path.join(scene_dir, folder_name)
    os.makedirs(folder_path, exist_ok=True)

    # 保存 scene.json
    scene_path = os.path.join(folder_path, "scene.json")
    with open(scene_path, "w") as f:
        json.dump(scene, f, indent=2)

    # 渲染命令（假设 Moer.exe 在项目根目录）
    print(f"Rendering {folder_name} ...")
    os.system(f"target\\bin\\Moer.exe {folder_path}")
    time.sleep(10)

# 主循环：生成所有场景并渲染
for param in parameters:
    for i in range(11):
        v = round(i / 10.0, 2)
        create_scene(param, v)
