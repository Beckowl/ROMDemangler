#include "Skybox.h"
#include "memory.h"
#include "stb_image_write.h"
#include "BinImg.h"
#include "main.h"

struct SkyboxTile {
    std::vector<u8> Texture;
    std::string Name;
};

static std::vector<SkyboxTile> GetSkyboxTiles(u32 SegStart, const std::vector<u8>& SegData, s32 NumTiles) {
    std::vector<SkyboxTile> Tiles;
    Tiles.reserve(NumTiles);

    u32 Offset = 0;

    for (int i = 0; i < NumTiles; i++) {
        SkyboxTile Tile;
        Tile.Name = std::format("SkyboxCustom_{:08X}_{}", SegmentedToROM(SegStart + Offset), i);
        Tile.Texture.resize(4096);

        BinImg::DecodeRGBA16(SegData.data() + Offset, Tile.Texture.data(), 1024);

        Offset += 2048;
        Tiles.push_back(std::move(Tile));
    }

    return Tiles;
}

static void ExportSkyboxTiles(const std::vector<SkyboxTile>& Tiles, const std::string& Path) {
    fs::create_directories(Path);

    for (const auto& Tile : Tiles) {
        std::string FileName = Path + Tile.Name + ".rgba16.png";
        stbi_write_png(FileName.c_str(), 32, 32, 4, Tile.Texture.data(), 32 * 4);
    }
}

static void ExportPtrList(FILE* File, const std::vector<SkyboxTile>& Tiles, const std::string& SkyboxName) {
    for (const auto& Tile : Tiles) {
        fprintf(File,
            "ALIGNED8 static const Texture %s[] = \"%s\";\n\n",
            Tile.Name.c_str(),
            ("textures/skybox_tiles/" + Tile.Name + ".rgba16").c_str()
        );
    }

    fprintf(File, "const Texture *const %s[] = {\n", SkyboxName.c_str());

    std::vector<SkyboxTile> PtrList(80);

    int Rows = Tiles.size() / 8;

    for (int Row = 0; Row < Rows; Row++) {
        for (int Col = 0; Col < 8; Col++) {
            fprintf(File, "\t%s,\n", Tiles[Row * 8 + Col].Name.c_str());
        }

        fprintf(File, "\t%s,\n", Tiles[Row * 8 + 0].Name.c_str());
        fprintf(File, "\t%s,\n", Tiles[Row * 8 + 1].Name.c_str());
    }

    fprintf(File, "};\n");
}

bool ExportSkybox(N64Rom& Rom, LevelScript& Script, std::string& SkyboxName) {
    if (!ValidateMemAddr(0x0A000000) or !SkyboxExport) {
        return false;
    }

    SkyboxName = std::format("SkyboxCustom_{:08X}", SegmentedToROM(0x0A000000));

    printf("Exporting skybox %s\n", SkyboxName.c_str());

    const auto& SegData = SegmentData[0xA];
    s32 NumTiles = 8 * ((SegData.size() - 0x140) / 16384);

    printf("Num skybox tiles: %i\n", NumTiles);

    std::vector<SkyboxTile> Tiles = GetSkyboxTiles(0x0A000000, SegData, NumTiles);

    std::string LevelPath = "output/levels/" + Script.Name + "/";
    std::string SkyboxTilesPath = "output/levels/textures/skybox_tiles/";
    std::string SkyboxPath = LevelPath + "skybox.inc.c";

    fs::create_directories(LevelPath);
    ExportSkyboxTiles(Tiles, SkyboxTilesPath);

    FILE* File = fopen(SkyboxPath.c_str(), "w");

    if (!File) {
        printf("Can't open \"%s\"\n", SkyboxPath.c_str());
        return false;
    }

    ExportPtrList(File, Tiles, SkyboxName);

    fclose(File);

    return true;
}