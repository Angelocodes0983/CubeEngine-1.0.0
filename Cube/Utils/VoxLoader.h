#pragma once
#include "Dependencies.h"


namespace Cube
{

    struct VoxVoxel {
        uint16_t x, y, z;
        uint8_t colorIndex;
    };

    struct VoxModel {
        uint32_t sizeX, sizeY, sizeZ;
        std::vector<VoxVoxel> voxels;
        uint32_t palette[256];
        glm::ivec3 worldOffset{ 0, 0, 0 };
    };

    struct GroupEntry {
        int32_t nodeId;
        std::vector<int32_t> childNodeIds;
    };

    inline std::vector<GroupEntry> groupNodes;
    // Default MagicaVoxel palette
    static const uint32_t VOX_DEFAULT_PALETTE[256] = {
        0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff,
        0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff,
        0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff, 0xff6699ff, 0xff3399ff,
        0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff,
        0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff,
        0xff0033ff, 0xffff00ff, 0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff,
        0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc,
        0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
        0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc,
        0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc,
        0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc, 0xff6633cc, 0xff3333cc,
        0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc,
        0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99,
        0xff00ff99, 0xffffcc99, 0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99,
        0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999,
        0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
        0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399,
        0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099,
        0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66, 0xff66ff66, 0xff33ff66,
        0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66,
        0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966,
        0xff009966, 0xffff6666, 0xffcc6666, 0xff996666, 0xff666666, 0xff336666,
        0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366,
        0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
        0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33,
        0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33,
        0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933, 0xff669933, 0xff339933,
        0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633,
        0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333,
        0xff003333, 0xffff0033, 0xffcc0033, 0xff990033, 0xff660033, 0xff330033,
        0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00,
        0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
        0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900,
        0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600,
        0xff006600, 0xffff3300, 0xffcc3300, 0xff993300, 0xff663300, 0xff333300,
        0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000,
        0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077,
        0xff000055, 0xff000044, 0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00,
        0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400,
        0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
        0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000,
        0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777,
        0xff555555, 0xff444444, 0xff222222, 0xff111111
    };

    class VoxLoader {
    public:

        static std::vector<VoxModel> loadAll(const std::string& path, size_t maxModels = SIZE_MAX) {

            std::cout << "[VoxLoader] Loading: " << path << "\n";

            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file.is_open())
                throw std::runtime_error("[VoxLoader] Failed to open .vox file: " + path);

            size_t fileSize = file.tellg();
            file.seekg(0);
            std::vector<uint8_t> buf(fileSize);
            file.read(reinterpret_cast<char*>(buf.data()), fileSize);
            file.close();
            std::cout << "[VoxLoader] File size: " << fileSize << " bytes\n";

            size_t pos = 0;

            auto readU32 = [&]() -> uint32_t {
                uint32_t v; memcpy(&v, buf.data() + pos, 4); pos += 4; return v;
                };
            auto readBytes = [&](void* dst, size_t n) {
                memcpy(dst, buf.data() + pos, n); pos += n;
                };
            auto skipBytes = [&](size_t n) { pos += n; };
            auto remaining = [&]() { return pos < fileSize; };


            auto readString = [&]() -> std::string {
                uint32_t len = readU32();
                if (len == 0) return {};
                std::string s(reinterpret_cast<const char*>(buf.data() + pos), len);
                pos += len;
                return s;
                };

         
            auto readDict = [&]() -> std::unordered_map<std::string, std::string> {
                std::unordered_map<std::string, std::string> dict;
                uint32_t count = readU32();
                for (uint32_t i = 0; i < count; i++) {
                    std::string key = readString();
                    std::string val = readString();
                    dict[key] = val;
                }
                return dict;
                };

            char magic[4];
            readBytes(magic, 4);
            if (memcmp(magic, "VOX ", 4) != 0)
                throw std::runtime_error("[VoxLoader] Not a valid .vox file: " + path);
            uint32_t version = readU32();
            std::cout << "[VoxLoader] VOX version: " << version << "\n";

            uint32_t sharedPalette[256];
            for (int i = 0; i < 256; i++) {
                uint32_t v = VOX_DEFAULT_PALETTE[i];
                uint8_t r = (v >> 24) & 0xFF;  
                uint8_t g = (v >> 16) & 0xFF;
                uint8_t b = (v >> 8) & 0xFF;
                uint8_t a = (v >> 0) & 0xFF;
 
                sharedPalette[i] = (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24);
            }
            bool hasCustomPalette = false;

            std::vector<VoxModel> models;
            VoxModel* pendingModel = nullptr;


            struct ShapeEntry { int32_t nodeId; std::vector<uint32_t> modelIndices; };
            std::vector<ShapeEntry> shapeNodes;

            struct TransformEntry {
                int32_t nodeId;
                int32_t childNodeId;
                glm::ivec3 translation{ 0,0,0 };
            };
            std::vector<TransformEntry> transformNodes;

            int chunkCount = 0;

            while (remaining()) {
                if (pos + 12 > fileSize) break;

                char chunkId[4];
                readBytes(chunkId, 4);
                uint32_t chunkSize = readU32();
                uint32_t childrenSize = readU32();
                size_t   chunkEnd = pos + chunkSize;

                std::string id(chunkId, 4);
                // std::cout << "[VoxLoader] Chunk[" << chunkCount++ << "] '" << id
                 //    << "' size=" << chunkSize << "\n";

                if (pos + chunkSize > fileSize) {
                    std::cerr << "[VoxLoader] WARNING: chunk past EOF\n";
                    break;
                }

                if (memcmp(chunkId, "MAIN", 4) == 0) {
                    // container only
                }
                else if (memcmp(chunkId, "PACK", 4) == 0) {
                    uint32_t n = readU32();
                    std::cout << "[VoxLoader]   -> PACK: " << n << " models\n";
                    models.reserve(std::min((size_t)n, maxModels));
                }
                else if (memcmp(chunkId, "SIZE", 4) == 0) {
                    if (models.size() >= maxModels) {
                        //std::cout << "[VoxLoader]   -> limit reached, skipping\n";
                        skipBytes(chunkSize);
                        pendingModel = nullptr;
                    }
                    else {
                        models.emplace_back();
                        pendingModel = &models.back();
                        memcpy(pendingModel->palette, sharedPalette, sizeof(sharedPalette));
                        pendingModel->sizeX = readU32();
                        pendingModel->sizeY = readU32();
                        pendingModel->sizeZ = readU32();
                        std::cout << "[VoxLoader]   -> SIZE[" << models.size() - 1 << "]: "
                            << pendingModel->sizeX << "x"
                            << pendingModel->sizeY << "x"
                            << pendingModel->sizeZ << "\n";
                    }
                }
                else if (memcmp(chunkId, "XYZI", 4) == 0) {
                    if (!pendingModel) { skipBytes(chunkSize); }
                    else {
                        uint32_t numVoxels = readU32();
                        pendingModel->voxels.resize(numVoxels);
                        const uint8_t* src = buf.data() + pos;
                        pos += numVoxels * 4;
                        for (uint32_t i = 0; i < numVoxels; i++) {
                            pendingModel->voxels[i].x = src[i * 4 + 0];
                            pendingModel->voxels[i].y = src[i * 4 + 1];
                            pendingModel->voxels[i].z = src[i * 4 + 2];
                            pendingModel->voxels[i].colorIndex = src[i * 4 + 3];
                        }
                        std::cout << "[VoxLoader]   -> XYZI[" << models.size() - 1
                            << "]: " << numVoxels << " voxels\n";
                        pendingModel = nullptr;
                    }
                }
                else if (memcmp(chunkId, "RGBA", 4) == 0) {
              
                    memcpy(sharedPalette + 1, buf.data() + pos, 255 * 4);
                    pos = chunkEnd; 
                    hasCustomPalette = true;

         
                    if (hasCustomPalette) {
                        for (auto& m : models)
                            memcpy(m.palette, sharedPalette, sizeof(sharedPalette));
                    }

                    std::cout << "[VoxLoader]   -> RGBA: custom palette loaded\n";
                }
                else if (memcmp(chunkId, "nTRN", 4) == 0) {
                    // Format:
                    //   int32  node_id
                    //   DICT   node_attributes  (ignored)
                    //   int32  child_node_id
                    //   int32  reserved_id      (-1)
                    //   int32  layer_id
                    //   int32  num_frames
                    //   for each frame: DICT frame_attributes

                    TransformEntry te;
                    te.nodeId = (int32_t)readU32();
                    readDict();                           
                    te.childNodeId = (int32_t)readU32();
                    readU32();                            
                    readU32();                           
                    uint32_t numFrames = readU32();

                    for (uint32_t f = 0; f < numFrames; f++) {
                        auto frameDict = readDict();
                        if (f == 0) {
                   
                            auto it = frameDict.find("_t");
                            if (it != frameDict.end()) {
                                int tx = 0, ty = 0, tz = 0;
                                sscanf(it->second.c_str(), "%d %d %d", &tx, &ty, &tz);
                                te.translation = { tx, ty, tz };
                                std::cout << "[VoxLoader]   -> nTRN node=" << te.nodeId
                                    << " child=" << te.childNodeId
                                    << " t=(" << tx << "," << ty << "," << tz << ")\n";
                            }
                        }
                    }
                    transformNodes.push_back(te);
                    pos = chunkEnd; 
                }
                else if (memcmp(chunkId, "nSHP", 4) == 0) {


                    ShapeEntry se;
                    se.nodeId = (int32_t)readU32();
                    readDict();
                    uint32_t numShapeModels = readU32();
                    for (uint32_t m = 0; m < numShapeModels; m++) {
                        uint32_t modelIdx = readU32();
                        readDict(); 
                        se.modelIndices.push_back(modelIdx);
                    }
                    std::cout << "[VoxLoader]   -> nSHP node=" << se.nodeId
                        << " models=[";
                    for (auto idx : se.modelIndices) std::cout << idx << " ";
                    std::cout << "]\n";
                    shapeNodes.push_back(se);
                    pos = chunkEnd;
                }
                else if (memcmp(chunkId, "nGRP", 4) == 0) {
                    GroupEntry ge;
                    ge.nodeId = (int32_t)readU32();
                    readDict(); 
                    uint32_t numChildren = readU32();
                    for (uint32_t i = 0; i < numChildren; i++)
                        ge.childNodeIds.push_back((int32_t)readU32());
                    groupNodes.push_back(ge);
                    pos = chunkEnd;
                }
                else {
                    skipBytes(chunkSize);
                }
            }

     
     
            std::unordered_map<int32_t, const TransformEntry*> transformById;
            for (const auto& te : transformNodes)
                transformById[te.nodeId] = &te;

            std::unordered_map<int32_t, const GroupEntry*> groupById;
            for (const auto& ge : groupNodes)
                groupById[ge.nodeId] = &ge;

            std::unordered_map<int32_t, const ShapeEntry*> shapeById;
            for (const auto& se : shapeNodes)
                shapeById[se.nodeId] = &se;

 
            std::function<void(int32_t, glm::ivec3)> walkNode;
            walkNode = [&](int32_t nodeId, glm::ivec3 parentTranslation) {

             
                auto tit = transformById.find(nodeId);
                if (tit != transformById.end()) {
                    const TransformEntry& te = *tit->second;
                    glm::ivec3 accumulated = parentTranslation + te.translation;
                    walkNode(te.childNodeId, accumulated);
                    return;
                }

                auto git = groupById.find(nodeId);
                if (git != groupById.end()) {
                    for (int32_t childId : git->second->childNodeIds)
                        walkNode(childId, parentTranslation);
                    return;
                }

                auto sit = shapeById.find(nodeId);
                if (sit != shapeById.end()) {
                    for (uint32_t modelIdx : sit->second->modelIndices) {
                        if (modelIdx >= models.size()) continue;
                        VoxModel& m = models[modelIdx];
                        m.worldOffset.x = parentTranslation.x - (int32_t)(m.sizeX / 2);
                        m.worldOffset.y = parentTranslation.y - (int32_t)(m.sizeY / 2);
                        m.worldOffset.z = parentTranslation.z - (int32_t)(m.sizeZ / 2);
                        std::cout << "[VoxLoader]   Applied offset to model[" << modelIdx << "]: ("
                            << m.worldOffset.x << "," << m.worldOffset.y << "," << m.worldOffset.z << ")\n";
                    }
                    return;
                }
                };

            std::unordered_set<int32_t> referencedNodes;
            for (const auto& te : transformNodes) referencedNodes.insert(te.childNodeId);
            for (const auto& ge : groupNodes)
                for (int32_t cid : ge.childNodeIds) referencedNodes.insert(cid);

            for (const auto& te : transformNodes) {
                if (referencedNodes.find(te.nodeId) == referencedNodes.end()) {
                 
                    walkNode(te.nodeId, glm::ivec3(0));
                    break;
                }
            }

            if (hasCustomPalette) {
                for (auto& m : models)
                    memcpy(m.palette, sharedPalette, sizeof(sharedPalette));
            }

            std::cout << "[VoxLoader] === Load Summary ===\n";
            std::cout << "[VoxLoader]   Models: " << models.size() << "\n";
            for (size_t i = 0; i < models.size(); i++) {
                std::cout << "[VoxLoader]   Model[" << i << "]: "
                    << models[i].sizeX << "x" << models[i].sizeY << "x" << models[i].sizeZ
                    << " offset=(" << models[i].worldOffset.x << ","
                    << models[i].worldOffset.y << ","
                    << models[i].worldOffset.z << ")"
                    << " voxels=" << models[i].voxels.size() << "\n";
            }
            std::cout << "[VoxLoader]   Palette: "
                << (hasCustomPalette ? "custom" : "default") << "\n";

            if (models.empty())
                std::cerr << "[VoxLoader] WARNING: No models loaded!\n";

            return models;
        }

        static VoxModel load(const std::string& path) {
            auto models = loadAll(path);
            if (models.empty())
                throw std::runtime_error("[VoxLoader] No models found in: " + path);
            if (models.size() > 1)
                std::cout << "[VoxLoader] WARNING: File has " << models.size()
                << " models; load() returns only the first. Use loadAll().\n";
            return models[0];
        }

        static std::vector<unsigned char> toVoxelArray(
            const VoxModel& voxModel,
            uint32_t gridW, uint32_t gridH, uint32_t gridD,
            glm::ivec3 offset = glm::ivec3(0),
            bool swapYZ = true)
        {
            std::vector<unsigned char> voxels(gridW * gridH * gridD, 0);
            int skipped = 0, placed = 0;

            for (const auto& v : voxModel.voxels) {
                int wx = (int)v.x + offset.x;
                int wy, wz;
                if (swapYZ) { wy = (int)v.z + offset.y; wz = (int)v.y + offset.z; }
                else { wy = (int)v.y + offset.y; wz = (int)v.z + offset.z; }

                if (wx < 0 || wy < 0 || wz < 0 ||
                    wx >= (int)gridW || wy >= (int)gridH || wz >= (int)gridD) {
                    skipped++; continue;
                }
                voxels[(size_t)wz * gridW * gridH + (size_t)wy * gridW + wx] = v.colorIndex;
                placed++;
            }

            std::cout << "[VoxLoader] toVoxelArray placed=" << placed
                << " skipped=" << skipped << "\n";
            return voxels;
        }

        static glm::vec3 paletteColor(const VoxModel& model, uint8_t index) {
            uint32_t packed = model.palette[index];
     
            return glm::vec3(
                ((packed >> 0) & 0xFF) / 255.0f,  
                ((packed >> 8) & 0xFF) / 255.0f,  
                ((packed >> 16) & 0xFF) / 255.0f   
            );
        }
    };
}