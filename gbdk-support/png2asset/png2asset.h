#ifndef _PNG2ASSET_H
#define _PNG2ASSET_H

#define RGBA32_SZ 4 // RGBA 8:8:8:8 is 4 bytes per pixel

enum {
    SPR_NONE,
    SPR_8x8,
    SPR_8x16,
    SPR_16x16_MSX
};

#define BIT(VALUE, INDEX) (1 & ((VALUE) >> (INDEX)))

struct Tile
{
    vector< unsigned char > data;
    unsigned char pal;

    Tile(size_t size = 0) : data(size), pal(0) {}
    bool operator==(const Tile& t) const
    {
        return data == t.data && pal == t.pal;
    }

    const Tile& operator=(const Tile& t)
    {
        data = t.data;
        pal = t.pal;
        return *this;
    }

    enum PackMode {
        GB,
        NES,
        SGB,
        SMS,
        BPP1
    };

    vector< unsigned char > GetPackedData(PackMode pack_mode, int tile_w, int tile_h, int bpp) {
        vector< unsigned char > ret((tile_w / 8) * tile_h * bpp, 0);
        if(pack_mode == GB) {
            for(int j = 0; j < tile_h; ++j) {
                for(int i = 0; i < 8; ++ i) {
                    unsigned char col = data[8 * j + i];
                    ret[j * 2    ] |= BIT(col, 0) << (7 - i);
                    ret[j * 2 + 1] |= BIT(col, 1) << (7 - i);
                }
            }
        }
        if (pack_mode == NES) {
            for (int j = 0; j < tile_h; ++j) {
                for (int i = 0; i < 8; ++i) {
                    unsigned char col = data[8 * j + i];
                    ret[j] |= BIT(col, 0) << (7 - i);
                    ret[j+8] |= BIT(col, 1) << (7 - i);
                }
            }
        }
        else if(pack_mode == SGB)
        {
            for(int j = 0; j < tile_h; ++j) {
                for(int i = 0; i < 8; ++ i) {
                    unsigned char col = data[8 * j + i];
                    ret[j * 2    ] |= BIT(col, 0) << (7 - i);
                    ret[j * 2 + 1] |= BIT(col, 1) << (7 - i);
                    ret[(tile_h + j) * 2    ] |= BIT(col, 2) << (7 - i);
                    ret[(tile_h + j) * 2 + 1] |= BIT(col, 3) << (7 - i);
                }
            }
        }
        else if(pack_mode == SMS)
        {
            for(int j = 0; j < tile_h; ++j) {
                for(int i = 0; i < 8; ++ i) {
                    unsigned char col = data[8 * j + i];
                    ret[j * 4    ] |= BIT(col, 0) << (7 - i);
                    ret[j * 4 + 1] |= BIT(col, 1) << (7 - i);
                    ret[j * 4 + 2] |= BIT(col, 2) << (7 - i);
                    ret[j * 4 + 3] |= BIT(col, 3) << (7 - i);
                }
            }
        }
        else if(pack_mode == BPP1)
        {
            // Packs 8 pixel wide rows in order set by ExtractTile**()
            // Process all rows of pixels in the tile
            for(int j = 0; j < ((tile_w / 8) * tile_h); j++) {
                // Pack each row of 8 pixels into one byte
                for(int i = 0; i < 8; i++) {
                    unsigned char col = data[8 * j + i];
                    ret[j] |= BIT(col, 0) << (7 - i);
                }
            }
        }
        return ret;
    }
};

struct PNGImage
{
    vector< unsigned char > data; //data in indexed format
    unsigned int w;
    unsigned int h;

    // Default tile size
    int tile_w = 8;
    int tile_h = 16;

    // TODO: embed these instead of deriving them many places in the code
    // int attribute_w_factor = 1;
    // int attribute_h_factor = 1;
    // int get_attribute_tile_w() { tile_w * attribute_w_factor; }
    // int get_attribute_tile_h() { tile_h * attribute_h_factor; }

    size_t colors_per_pal;  // Number of colors per palette (ex: CGB has 4 colors per palette x 8 palettes total)
    size_t total_color_count; // Total number of colors across all palettes (palette_count x colors_per_pal)
    unsigned char* palette; //palette colors in RGBA (1 color == 4 bytes)

private:
    bool zero_palette = false;

public:
    unsigned char GetGBColor(int x, int y)
    {
        return data[w * y + x] % colors_per_pal;
    }


    // This needs separate tile_w and tile_h params since
    // MSX tile extraction uses it to pull out the 4 sub-tiles
    bool ExtractGBTile(int x, int y, int extract_tile_w, int extract_tile_h, Tile& tile, int buffer_offset)
    {
        // Set the palette to 0 when pals are not stored in tiles to allow tiles to be equal even when their palettes are different
        tile.pal = zero_palette ? 0 : data[w * y + x] >> 2;

        bool all_zero = true;
        for(int j = 0; j < extract_tile_h; ++ j)
        {
            for(int i = 0; i < extract_tile_w; ++i)
            {
                unsigned char color_idx = GetGBColor(x + i, y + j);
                tile.data[(j * extract_tile_w) + i + buffer_offset] = color_idx;
                all_zero = all_zero && (color_idx == 0);
            }
        }
        return !all_zero;
    }

    bool ExtractTile_MSX16x16(int x, int y, Tile& tile)
    {
        // MSX 16x16 sprite tiles are composed of four 8x8 tiles in this order UL, LL, UR, LR
        bool UL_notempty, LL_notempty, UR_notempty, LR_notempty;

        // Call these separately since otherwise some get optimized out during
        // runtime if any single one before it returns false
        UL_notempty = ExtractGBTile(x,     y,     8, 8, tile, 0);
        LL_notempty = ExtractGBTile(x,     y + 8, 8, 8, tile, ((8 *8) * 1));
        UR_notempty = ExtractGBTile(x + 8, y,     8, 8, tile, ((8 *8) * 2));
        LR_notempty = ExtractGBTile(x + 8, y + 8, 8, 8, tile, ((8 *8) * 3));
        return (UL_notempty || LL_notempty || UR_notempty || LR_notempty);
    }

    bool ExtractTile(int x, int y, Tile& tile, int sprite_mode, bool export_as_map, bool use_map_attributes)
    {
        // Set the palette to 0 when pals are not stored in tiles to allow tiles to be equal even when their palettes are different
        zero_palette = !(export_as_map && !use_map_attributes);

        if (sprite_mode == SPR_16x16_MSX)
            return ExtractTile_MSX16x16(x, y, tile);
        else
            return ExtractGBTile(x, y, tile_w, tile_h, tile, 0); // No buffer offset for normal tile extraction
    }
// private:
//     bool zero_palette = false;
};

#endif
