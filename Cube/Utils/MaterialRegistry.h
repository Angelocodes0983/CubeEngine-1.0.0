#pragma once
#include "Dependencies.h"
#include "VoxLoader.h"

namespace Cube
{
    enum class MaterialType : uint8_t {
        Empty = 0,
        Stone = 1,
        Dirt = 2,
        Grass = 3,
        Sand = 4,
        Wood = 5,
        Leaves = 6,
        Water = 7,
        Lava = 8,
        Metal = 9,
        Glass = 10,
        Snow = 11,
        COUNT = 12
    };

    struct MaterialDef {
        std::string name;
        float hardness;
        float density;
        float emissive;
        bool  isTransparent;
        bool  isLiquid;
    };


    struct GPUMaterial {
        glm::vec4 color;
        float     emissive;
        float     hardness;
        float     isTransparent;
        float     isLiquid;
    };
    static_assert(sizeof(GPUMaterial) == 32, "GPUMaterial size mismatch");


    class MaterialRegistry {
    public:

        std::array<MaterialType, 256> paletteToType;

        std::array<MaterialDef, (size_t)MaterialType::COUNT> defs;

        MaterialRegistry() {

            paletteToType.fill(MaterialType::Stone);
            paletteToType[0] = MaterialType::Empty;

            //                           name    hard  dens  emis  transp liquid
            define(MaterialType::Empty, "empty", 0.0f, 0.0f, 0.0f, false, false);
            define(MaterialType::Stone, "stone", 0.9f, 2.5f, 0.0f, false, false);
            define(MaterialType::Dirt, "dirt", 0.4f, 1.5f, 0.0f, false, false);
            define(MaterialType::Grass, "grass", 0.3f, 1.2f, 0.0f, false, false);
            define(MaterialType::Sand, "sand", 0.2f, 1.6f, 0.0f, false, false);
            define(MaterialType::Wood, "wood", 0.6f, 0.8f, 0.0f, false, false);
            define(MaterialType::Leaves, "leaves", 0.1f, 0.3f, 0.0f, true, false);
            define(MaterialType::Water, "water", 0.0f, 1.0f, 0.0f, true, true);
            define(MaterialType::Lava, "lava", 0.0f, 3.0f, 1.0f, false, true);
            define(MaterialType::Metal, "metal", 1.0f, 7.0f, 0.0f, false, false);
            define(MaterialType::Glass, "glass", 0.3f, 2.5f, 0.0f, true, false);
            define(MaterialType::Snow, "snow", 0.1f, 0.5f, 0.0f, false, false);
        }

        void assign(int paletteIndex, MaterialType type) {
            if (paletteIndex < 0 || paletteIndex > 255) {
                std::cerr << "[MaterialRegistry] assign: index " << paletteIndex << " out of range\n";
                return;
            }
            paletteToType[paletteIndex] = type;
            std::cout << "[MaterialRegistry] slot " << paletteIndex
                << " -" << defs[(uint8_t)type].name << "\n";
        }

        void assignRange(int from, int to, MaterialType type) {
            for (int i = from; i <= to; i++)
                paletteToType[i] = type;
            std::cout << "[MaterialRegistry] slots [" << from << "-" << to
                << "] -> " << defs[(uint8_t)type].name << "\n";
        }

        void printAssignments() const {
            std::cout << "[MaterialRegistry] Current assignments:\n";
            for (int i = 1; i < 256; i++) {
                MaterialType t = paletteToType[i];
                if (t == MaterialType::Stone) continue;
                std::cout << "  slot " << i
                    << " -> " << defs[(uint8_t)t].name << "\n";
            }
        }

        std::array<GPUMaterial, 256> buildGPUBuffer(const VoxModel& voxModel) const {
            std::array<GPUMaterial, 256> gpu{};

            for (int i = 0; i < 256; i++) {
                MaterialType  type = paletteToType[i];
                const MaterialDef& def = defs[(uint8_t)type];

                glm::vec3 color = VoxLoader::paletteColor(voxModel, (uint8_t)i);

                gpu[i].color = glm::vec4(color, 1.0f);
                gpu[i].emissive = def.emissive;
                gpu[i].hardness = def.hardness;
                gpu[i].isTransparent = def.isTransparent ? 1.0f : 0.0f;
                gpu[i].isLiquid = def.isLiquid ? 1.0f : 0.0f;

                if (i == 246)
                {
                    gpu[i].color = glm::vec4(color, 1.0f);
                    gpu[i].emissive = 0.9;
                    gpu[i].hardness = def.hardness;
                    gpu[i].isTransparent = def.isTransparent ? 1.0f : 0.0f;
                    gpu[i].isLiquid = def.isLiquid ? 1.0f : 0.0f;
                }
            }

            gpu[0] = {};

            std::cout << "[MaterialRegistry] Built GPU buffer ("
                << sizeof(GPUMaterial) * 256 << " bytes)\n";
            return gpu;
        }

        const MaterialDef& getDefForVoxel(uint8_t paletteIndex) const {
            return defs[(uint8_t)paletteToType[paletteIndex]];
        }

        bool isBreakable(uint8_t paletteIndex) const {
            return getDefForVoxel(paletteIndex).hardness < 1.0f;
        }

        bool isLiquid(uint8_t paletteIndex) const {
            return getDefForVoxel(paletteIndex).isLiquid;
        }

    private:
        void define(MaterialType type, const std::string& name,
            float hardness, float density, float emissive,
            bool transparent, bool liquid)
        {
            auto& d = defs[(uint8_t)type];
            d.name = name;
            d.hardness = hardness;
            d.density = density;
            d.emissive = emissive;
            d.isTransparent = transparent;
            d.isLiquid = liquid;
        }
    };
}