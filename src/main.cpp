/*/////////////////////////////////////////////////////////////////////////////
/// @summary 
/// @author Russell Klenk (contact@russellklenk.com)
///////////////////////////////////////////////////////////////////////////80*/

#define STB_DXT_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

/*////////////////
//   Includes   //
////////////////*/
#include <stdio.h>
#include <stdlib.h>
#include "stb_dxt.h"
#include "stb_image.h"
#include "stb_image_resize.h"
#include "lldatain.hpp"

/*/////////////////
//   Constants   //
/////////////////*/

/*///////////////////////
//   Local Functions   //
///////////////////////*/
/// @summary Prints the application header.
/// @param fp The output stream.
static void print_header(FILE *fp)
{
    fprintf(fp, "makedds: Convert images to DDS format.\n");
    fprintf(fp, "Public Domain software, use, modify and redistribute freely.\n");
    fprintf(fp, "Special thanks to Sean T. Barrett for his various stb_* libraries.\n");
    fprintf(fp, "\n");
}

/// @summary Prints program usage information.
/// @param fp The output stream.
static void print_usage(FILE *fp)
{
    fprintf(fp, "USAGE: makedds inputfile outputfile\n");
    fprintf(fp, "inputfile:  The path to the image or JSON file to load. Images may be\n");
    fprintf(fp, "            JPEG (non-progressive), PNG (8-bit-per-channel), TGA, GIF,\n");
    fprintf(fp, "            BMP (> 1bpp, non-RLE), PSD (composited view only, no extra\n");
    fprintf(fp, "            channels), HDR or PIC format.\n");
    fprintf(fp, "\n");
    fprintf(fp, "            The input file can also be a JSON file specifying advanced\n");
    fprintf(fp, "            conversion parameters to generate cubemaps, mipmaps, volume\n");
    fprintf(fp, "            images, and so on.\n");
    fprintf(fp, "\n");
    fprintf(fp, "outputfile: The path to the output .dds file.\n");
    fprintf(fp, "\n");
}

/// @summary Initializes the fields of a DDS_HEADER_DXT10 structure for a 2D 
/// image with non-premultiplied alpha and R8G8B8A8_UNORM format.
/// @param head The header structure to initialize.
static void init_dds_header_dxt10(data::dds_header_dxt10_t *head)
{
    head->Format    = data::DXGI_FORMAT_R8G8B8A8_UNORM;
    head->Dimension = data::D3D11_RESOURCE_DIMENSION_TEXTURE2D;
    head->Flags     = 0; // d3d11_resource_misc_flag_e
    head->ArraySize = 1;
    head->Flags2    = data::DDS_ALPHA_MODE_STRAIGHT;
}

/// @summary Initializes the fields of a DDS_PIXELFORMAT structure for a 32bpp
/// image with alpha and R8G8B8A8_UNORM format. Indicates a DX10 header is present.
/// @param ddspf The structure to initialize.
static void init_dds_pixelformat(data::dds_pixelformat_t *ddspf)
{
    ddspf->Size        = sizeof(data::dds_pixelformat_t);
    ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHA;
    ddspf->FourCC      = data::fourcc_le('D','X','1','0');
    ddspf->RGBBitCount = 32;
    ddspf->BitMaskR    = 0x000000FF;
    ddspf->BitMaskG    = 0x0000FF00;
    ddspf->BitMaskB    = 0x00FF0000;
    ddspf->BitMaskA    = 0xFF000000;
}

/// @summary Initializes the fields of a DDS_HEADER structure for a non-cubemap,
/// image, possibly with mipmaps, and in 32bpp R8G8B8A8_UNORM format.
/// @param head The header structure to initialize.
/// @param w The top-level image width, in pixels.
/// @param h The top-level image height, in pixels.
/// @param d The number of slices in the image. Specify 1 for a 2D image.
/// @param l The number of mipmap levels in the image. Specify 1 for no mipmaps.
static void init_dds_header(data::dds_header_t *head, int w, int h, int d, int l)
{
    head->Size      = sizeof(data::dds_header_t);
    head->Flags     = 0; // dds_header_flags_e
    head->Height    = uint32_t(w);
    head->Width     = uint32_t(h);
    head->Pitch     = uint32_t(w * 4);        // use data::dds_pitch(...)
    head->Depth     = uint32_t(d);
    head->Levels    = l > 1 ? uint32_t(l) : 0;
    head->Caps      = data::DDSCAPS_TEXTURE;
    head->Caps2     = data::DDSCAPS2_NONE;    // for cubemaps or volumes
    head->Caps3     = data::DDSCAPS3_NONE;
    head->Caps4     = data::DDSCAPS4_NONE;
    head->Reserved2 = 0;
    if (l > 1)
    {
        head->Caps |= data::DDSCAPS_COMPLEX | data::DDSCAPS_MIPMAP;
    }
    init_dds_pixelformat(&head->Format);
}

/*////////////////////////
//   Public Functions   //
////////////////////////*/
/// @summary Implements the entry point of the application.
/// @param argc The number of arguments passed on the command line.
/// @param argv An array of strings specifying command line arguments.
/// @return EXIT_SUCCESS or EXIT_FAILURE.
int main(int argc, char **argv)
{
    print_header(stdout);
    if (argc < 3)
    {
        print_usage(stdout);
        exit(EXIT_FAILURE);
    }

    int x, y, n;
    uint8_t *rgba_in = stbi_load(argv[1], &x, &y, &n, 4);

    data::dds_header_t dds;
    init_dds_header(&dds, x, y, 1, 1);
    data::dds_header_dxt10_t dx10;
    init_dds_header_dxt10(&dx10);

    FILE *fp = fopen(argv[2], "w+b");
    if (fp != NULL)
    {
        uint32_t magic = data::fourcc_le('D','D','S',' ');
        fwrite(&magic  , sizeof(uint32_t), 1, fp);
        fwrite(&dds    , sizeof(data::dds_header_t), 1, fp);
        fwrite(&dx10   , sizeof(data::dds_header_dxt10_t), 1, fp);
        fwrite(rgba_in , sizeof(uint8_t), x * y * 4, fp);
        fclose(fp);
    }

    stbi_image_free(rgba_in);

    exit(EXIT_SUCCESS);
}

