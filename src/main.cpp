/*/////////////////////////////////////////////////////////////////////////////
/// @summary Implements the entry point and most of the functionality of the
/// makedds utility, which generates DDS image files, including array, volume
/// and cubemap images, from other file formats. This utility relies heavily
/// on the stb_* libraries to do most of the heavy lifting.
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
#include <string.h>
#include "stb_dxt.h"
#include "stb_image.h"
#include "stb_image_resize.h"
#include "lldatain.hpp"

#ifdef _MSC_VER
#define stricmp_fn   _stricmp
#endif

#ifdef __GNUC__
#include <strings.h>
#define stricmp_fn   strcasecmp
#endif

/*/////////////////
//   Constants   //
/////////////////*/
/// @summary Define the maximum number of source images. Normally this wouldn't
/// be so large, but volume images can have many slices.
static size_t   const  MAX_SOURCE_IMAGES = 4096;

/// @summary An array of values corresponding to the ALPHAMODE_STRINGS array.
static uint32_t const  ALPHAMODE_VALUES   [] =
{
    data::DDS_ALPHA_MODE_STRAIGHT,
    data::DDS_ALPHA_MODE_PREMULTIPLIED,
    data::DDS_ALPHA_MODE_OPAQUE,
    data::DDS_ALPHA_MODE_CUSTOM
};

/// @summary An array of values corresponding to the DXGI_FORMAT_STRINGS array.
static uint32_t const  DXGI_FORMAT_VALUES [] =
{
    data::DXGI_FORMAT_R32G32B32A32_TYPELESS,
    data::DXGI_FORMAT_R32G32B32A32_FLOAT,
    data::DXGI_FORMAT_R32G32B32A32_UINT,
    data::DXGI_FORMAT_R32G32B32A32_SINT,
    data::DXGI_FORMAT_R32G32B32_TYPELESS,
    data::DXGI_FORMAT_R32G32B32_FLOAT,
    data::DXGI_FORMAT_R32G32B32_UINT,
    data::DXGI_FORMAT_R32G32B32_SINT,
    data::DXGI_FORMAT_R16G16B16A16_TYPELESS,
    data::DXGI_FORMAT_R16G16B16A16_FLOAT,
    data::DXGI_FORMAT_R16G16B16A16_UNORM,
    data::DXGI_FORMAT_R16G16B16A16_UINT,
    data::DXGI_FORMAT_R16G16B16A16_SNORM,
    data::DXGI_FORMAT_R16G16B16A16_SINT,
    data::DXGI_FORMAT_R32G32_TYPELESS,
    data::DXGI_FORMAT_R32G32_FLOAT,
    data::DXGI_FORMAT_R32G32_UINT,
    data::DXGI_FORMAT_R32G32_SINT,
    data::DXGI_FORMAT_R32G8X24_TYPELESS,
    data::DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    data::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
    data::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
    data::DXGI_FORMAT_R10G10B10A2_TYPELESS,
    data::DXGI_FORMAT_R10G10B10A2_UNORM,
    data::DXGI_FORMAT_R10G10B10A2_UINT,
    data::DXGI_FORMAT_R11G11B10_FLOAT,
    data::DXGI_FORMAT_R8G8B8A8_TYPELESS,
    data::DXGI_FORMAT_R8G8B8A8_UNORM,
    data::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    data::DXGI_FORMAT_R8G8B8A8_UINT,
    data::DXGI_FORMAT_R8G8B8A8_SNORM,
    data::DXGI_FORMAT_R8G8B8A8_SINT,
    data::DXGI_FORMAT_R16G16_TYPELESS,
    data::DXGI_FORMAT_R16G16_FLOAT,
    data::DXGI_FORMAT_R16G16_UNORM,
    data::DXGI_FORMAT_R16G16_UINT,
    data::DXGI_FORMAT_R16G16_SNORM,
    data::DXGI_FORMAT_R16G16_SINT,
    data::DXGI_FORMAT_R32_TYPELESS,
    data::DXGI_FORMAT_D32_FLOAT,
    data::DXGI_FORMAT_R32_FLOAT,
    data::DXGI_FORMAT_R32_UINT,
    data::DXGI_FORMAT_R32_SINT,
    data::DXGI_FORMAT_R24G8_TYPELESS,
    data::DXGI_FORMAT_D24_UNORM_S8_UINT,
    data::DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
    data::DXGI_FORMAT_X24_TYPELESS_G8_UINT,
    data::DXGI_FORMAT_R8G8_TYPELESS,
    data::DXGI_FORMAT_R8G8_UNORM,
    data::DXGI_FORMAT_R8G8_UINT,
    data::DXGI_FORMAT_R8G8_SNORM,
    data::DXGI_FORMAT_R8G8_SINT,
    data::DXGI_FORMAT_R16_TYPELESS,
    data::DXGI_FORMAT_R16_FLOAT,
    data::DXGI_FORMAT_D16_UNORM,
    data::DXGI_FORMAT_R16_UNORM,
    data::DXGI_FORMAT_R16_UINT,
    data::DXGI_FORMAT_R16_SNORM,
    data::DXGI_FORMAT_R16_SINT,
    data::DXGI_FORMAT_R8_TYPELESS,
    data::DXGI_FORMAT_R8_UNORM,
    data::DXGI_FORMAT_R8_UINT,
    data::DXGI_FORMAT_R8_SNORM,
    data::DXGI_FORMAT_R8_SINT,
    data::DXGI_FORMAT_A8_UNORM,
    data::DXGI_FORMAT_R1_UNORM,
    data::DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
    data::DXGI_FORMAT_R8G8_B8G8_UNORM,
    data::DXGI_FORMAT_G8R8_G8B8_UNORM,
    data::DXGI_FORMAT_BC1_TYPELESS,
    data::DXGI_FORMAT_BC1_UNORM,
    data::DXGI_FORMAT_BC1_UNORM_SRGB,
    data::DXGI_FORMAT_BC2_TYPELESS,
    data::DXGI_FORMAT_BC2_UNORM,
    data::DXGI_FORMAT_BC2_UNORM_SRGB,
    data::DXGI_FORMAT_BC3_TYPELESS,
    data::DXGI_FORMAT_BC3_UNORM,
    data::DXGI_FORMAT_BC3_UNORM_SRGB,
    data::DXGI_FORMAT_BC4_TYPELESS,
    data::DXGI_FORMAT_BC4_UNORM,
    data::DXGI_FORMAT_BC4_SNORM,
    data::DXGI_FORMAT_BC5_TYPELESS,
    data::DXGI_FORMAT_BC5_UNORM,
    data::DXGI_FORMAT_BC5_SNORM,
    data::DXGI_FORMAT_B5G6R5_UNORM,
    data::DXGI_FORMAT_B5G5R5A1_UNORM,
    data::DXGI_FORMAT_B8G8R8A8_UNORM,
    data::DXGI_FORMAT_B8G8R8X8_UNORM,
    data::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
    data::DXGI_FORMAT_B8G8R8A8_TYPELESS,
    data::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    data::DXGI_FORMAT_B8G8R8X8_TYPELESS,
    data::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
    data::DXGI_FORMAT_BC6H_TYPELESS,
    data::DXGI_FORMAT_BC6H_UF16,
    data::DXGI_FORMAT_BC6H_SF16,
    data::DXGI_FORMAT_BC7_TYPELESS,
    data::DXGI_FORMAT_BC7_UNORM,
    data::DXGI_FORMAT_BC7_UNORM_SRGB,
    data::DXGI_FORMAT_AYUV,
    data::DXGI_FORMAT_Y410,
    data::DXGI_FORMAT_Y416,
    data::DXGI_FORMAT_NV12,
    data::DXGI_FORMAT_P010,
    data::DXGI_FORMAT_P016,
    data::DXGI_FORMAT_420_OPAQUE,
    data::DXGI_FORMAT_YUY2,
    data::DXGI_FORMAT_Y210,
    data::DXGI_FORMAT_Y216,
    data::DXGI_FORMAT_NV11,
    data::DXGI_FORMAT_AI44,
    data::DXGI_FORMAT_IA44,
    data::DXGI_FORMAT_P8,
    data::DXGI_FORMAT_A8P8,
    data::DXGI_FORMAT_B4G4R4A4_UNORM
};

/// @summary An array of strings used in conjunction with DXGI_FORMAT_VALUES to
/// translate the string representation of a data::dxgi_format_e value into the
/// corresponding enumertion value.
static char     const *DXGI_FORMAT_STRINGS[] =
{
    "R32G32B32A32_TYPELESS",
    "R32G32B32A32_FLOAT",
    "R32G32B32A32_UINT",
    "R32G32B32A32_SINT",
    "R32G32B32_TYPELESS",
    "R32G32B32_FLOAT",
    "R32G32B32_UINT",
    "R32G32B32_SINT",
    "R16G16B16A16_TYPELESS",
    "R16G16B16A16_FLOAT",
    "R16G16B16A16_UNORM",
    "R16G16B16A16_UINT",
    "R16G16B16A16_SNORM",
    "R16G16B16A16_SINT",
    "R32G32_TYPELESS",
    "R32G32_FLOAT",
    "R32G32_UINT",
    "R32G32_SINT",
    "R32G8X24_TYPELESS",
    "D32_FLOAT_S8X24_UINT",
    "R32_FLOAT_X8X24_TYPELESS",
    "X32_TYPELESS_G8X24_UINT",
    "R10G10B10A2_TYPELESS",
    "R10G10B10A2_UNORM",
    "R10G10B10A2_UINT",
    "R11G11B10_FLOAT",
    "R8G8B8A8_TYPELESS",
    "R8G8B8A8_UNORM",
    "R8G8B8A8_UNORM_SRGB",
    "R8G8B8A8_UINT",
    "R8G8B8A8_SNORM",
    "R8G8B8A8_SINT",
    "R16G16_TYPELESS",
    "R16G16_FLOAT",
    "R16G16_UNORM",
    "R16G16_UINT",
    "R16G16_SNORM",
    "R16G16_SINT",
    "R32_TYPELESS",
    "D32_FLOAT",
    "R32_FLOAT",
    "R32_UINT",
    "R32_SINT",
    "R24G8_TYPELESS",
    "D24_UNORM_S8_UINT",
    "R24_UNORM_X8_TYPELESS",
    "X24_TYPELESS_G8_UINT",
    "R8G8_TYPELESS",
    "R8G8_UNORM",
    "R8G8_UINT",
    "R8G8_SNORM",
    "R8G8_SINT",
    "R16_TYPELESS",
    "R16_FLOAT",
    "D16_UNORM",
    "R16_UNORM",
    "R16_UINT",
    "R16_SNORM",
    "R16_SINT",
    "R8_TYPELESS",
    "R8_UNORM",
    "R8_UINT",
    "R8_SNORM",
    "R8_SINT",
    "A8_UNORM",
    "R1_UNORM",
    "R9G9B9E5_SHAREDEXP",
    "R8G8_B8G8_UNORM",
    "G8R8_G8B8_UNORM",
    "BC1_TYPELESS",
    "BC1_UNORM",
    "BC1_UNORM_SRGB",
    "BC2_TYPELESS",
    "BC2_UNORM",
    "BC2_UNORM_SRGB",
    "BC3_TYPELESS",
    "BC3_UNORM",
    "BC3_UNORM_SRGB",
    "BC4_TYPELESS",
    "BC4_UNORM",
    "BC4_SNORM",
    "BC5_TYPELESS",
    "BC5_UNORM",
    "BC5_SNORM",
    "B5G6R5_UNORM",
    "B5G5R5A1_UNORM",
    "B8G8R8A8_UNORM",
    "B8G8R8X8_UNORM",
    "R10G10B10_XR_BIAS_A2_UNORM",
    "B8G8R8A8_TYPELESS",
    "B8G8R8A8_UNORM_SRGB",
    "B8G8R8X8_TYPELESS",
    "B8G8R8X8_UNORM_SRGB",
    "BC6H_TYPELESS",
    "BC6H_UF16",
    "BC6H_SF16",
    "BC7_TYPELESS",
    "BC7_UNORM",
    "BC7_UNORM_SRGB",
    "AYUV",
    "Y410",
    "Y416",
    "NV12",
    "P010",
    "P016",
    "420_OPAQUE",
    "YUY2",
    "Y210",
    "Y216",
    "NV11",
    "AI44",
    "IA44",
    "P8",
    "A8P8",
    "B4G4R4A4_UNORM"
};

/// @summary An array of strings used in conjunction with ALPHAMODE_VALUES to
/// translate the string representation of a data::dds_alphamode_e value into
/// the corresponding enumertion value.
static char     const *ALPHAMODE_STRINGS  [] =
{
    "STRAIGHT",
    "PREMULTIPLIED",
    "OPAQUE",
    "CUSTOM"
};

/*//////////////////
//   Data Types   //
//////////////////*/
/// @summary Define the set of input parameters to the application.
struct dds_params_t
{
    size_t      Width;        /// Desired width, in pixels. Default = width of source.
    size_t      Height;       /// Desired height, in pixels. Default = height of source.
    size_t      BaseWidth;    /// The original width, in pixels. Always set to width of source.
    size_t      BaseHeight;   /// The original height, in pixels. Always set to height of source.
    size_t      MaxMipLevels; /// The maximum number of mipmap levels to generate. Default = 1.
    size_t      ArraySize;    /// Number of items in an array. Default depends on other options.
    uint32_t    Format;       /// One of data::dxgi_format_e. Default = format of source.
    uint32_t    AlphaMode;    /// One of data::dds_alphamode_e. Default = DDS_ALPHA_MODE_PREMULTIPLIED.
    bool        Mipmaps;      /// true if the output has mipmaps.
    bool        Cubemap;      /// true if the output is a cubemap or cubemap array. Default = false.
    bool        Volume;       /// true if the output is a volume image. Default = false.
    bool        ForcePow2;    /// true if the output dimensions should be powers of two. Default = false.
    char       *OutputFile;   /// The path or filename of the output file to generate.
    char       *JsonBuffer;   /// The buffer containing the input JSON data, or NULL.
    size_t      SourceCount;  /// The number of items in SourceFiles.
    size_t      SourceIndex;  /// The index of the item in SourceFiles being processed.
    char const *SourceFiles[MAX_SOURCE_IMAGES]; /// The filenames of all input files.
};

/// @summary Represents a single image slice loaded into memory by stb_image.
struct image_info_t
{
    void       *Pixels;       /// Pointer to the input buffer of pixels.
    int         Width;        /// The number of pixels per-row in the input image.
    int         Height;       /// The number of rows in the input image.
    int         Channels;     /// The number of channels in the input image.
    uint32_t    Format;       /// One of data::dxgi_format_e indicating the 'default' format.
    bool        HDR;          /// true if this is an HDR image and Pixels are float.
};

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

/// @summary Uses stb_image to load an image file from disk.
/// @param fp The stream to which any errors or warnings will be written.
/// @param infile The path of the input image file.
/// @param image On return, stores information about the loaded image.
/// @return true if the image was loaded, or false if an error occurred.
static bool load_image(FILE *fp, char const *infile, image_info_t &image)
{
    image.Pixels   = NULL;
    image.Width    = 0;
    image.Height   = 0;
    image.Channels = 0;
    image.Format   = data::DXGI_FORMAT_UNKNOWN;
    image.HDR      = false;

    if (stbi_is_hdr(infile))
    {
        int    w  = 0;
        int    h  = 0;
        int    n  = 0;
        float *px = stbi_loadf(infile, &w, &h, &n, 0);
        if (px != NULL)
        {
            switch (n)
            {
                case 1:
                    image.Format = data::DXGI_FORMAT_R32_FLOAT;
                    break;
                case 2:
                    image.Format = data::DXGI_FORMAT_R32G32_FLOAT;
                    break;
                case 3:
                    image.Format = data::DXGI_FORMAT_R32G32B32_FLOAT;
                    break;
                case 4:
                    image.Format = data::DXGI_FORMAT_R32G32B32A32_FLOAT;
                    break;
                default:
                    stbi_image_free(px);
                    fprintf(fp, "ERROR: Unexpected number of channels %d in HDR input \'%s\'.\n", n, infile);
                    return false;
            }
            image.Pixels   = px;
            image.Width    = w;
            image.Height   = h;
            image.Channels = n;
            image.HDR      = true;
            return true;
        }
        else
        {
            fprintf(fp, "ERROR: Unable to load HDR input \'%s\'.\n", infile);
            return false;
        }
    }
    else
    {
        int       w  = 0;
        int       h  = 0;
        int       n  = 0;
        uint8_t  *px = stbi_load(infile, &w, &h, &n, 0);
        if (n  == 3)
        {
            fprintf(fp, "WARNING: Re-loading 24-bpp file \'%s\' as 32-bpp. Export 32-bpp for best performance.\n", infile);
            stbi_image_free(px);
            px = stbi_load(infile, &w, &h, &n, 4);
            n  = 4; // force the path in the switch below.
        }
        if (px != NULL)
        {
            switch (n)
            {
                case 1:
                    image.Format = data::DXGI_FORMAT_R8_UNORM;
                    break;
                case 2:
                    image.Format = data::DXGI_FORMAT_R8G8_UNORM;
                    break;
                case 4:
                    image.Format = data::DXGI_FORMAT_R8G8B8A8_UNORM;
                    break;
                default:
                    fprintf(fp, "ERROR: Unexpected number of channels %d in LDR input \'%s\'.\n", n, infile);
                    return false;
            }
            image.Pixels   = px;
            image.Width    = w;
            image.Height   = h;
            image.Channels = n;
            image.HDR      = false;
            return true;
        }
        else
        {
            fprintf(fp, "ERROR: Unable to load LDR input \'%s\'.\n", infile);
            return false;
        }
    }
}

/// @summary Resizes an image into a new buffer. The format and number of 
/// channels remain the same as the input image buffer.
/// @param fp The output stream to which errors and warnings will be written.
/// @param output On return, describes the output image.
/// @param input Describes the input image.
/// @param new_width The number of columns in the scaled image.
/// @param new_height The number of rows in the scaled image.
/// @return true if the output image was generated.
static bool resize_image(FILE *fp, image_info_t &output, image_info_t const &input, size_t new_width, size_t new_height)
{
    size_t bpc    = input.HDR ? sizeof(float) : sizeof(uint8_t);
    size_t nbytes = new_width * new_height * size_t(input.Channels) * bpc;
    void  *pixels = malloc(nbytes);
    if (pixels == NULL)
    {
        fprintf(fp, "ERROR: Unable to allocate %u bytes for resized image.\n", unsigned(nbytes));
        output.Pixels   = NULL;
        output.Width    = 0;
        output.Height   = 0;
        output.Channels = 0;
        output.Format   = data::DXGI_FORMAT_UNKNOWN;
        output.HDR      = false;
        return false;
    }

    output.Pixels   = pixels;
    output.Width    = int(new_width);
    output.Height   = int(new_height);
    output.Channels = input.Channels;
    output.Format   = input.Format;
    output.HDR      = input.HDR;
    if (input.HDR)
    {   // use the default upsample/downsample filters.
        // edge mode is clamp-to-edge.
        // colorspace is linear.
        stbir_resize_float(
            (float const*)  input.Pixels,  input.Width,  input.Height, 0, 
            (float      *) output.Pixels, output.Width, output.Height, 0, output.Channels);
    }
    else
    {   // use the default upsample/downsample filters.
        // edge mode is clamp-to-edge.
        // colorspace is sRGB.
        stbir_resize_uint8_srgb(
            (uint8_t const*)  input.Pixels,  input.Width,  input.Height, 0,
            (uint8_t      *) output.Pixels, output.Width, output.Height, 0, output.Channels, 
            output.Channels == 4 ? 3 : STBIR_ALPHA_CHANNEL_NONE, 0);
    }
    return true;
}

/// @summary Frees an image loaded by stb_image.
/// @param image The image state to free and re-initialize.
static void free_image(image_info_t &image)
{
    if (image.Pixels != NULL)
    {
        stbi_image_free(image.Pixels);
        image.Pixels  = NULL;
    }
    image.Width    = 0;
    image.Height   = 0;
    image.Channels = 0;
}

/// @summary Find the end of a volume and directory information portion of a path.
/// @param path The path string to search.
/// @param out_pathlen On return, indicates the number of bytes in the volume and
/// directory information of the path string. If the input path has no volume or
/// directory information, this value will be set to zero.
/// @param out_strlen On return, indicates the number of bytes in the input path,
/// not including the trailing zero byte.
/// @return A pointer to one past the last volume or directory separator, if present;
/// otherwise, the input pointer path.
static char const* pathend(char const *path, size_t &out_pathlen, size_t &out_strlen)
{
    if (path == NULL)
    {
        out_pathlen = 0;
        out_strlen  = 0;
        return path;
    }

    char        ch   = 0;
    char const *last = path;
    char const *iter = path;
    while ((ch = *iter++) != 0)
    {
        if (ch == ':' || ch == '\\' || ch == '/')
            last = iter;
    }
    out_strlen  = size_t(iter - path - 1);
    out_pathlen = size_t(last - path);
    return last;
}

/// @summary Find the extension part of a filename or path string.
/// @param path The path string to search; ideally just the filename portion.
/// @param out_extlen On return, indicates the number of bytes of extension information.
/// @return A pointer to the first character of the extension. Check the value of
/// out_extlen to be sure that there is extension information.
static char const* extpart(char const *path, size_t &out_extlen)
{
    if (path == NULL)
    {
        out_extlen = 0;
        return path;
    }

    char        ch    = 0;
    char const *last  = path;
    char const *iter  = path;
    while ((ch = *iter++) != 0)
    {
        if (ch == '.')
            last = iter;
    }
    if (last != path)
    {   // we found an extension separator somewhere in the input path.
        // @note: this also filters out the case of ex. path = '.gitignore'.
        out_extlen = size_t(iter - last - 1);
    }
    else
    {
        // no extension part is present in the input path.
        out_extlen = 0;
    }
    return last;
}

/// @summary Initializes a DDS output parameters structure with default values.
/// @param params The DDS output parameters to populate.
/// @param buffer The JSON input buffer, or NULL.
static void init_params(dds_params_t &params, char *buffer)
{
    params.Width         = 0;
    params.Height        = 0;
    params.BaseWidth     = 0;
    params.BaseHeight    = 0;
    params.MaxMipLevels  = 1;
    params.ArraySize     = 1;
    params.Format        = data::DXGI_FORMAT_UNKNOWN;
    params.AlphaMode     = data::DDS_ALPHA_MODE_UNKNOWN;
    params.Mipmaps       = false;
    params.Cubemap       = false;
    params.Volume        = false;
    params.ForcePow2     = false;
    params.OutputFile    = NULL;
    params.JsonBuffer    = buffer;
    params.SourceCount   = 0;
    params.SourceIndex   = 0;
}

/// @summary Processes an input node of a JSON document.
/// @param fp The stream to which errors will be written.
/// @param node The JSON document node to process.
/// @param params The DDS output parameters to populate.
static bool process_json_node(FILE *fp, data::json_item_t *node, dds_params_t &params)
{
    if (node == NULL)
        return true;

    switch (node->ValueType)
    {
        case data::JSON_TYPE_OBJECT:
            {   // we expect a top-level object, but that's it.
                data::json_item_t *child = node->FirstChild;
                while (child != NULL)
                {   // recursively process the fields that make up the object.
                    if (!process_json_node(fp, child, params))
                        return false;
                    child = child->Next;
                }
            }
            return true;

        case data::JSON_TYPE_ARRAY:
            {   // we expect only the SourceFiles element to be an array.
                if (0 != stricmp_fn(node->Key, "SourceFiles"))
                {
                    fprintf(fp, "WARNING: Unexpected array element \'%s\'.", node->Key);
                    return true;
                }
                params.SourceCount         = 0;
                data::json_item_t *element = node->FirstChild;
                while (element != NULL)
                {   // all child elements must be strings.
                    if (element->ValueType != data::JSON_TYPE_STRING)
                    {
                        fprintf(fp, "WARNING: Expect only strings in SourceFiles array; item %u will be ignored.\n", unsigned(params.SourceCount));
                        element = element->Next;
                        continue;
                    }
                    if (params.SourceCount == MAX_SOURCE_IMAGES)
                    {
                        fprintf(fp, "WARNING: A maximum of %u source images are supported.\n", unsigned(MAX_SOURCE_IMAGES));
                        break;
                    }
                    params.SourceFiles[params.SourceCount++] = element->Value.string;
                    element = element->Next;
                }
            }
            return true;

        case data::JSON_TYPE_STRING:
            {   // we expect 'Format' and 'AlphaMode' to be strings.
                if (0 != stricmp_fn(node->Key, "Format"   ) &&
                    0 != stricmp_fn(node->Key, "AlphaMode"))
                {
                    fprintf(fp, "ERROR: Unexpected string field \'%s\'.\n", node->Key);
                    return false;
                }

                if (0 == stricmp_fn(node->Key, "Format"))
                {
                    bool         match = false;
                    size_t const nvals = sizeof(DXGI_FORMAT_VALUES) / sizeof(DXGI_FORMAT_VALUES[0]);
                    for (size_t i = 0; i < nvals; ++i)
                    {
                        if (0 == stricmp_fn(node->Value.string, DXGI_FORMAT_STRINGS[i]))
                        {
                            params.Format = DXGI_FORMAT_VALUES[i];
                            match = true;
                            break;
                        }
                    }
                    if (!match)
                    {
                        fprintf(fp, "ERROR: Unknown DXGI_FORMAT_ value \'%s\'.\n", node->Value.string);
                        return false;
                    }
                }
                else if (0 == stricmp_fn(node->Key, "AlphaMode"))
                {
                    bool         match = false;
                    size_t const nvals = sizeof(ALPHAMODE_VALUES) / sizeof(ALPHAMODE_VALUES[0]);
                    for (size_t i = 0; i < nvals; ++i)
                    {
                        if (0 == stricmp_fn(node->Value.string, ALPHAMODE_STRINGS[i]))
                        {
                            params.AlphaMode = ALPHAMODE_VALUES[i];
                            match = true;
                            break;
                        }
                    }
                    if (!match)
                    {
                        fprintf(fp, "ERROR: Unknown DDS_ALPHA_MODE_ value \'%s\'.\n", node->Value.string);
                        return false;
                    }
                }
                else fprintf(fp, "WARNING: Unexpected string field \'%s\'.\n", node->Key);
            }
            return true;

        case data::JSON_TYPE_INTEGER:
            {   // Width, Height, MaxMipLevels, and ArraySize may be integers.
                     if (0 == stricmp_fn(node->Key, "Width"       )) params.Width        = size_t(node->Value.integer);
                else if (0 == stricmp_fn(node->Key, "Height"      )) params.Height       = size_t(node->Value.integer);
                else if (0 == stricmp_fn(node->Key, "MaxMipLevels")) params.MaxMipLevels = size_t(node->Value.integer);
                else if (0 == stricmp_fn(node->Key, "ArraySize"   )) params.ArraySize    = size_t(node->Value.integer);
                else fprintf(fp, "WARNING: Unexpected Integer field \'%s\'.\n", node->Key);
            }
            return true;

        case data::JSON_TYPE_NUMBER:
            {   // No Number values are expected currently.
                fprintf(fp, "WARNING: Unexpected Number field \'%s\'.\n", node->Key);
            }
            return true;

        case data::JSON_TYPE_BOOLEAN:
            {
                // ForcePow2, Cubemap, Volume and Mipmaps may be booleans.
                     if (0 == stricmp_fn(node->Key, "Cubemap"  )) params.Cubemap   = node->Value.boolean;
                else if (0 == stricmp_fn(node->Key, "Mipmaps"  )) params.Mipmaps   = node->Value.boolean;
                else if (0 == stricmp_fn(node->Key, "Volume"   )) params.Volume    = node->Value.boolean;
                else if (0 == stricmp_fn(node->Key, "ForcePow2")) params.ForcePow2 = node->Value.boolean;
                else fprintf(fp, "WARNING: Unexpected Boolean field \'%s\'.\n" , node->Key);
            }
            return true;

        case data::JSON_TYPE_NULL:
            {   // any supported value may be null, and assumes the default.
                     if (0 == stricmp_fn(node->Key, "Cubemap"     )) params.Cubemap      = false;
                else if (0 == stricmp_fn(node->Key, "Mipmaps"     )) params.Mipmaps      = false;
                else if (0 == stricmp_fn(node->Key, "Volume"      )) params.Volume       = false;
                else if (0 == stricmp_fn(node->Key, "ForcePow2"   )) params.ForcePow2    = false;
                else if (0 == stricmp_fn(node->Key, "Width"       )) params.Width        = 0;
                else if (0 == stricmp_fn(node->Key, "Height"      )) params.Height       = 0;
                else if (0 == stricmp_fn(node->Key, "Format"      )) params.Format       = data::DXGI_FORMAT_B8G8R8A8_UNORM;
                else if (0 == stricmp_fn(node->Key, "AlphaMode"   )) params.AlphaMode    = data::DDS_ALPHA_MODE_PREMULTIPLIED;
                else if (0 == stricmp_fn(node->Key, "MaxMipLevels")) params.MaxMipLevels = 1;
                else if (0 == stricmp_fn(node->Key, "ArraySize"   )) params.ArraySize    = 1;
                else if (0 == stricmp_fn(node->Key, "SourceFiles" ))
                {
                    fprintf(fp, "ERROR: SourceFiles cannot be null.\n");
                    return false;
                }
                else fprintf(fp, "WARNING: Unexpected null field \'%s\'.\n", node->Key);
            }
            return true;

        default:
            break;
    }
    return false;
}

/// @summary Parses a JSON string to extract DDS output parameters.
/// @param fp The stream to which parsing errors will be written.
/// @param json Pointer to a NULL-terminated buffer containing the input JSON.
/// @param json_size The number of bytes of JSON to parse, or 0 to parse the entire string.
/// @param free_buffer Specify true to save a reference to the input JSON buffer
/// so it can be freed when program execution has completed.
/// @param params On return, stores image processing parameters loaded from the JSON.
/// @param image On return, stores information about the loaded image. If no image was 
/// loaded, the image_info_t::Pixels field will be NULL; this does not indicate an error.
/// @return true if the JSON was successfully parsed.
static bool params_from_json(FILE *fp, char *json, size_t json_size, bool free_buffer, dds_params_t &params, image_info_t &image)
{
    if (json == NULL)
    {
        init_params(params, free_buffer ? json : NULL);
        fprintf(fp, "ERROR: Unable to load the input file.\n");
        return false;
    }
    if (json_size == 0)
    {   // calculate the size for the caller.
        json_size  = strlen(json);
    }

    data::json_item_t  *root = NULL;
    data::json_error_t  error;
    if (!data::json_parse(json, json_size, NULL, &root, &error))
    {
        init_params(params, free_buffer ? json : NULL);
        fprintf(fp, "ERROR: Unable to parse input JSON:\n");
        fprintf(fp, "%s at line %u:\n", error.Description, unsigned(error.Line));
        fprintf(fp, "  %s\n\n", error.Position);
        return false;
    }

    // perform the actual document parsing:
    init_params(params, free_buffer  ? json : NULL);
    if (!process_json_node(fp, root, params))
    {
        data::json_free(root, NULL);
        return false;
    }

    // free all of the JSON nodes; they are no longer needed.
    data::json_free(root, NULL);

    // perform some additional parameter validation.
    if (params.Cubemap && (params.SourceCount % 6) != 0)
    {
        fprintf(fp, "ERROR: The number of SourceFiles specified for a cubemap must be a multiple of six, got %u.\n", unsigned(params.SourceCount));
        return false;
    }
    if (params.Cubemap &&  params.SourceCount > 6)
    {   // this is an array of cubemap images.
        params.ArraySize = params.SourceCount / 6;
    }
    if (params.Volume)
    {   // the array size must be set to 1. volume arrays are not supported.
        params.ArraySize = 1;
    }
    if (params.Volume == false && params.Cubemap == false && params.ArraySize <= 1)
    {   // default to the number of source files specified.
        params.ArraySize = params.SourceCount;
    }
    if (params.Volume && params.Mipmaps)
    {
        fprintf(fp, "WARNING: Mipmaps are not currently supported for volume images and will not be output.\n");
        params.Mipmaps  = false;
        params.MaxMipLevels = 1;
    }

    if (params.SourceCount == 1 && !params.Volume)
    {   // if there's only one source file, load it now.
        if (load_image(fp, params.SourceFiles[0], image) == false)
        {   // additional information is printed out by load_image().
            return false;
        }

        if (params.Width  == 0) params.Width  = size_t(image.Width);
        if (params.Height == 0) params.Height = size_t(image.Height);
        params.BaseWidth   = size_t(image.Width);
        params.BaseHeight  = size_t(image.Height);
        params.SourceIndex = 1;
        if (params.Format == data::DXGI_FORMAT_UNKNOWN)
        {   // use the default format for the image.
            params.Format  = image.Format;
        }
    }
    else
    {   // more than one image, so defer loading until we generate the DDS.
        params.SourceIndex = 1;
        image.Pixels       = NULL;
        image.Width        = 0;
        image.Height       = 0;
        image.Channels     = 0;
        image.Format       = data::DXGI_FORMAT_UNKNOWN;
        image.HDR          = false;
    }
    return true;
}

/// @summary Fills out basic image processing parameters based on the file 
/// extension of the input file.
/// @param fp The stream to which error output will be written.
/// @param inpath The path, filename and extension of the input file.
/// @param params The image processing parameters to populate.
/// @param image On return, stores information about the loaded image. If no image was 
/// loaded, the image_info_t::Pixels field will be NULL; this does not indicate an error.
/// @return true if the input file is a recognized format.
static bool params_from_path(FILE *fp, char const *inpath, dds_params_t &params, image_info_t &image)
{
    size_t      ext_len = 0;
    char const *ext_str = extpart(inpath, ext_len);

    if (ext_len == 0)
    {
        fprintf(fp, "ERROR: No file extension on \'%s\'.\nUnable to determine file type.\n", inpath);
        return false;
    }

    if (0 == stricmp_fn(ext_str, "json"))
    {   // JSON files can describe complex images (cubemaps, volumes, etc.)
        size_t  nb = 0;
        char *json = data::load_text(inpath, &nb, NULL);
        return params_from_json(fp, json, nb, true, params, image);
    }
    if (0 == stricmp_fn(ext_str, "png" ) || 
        0 == stricmp_fn(ext_str, "jpg" ) || 
        0 == stricmp_fn(ext_str, "jpeg") || 
        0 == stricmp_fn(ext_str, "tga" ) || 
        0 == stricmp_fn(ext_str, "psd" ) || 
        0 == stricmp_fn(ext_str, "bmp" ) || 
        0 == stricmp_fn(ext_str, "hdr" ) || 
        0 == stricmp_fn(ext_str, "pic" ) || 
        0 == stricmp_fn(ext_str, "gif" ))
    {   // raw image files can describe only simple images.
        // LDR images are always R8[G8B8A8]_UNORM. HDR images are always R32[G32B32A32]_FLOAT.
        // if you need something other than this, use a JSON file and specify the format.
        if (load_image(fp, inpath, image) == false)
        {   // load_image() outputs error information.
            return false;
        }

        params.Width          = size_t(image.Width);
        params.Height         = size_t(image.Height);
        params.BaseWidth      = size_t(image.Width);
        params.BaseHeight     = size_t(image.Height);
        params.MaxMipLevels   = 1;
        params.ArraySize      = 1;
        params.Format         = image.Format;
        params.AlphaMode      = image.Channels == 4 ? 
                                  data::DDS_ALPHA_MODE_PREMULTIPLIED : 
                                  data::DDS_ALPHA_MODE_OPAQUE;
        params.Mipmaps        = false;
        params.Cubemap        = false;
        params.Volume         = false;
        params.ForcePow2      = false;
        params.OutputFile     = NULL;
        params.JsonBuffer     = NULL;
        params.SourceCount    = 1;
        params.SourceIndex    = 1;
        params.SourceFiles[0] = inpath;
        return true;
    }

    fprintf(fp, "ERROR: Unrecognized file extension \'%s\' on \'%s\'.\n", ext_str, inpath);
    return false;
}

/// @summary Calculate a power-of-two value greater than or equal to a given value.
/// @param The input value, which may or may not be a power of two.
/// @param min The minimum power-of-two value, which must also be non-zero.
/// @return A power-of-two that is greater than or equal to value.
static inline size_t pow2_ge(size_t value, size_t min)
{
    size_t x = min;
    while (x < value)
        x  <<= 1;
    return x;
}

/// @summary Applies any command-line modifiers and calculates default values
/// to force power-of-two dimensions and calculate the number of mipmap levels.
/// @param argc The number of command-line arguments.
/// @param argv An array of NULL-terminated strings specifying command-line arguments.
/// @param params The image processing parameters to update.
static void modify_params(int argc, char **argv, dds_params_t &params)
{
    // look for the --mipmap and --pow2 command line arguments and 
    // modify the params structure.
    for (int i = 0; i < argc; ++i)
    {
        if (0 == stricmp_fn(argv[i], "--mipmap"))
        {
            params.Mipmaps      = true;
            params.MaxMipLevels = 0;
            continue;
        }
        if (0 == stricmp_fn(argv[i], "--pow2"))
        {
            params.ForcePow2 = true;
            continue;
        }
    }
    if (params.ForcePow2 && params.Width != 0 && params.Height != 0)
    {   // set Width and Height to the nearest power of 2.
        params.Width  = pow2_ge(params.Width , 1);
        params.Height = pow2_ge(params.Height, 1);
    }
    if (params.MaxMipLevels == 0)
    {   // compute all the way down to 1x1.
        size_t lw = params.Width;
        size_t lh = params.Height;
        while (lw > 1 || lh > 1)
        {
            params.MaxMipLevels++;
            lw >>= 1; if (lw == 0) lw = 1;
            lh >>= 1; if (lh == 0) lh = 1;
        }
        // include the base level in the count.
        params.MaxMipLevels++;
    }
}

/// @summary Initializes the fields of a DDS_HEADER_DXT10 structure based on 
/// the current image processing parameters.
/// @param head The header structure to initialize.
/// @param params The image processing parameters.
static void init_dds_header_dxt10(data::dds_header_dxt10_t *head, dds_params_t const &params)
{
    if (params.Volume)
    {
        head->Dimension = data::D3D11_RESOURCE_DIMENSION_TEXTURE3D;
        head->Flags     = 0;
    }
    else if (params.Cubemap)
    {
        head->Dimension = data::D3D11_RESOURCE_DIMENSION_TEXTURE2D;
        head->Flags     = data::D3D11_RESOURCE_MISC_TEXTURECUBE;
    }
    else if (params.BaseWidth == 1 || params.BaseHeight == 1)
    {
        head->Dimension = data::D3D11_RESOURCE_DIMENSION_TEXTURE1D;
        head->Flags     = 0;
    }
    else
    {
        head->Dimension = data::D3D11_RESOURCE_DIMENSION_TEXTURE2D;
        head->Flags     = 0;
    }
    head->Format    = params.Format;
    head->ArraySize = params.ArraySize;
    head->Flags2    = params.AlphaMode;
}

/// @summary Initializes the fields of a DDS_PIXELFORMAT structure based on 
/// the image processing parameters. Always indicates the presence of a DX10 header.
/// @param ddspf The structure to initialize.
/// @param params The image processing parameters.
static void init_dds_pixelformat(data::dds_pixelformat_t *ddspf, dds_params_t const &params)
{
    ddspf->Size        = sizeof(data::dds_pixelformat_t);
    ddspf->Flags       = data::DDPF_FOURCC;
    ddspf->FourCC      = data::fourcc_le('D','X','1','0');
    
    switch (params.Format)
    {
        case data::DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case data::DXGI_FORMAT_R10G10B10A2_UNORM:
        case data::DXGI_FORMAT_R10G10B10A2_UINT:
        case data::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x000003FFU;
            ddspf->BitMaskG    = 0x000FFC00U;
            ddspf->BitMaskB    = 0x3FF00000U;
            ddspf->BitMaskA    = 0xC0000000U;
            break;
        case data::DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case data::DXGI_FORMAT_R8G8B8A8_UNORM:
        case data::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case data::DXGI_FORMAT_R8G8B8A8_UINT:
        case data::DXGI_FORMAT_R8G8B8A8_SNORM:
        case data::DXGI_FORMAT_R8G8B8A8_SINT:
            ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x000000FFU;
            ddspf->BitMaskG    = 0x0000FF00U;
            ddspf->BitMaskB    = 0x00FF0000U;
            ddspf->BitMaskA    = 0xFF000000U;
            break;
        case data::DXGI_FORMAT_R16G16_TYPELESS:
            ddspf->Flags       = data::DDPF_RGB;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x0000FFFFU;
            ddspf->BitMaskG    = 0xFFFF0000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x00000000U;
            break;
        case data::DXGI_FORMAT_R32_TYPELESS:
        case data::DXGI_FORMAT_R32_UINT:
        case data::DXGI_FORMAT_R32_SINT:
            ddspf->Flags       = data::DDPF_RGB;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0xFFFFFFFFU;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x00000000U;
            break;
        case data::DXGI_FORMAT_R24G8_TYPELESS:
            ddspf->Flags       = data::DDPF_LUMINANCE | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x00FFFFFFU;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0xFF000000U;
            break;
        case data::DXGI_FORMAT_R8G8_TYPELESS:
        case data::DXGI_FORMAT_R8G8_UNORM:
        case data::DXGI_FORMAT_R8G8_UINT:
        case data::DXGI_FORMAT_R8G8_SNORM:
        case data::DXGI_FORMAT_R8G8_SINT:
            ddspf->Flags       = data::DDPF_LUMINANCE | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 16;
            ddspf->BitMaskR    = 0x000000FFU;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x0000FF00U;
            break;
        case data::DXGI_FORMAT_A8P8:
            ddspf->Flags       = data::DDPF_LUMINANCE | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 16;
            ddspf->BitMaskR    = 0x0000FF00U;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x000000FFU;
            break;
        case data::DXGI_FORMAT_R16_TYPELESS:
        case data::DXGI_FORMAT_R16_UNORM:
        case data::DXGI_FORMAT_R16_UINT:
        case data::DXGI_FORMAT_R16_SNORM:
        case data::DXGI_FORMAT_R16_SINT:
            ddspf->Flags       = data::DDPF_LUMINANCE;
            ddspf->RGBBitCount = 16;
            ddspf->BitMaskR    = 0x0000FFFFU;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x00000000U;
            break;
        case data::DXGI_FORMAT_R8_TYPELESS:
        case data::DXGI_FORMAT_R8_UNORM:
        case data::DXGI_FORMAT_R8_UINT:
        case data::DXGI_FORMAT_R8_SNORM:
        case data::DXGI_FORMAT_R8_SINT:
        case data::DXGI_FORMAT_A8_UNORM:
        case data::DXGI_FORMAT_P8:
            ddspf->Flags       = data::DDPF_LUMINANCE;
            ddspf->RGBBitCount = 8;
            ddspf->BitMaskR    = 0x000000FFU;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x00000000U;
            break;
        case data::DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            ddspf->Flags       = data::DDPF_RGB;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x00FFFFFFU;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x00000000U;
            break;
        case data::DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            ddspf->Flags       = data::DDPF_ALPHA;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x00000000U;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0xFF000000U;
            break;
        case data::DXGI_FORMAT_R1_UNORM:
            ddspf->Flags       = data::DDPF_ALPHA;
            ddspf->RGBBitCount = 1;
            ddspf->BitMaskR    = 0x00000000U;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x00000001U;
            break;
        case data::DXGI_FORMAT_R8G8_B8G8_UNORM:
            ddspf->Flags       = data::DDPF_RGB;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x000000FFU;
            ddspf->BitMaskG    = 0x0000FF00U;
            ddspf->BitMaskB    = 0x00FF0000U;
            ddspf->BitMaskA    = 0x0000FF00U;
            break;
        case data::DXGI_FORMAT_G8R8_G8B8_UNORM:
            ddspf->Flags       = data::DDPF_RGB;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x0000FF00U;
            ddspf->BitMaskG    = 0x000000FFU;
            ddspf->BitMaskB    = 0x0000FF00U;
            ddspf->BitMaskA    = 0x00FF0000U;
            break;
        case data::DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case data::DXGI_FORMAT_B8G8R8A8_UNORM:
        case data::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x00FF0000U;
            ddspf->BitMaskG    = 0x0000FF00U;
            ddspf->BitMaskB    = 0x000000FFU;
            ddspf->BitMaskA    = 0xFF000000U;
            break;
        case data::DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case data::DXGI_FORMAT_B8G8R8X8_UNORM:
        case data::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            ddspf->Flags       = data::DDPF_RGB;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x00FF0000U;
            ddspf->BitMaskG    = 0x0000FF00U;
            ddspf->BitMaskB    = 0x000000FFU;
            ddspf->BitMaskA    = 0x00000000U;
            break;
        case data::DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 32;
            ddspf->BitMaskR    = 0x000001FFU;
            ddspf->BitMaskG    = 0x0003FE00U;
            ddspf->BitMaskB    = 0x07FC0000U;
            ddspf->BitMaskA    = 0xF8000000U;
            break;
        case data::DXGI_FORMAT_B5G6R5_UNORM:
            ddspf->Flags       = data::DDPF_RGB;
            ddspf->RGBBitCount = 16;
            ddspf->BitMaskR    = 0x0000F800U;
            ddspf->BitMaskG    = 0x000007E0U;
            ddspf->BitMaskB    = 0x00000001U;
            ddspf->BitMaskA    = 0x00000000U;
            break;
        case data::DXGI_FORMAT_B5G5R5A1_UNORM:
            ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 16;
            ddspf->BitMaskR    = 0x00007C00U;
            ddspf->BitMaskG    = 0x000003E0U;
            ddspf->BitMaskB    = 0x00000001U;
            ddspf->BitMaskA    = 0x00008000U;
            break;
        case data::DXGI_FORMAT_B4G4R4A4_UNORM:
            ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHAPIXELS;
            ddspf->RGBBitCount = 16;
            ddspf->BitMaskR    = 0x00000F00U;
            ddspf->BitMaskG    = 0x000000F0U;
            ddspf->BitMaskB    = 0x0000000FU;
            ddspf->BitMaskA    = 0x0000F000U;
            break;
        default:
            // don't specify any information.
            ddspf->Flags       = data::DDPF_NONE;
            ddspf->RGBBitCount = 0;
            ddspf->BitMaskR    = 0x00000000U;
            ddspf->BitMaskG    = 0x00000000U;
            ddspf->BitMaskB    = 0x00000000U;
            ddspf->BitMaskA    = 0x00000000U;
            break;
    }
}

/// @summary Initializes the fields of a DDS_HEADER structure based on the 
/// current image processing parameters.
/// @param head The header structure to initialize.
/// @param params The image processing parameters.
static void init_dds_header(data::dds_header_t *head, dds_params_t const &params)
{
    uint32_t flags   = data::DDSD_CAPS        | data::DDSD_HEIGHT      | 
                       data::DDSD_WIDTH       | data::DDSD_PIXELFORMAT | 
                       data::DDSD_MIPMAPCOUNT;
    if (params.Volume) flags |= data::DDSD_DEPTH;
    if (data::dds_block_compressed(params.Format)) flags |= data::DDSD_LINEARSIZE;
    else flags |= data::DDSD_PITCH;

    uint32_t caps    = data::DDSCAPS_TEXTURE;
    if (params.Mipmaps) caps |= data::DDSCAPS_COMPLEX | data::DDSCAPS_MIPMAP;
    if (params.Cubemap) caps |= data::DDSCAPS_COMPLEX;

    uint32_t caps2   = data::DDSCAPS2_NONE;
    if (params.Cubemap)caps2 |= data::DDS_CUBEMAP_ALLFACES;
    if (params.Volume) caps2 |= data::DDSCAPS2_VOLUME;

    head->Size      = sizeof(data::dds_header_t);
    head->Flags     = flags;
    head->Height    = uint32_t(params.Height);
    head->Width     = uint32_t(params.Width);
    head->Pitch     = data::dds_pitch(params.Format, params.Width);
    head->Depth     = uint32_t(params.SourceCount);
    head->Levels    = uint32_t(params.MaxMipLevels);
    head->Caps      = caps;
    head->Caps2     = caps2;
    head->Caps3     = data::DDSCAPS3_NONE;
    head->Caps4     = data::DDSCAPS4_NONE;
    head->Reserved2 = 0;
    init_dds_pixelformat(&head->Format, params);
}

/// @summary Loads a specific source image from disk.
/// @param fp The output stream to which errors and warnings will be written.
/// @param params Image processing parameters.
/// @param index The zero-based index of the source image to load.
/// @param image On return, stores information about the loaded image.
/// @return true if the image was loaded.
static bool load_source(FILE *fp, dds_params_t &params, size_t index, image_info_t &image)
{
    char const *infile  = params.SourceFiles[index];
    return load_image(fp, infile, image);
}

/// @summary Loads the next source image in the SourceFiles list.
/// @param fp The output stream to which errors and warnings will be written.
/// @param params Image processing parameters.
/// @param image On return, stores information about the loaded image.
/// @return true if the image was loaded.
static bool load_next_source(FILE *fp, dds_params_t &params, image_info_t &image)
{
    if (params.SourceIndex == params.SourceCount)
    {   // no more source files to load.
        return false;
    }
    return load_source(fp, params, params.SourceIndex++, image);
}

/// @summary Generates and writes to disk the mipmap chain for an image. This 
/// function writes the base image first, followed by all sub-levels.
/// @param fp The output stream to which errors and warnings will be written.
/// @param dds The output stream to which the image data will be written.
/// @param params Image processing parameters.
/// @param base_level A description of the highest-resolution image. If resizing
/// is requested, or the image needs to be forced to power-of-two dimensions, 
/// on return base_level will describe the resized image, and the original image
/// described by base_level is freed.
/// @return true if the entire mipchain was written to stream dds.
static bool write_image_chain(FILE *fp, FILE *dds, dds_params_t const &params, image_info_t &base_level)
{
    if (base_level.Pixels == NULL)
    {   // unable to load one of the source images.
        return false;
    }

    if (params.Width != params.BaseWidth || params.Height != params.BaseHeight)
    {   // explicit resample requested, or we need to force power-of-two.
        image_info_t out;
        if (resize_image(fp, out, base_level, params.Width, params.Height) == false)
        {   // resize_image() outputs error messages.
            return false;
        }
        free_image(base_level);
        base_level = out;
    }

    // write the highest-resolution image.
    size_t pitch = data::dds_pitch(params.Format, params.Width);
    size_t nb    = pitch  * params.Height;
    fwrite(base_level.Pixels, nb, 1, dds);

    // write any additional levels in the mipmap chain.
    if (params.Mipmaps && params.MaxMipLevels > 1)
    {   // always generate the miplevel from the high-resolution source.
        for (size_t i = 1; i < params.MaxMipLevels; ++i)
        {
            size_t lw = params.Width  >> i;
            size_t lh = params.Height >> i;
            if (lw < 1) lw = 1;
            if (lh < 1) lh = 1;

            image_info_t mip;
            if (resize_image(fp, mip, base_level, lw, lh))
            {   // write the mip-level to the output stream and delete it.
                pitch = data::dds_pitch(params.Format, lw);
                nb    = pitch * lh;
                fwrite(mip.Pixels , nb, 1, dds);
                free_image(mip);
            }
            else return false;
        }
    }
    return true;
}

/// @summary Loads six source files specified in the image processing parameters
/// and writes them to the DDS output stream as a cubemap image. If requested, 
/// mipmaps are generated and written to the output stream as well.
/// @param fp The output stream to which errors and warnings will be written.
/// @param dds The output stream to which the image data will be written.
/// @param params Image processing parameters. These parameters may be updated
/// with defaults based on the first cubemap face loaded.
/// @return true if the entire cubemap image chain was written to the DDS output stream.
static bool write_cubemap_image(FILE *fp, FILE *dds, dds_params_t &params)
{
    for (size_t i = 0; i < 6; ++i)
    {   // note that params.SourceIndex is used to keep track of state in this
        // case, because it's possible to have arrays of cubemaps with mipmaps.
        image_info_t face;
        bool   first_face  = params.SourceIndex == 0;
        if (load_next_source(fp, params, face))
        {
            if (first_face)
            {   // set any 'default to source' parameters.
                if (params.Width  == 0) params.Width  = size_t(face.Width);
                if (params.Height == 0) params.Height = size_t(face.Height);
                params.BaseWidth   = size_t(face.Width);
                params.BaseHeight  = size_t(face.Height);

                if (params.Format == data::DXGI_FORMAT_UNKNOWN)
                {   // set the format to that of the first face.
                    params.Format  = face.Format;
                }
                if (params.AlphaMode == data::DDS_ALPHA_MODE_UNKNOWN)
                {
                    if (face.Channels == 4) params.AlphaMode = data::DDS_ALPHA_MODE_PREMULTIPLIED;
                    else params.AlphaMode = data::DDS_ALPHA_MODE_OPAQUE;
                }
                if (params.MaxMipLevels == 0)
                {   // compute all the way down to 1x1.
                    size_t lw = params.Width;
                    size_t lh = params.Height;
                    while (lw > 1 || lh > 1)
                    {
                        params.MaxMipLevels++;
                        lw >>= 1; if (lw == 0) lw = 1;
                        lh >>= 1; if (lh == 0) lh = 1;
                    }
                    // include the base level in the count.
                    params.MaxMipLevels++;
                }
            }

            if (!write_image_chain(fp, dds, params, face))
            {
                fprintf(fp, "ERROR: Unable to write face %u/6 (\'%s\').\n", unsigned(i), params.SourceFiles[params.SourceIndex-1]);
                return false;
            }
            else free_image(face);
        }
    }
    return true;
}

/// @summary Loads the source files specified in the image processing parameters
/// and writes them to the DDS output stream as an image array. If requested, 
/// mipmaps are generated and written to the output stream as well.
/// @param fp The output stream to which errors and warnings will be written.
/// @param dds The output stream to which the image data will be written.
/// @param params Image processing parameters. These parameters may be updated
/// with defaults based on the first image loaded.
/// @return true if the entire image array chain was written to the DDS output stream.
static bool write_array_image(FILE *fp, FILE *dds, dds_params_t &params)
{
    params.SourceIndex = 0;
    if (params.Cubemap)
    {
        for (size_t  i = 0, n = params.ArraySize; i < n; ++i)
        {
            if (!write_cubemap_image(fp, dds, params))
            {
                fprintf(fp, "ERROR: Unable to write element %u/%u.\n", unsigned(i), unsigned(n));
                return false;
            }
        }
    }
    else
    {
        for (size_t  i = 0, n = params.ArraySize; i < n; ++i)
        {
            image_info_t image;
            if (load_next_source(fp, params, image))
            {
                if (i == 0)
                {   // set any 'default to source' parameters. note that all 
                    // images in the array have the same dimension and format.
                    if (params.Width  == 0) params.Width  = size_t(image.Width);
                    if (params.Height == 0) params.Height = size_t(image.Height);
                    params.BaseWidth   = size_t(image.Width);
                    params.BaseHeight  = size_t(image.Height);

                    if (params.Format == data::DXGI_FORMAT_UNKNOWN)
                    {   // set the format to that of the first image.
                        params.Format  = image.Format;
                    }
                    if (params.AlphaMode == data::DDS_ALPHA_MODE_UNKNOWN)
                    {
                        if (image.Channels == 4) params.AlphaMode = data::DDS_ALPHA_MODE_PREMULTIPLIED;
                        else params.AlphaMode = data::DDS_ALPHA_MODE_OPAQUE;
                    }
                    if (params.MaxMipLevels == 0)
                    {   // compute all the way down to 1x1.
                        size_t lw = params.Width;
                        size_t lh = params.Height;
                        while (lw > 1 || lh > 1)
                        {
                            params.MaxMipLevels++;
                            lw >>= 1; if (lw == 0) lw = 1;
                            lh >>= 1; if (lh == 0) lh = 1;
                        }
                        // include the base level in the count.
                        params.MaxMipLevels++;
                    }
                }
                if (!write_image_chain(fp, dds, params, image))
                {
                    free_image(image);
                    fprintf(fp, "ERROR: Unable to write element %u/%u.\n", unsigned(i), unsigned(n));
                    return false;
                }
                else free_image(image);
            }
            else
            {
                fprintf(fp, "ERROR: Unable to load element %u/%u (\'%s\').\n", unsigned(i), unsigned(n), params.SourceFiles[i]);
                return false;
            }
        }
    }
    return true;
}

/// @summary Loads the series of source files specified in the image processing
/// parameters, and writes them to the DDS output stream as a volume image.
/// @param fp The output stream to which errors and warnings will be written.
/// @param dds The output stream to which the image data will be written.
/// @param params Image processing parameters. These parameters may be updated
/// with defaults based on the first slice loaded.
/// @return true if the entire volume image was written to the DDS output stream.
static bool write_volume_image(FILE *fp, FILE *dds, dds_params_t &params)
{
    params.SourceIndex = 0;
    for (size_t i = 0, n = params.SourceCount; i < n; ++i)
    {
        image_info_t slice;
        if (load_next_source(fp, params, slice))
        {
            if (i == 0)
            {   // set any 'default to source' parameters.
                // note that volume images don't currently support mipmaps.
                if (params.Width  == 0) params.Width  = size_t(slice.Width);
                if (params.Height == 0) params.Height = size_t(slice.Height);
                params.BaseWidth   = size_t(slice.Width);
                params.BaseHeight  = size_t(slice.Height);
                
                if (params.Format == data::DXGI_FORMAT_UNKNOWN)
                {   // set the format to that of the base slice.
                    params.Format  = slice.Format;
                }
                if (params.AlphaMode == data::DDS_ALPHA_MODE_UNKNOWN)
                {   // set the alpha mode based on the number of source channels.
                    if (slice.Channels == 4) params.AlphaMode = data::DDS_ALPHA_MODE_PREMULTIPLIED;
                    else params.AlphaMode = data::DDS_ALPHA_MODE_OPAQUE;
                }
                if (params.ForcePow2)
                {   // set Width and Height to the nearest power of 2.
                    params.Width   = pow2_ge(params.Width , 1);
                    params.Height  = pow2_ge(params.Height, 1);
                }
            }

            if (params.Width != params.BaseWidth || params.Height != params.BaseHeight)
            {   // explicit resample requested, or we need to force a power-of-two.
                image_info_t out;
                if (resize_image(fp, out, slice, params.Width, params.Height) == false)
                {   // resize_image() outputs error messages.
                    return false;
                }
                free_image(slice);
                slice = out;
            }

            // write the slice data out to the DDS file.
            size_t pitch = data::dds_pitch(params.Format, params.Width);
            size_t nb    = pitch  * params.Height;
            fwrite(slice.Pixels, nb, 1, dds);
            free_image(slice);
        }
        else
        {
            fprintf(fp, "ERROR: Unable to load slice %u/%u (\'%s\').\n", unsigned(i), unsigned(n), params.SourceFiles[i]);
            return false;
        }
    }
    return true;
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

    // figure out which argv is the output path, set params.OutputFile.
    int path_count  = 0;
    int last_path   = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strlen(argv[i]) > 5) // a.ext
        {
            if (argv[i][0] != '-' && strstr(argv[i], ".") != NULL)
            {
                last_path = i;
                path_count++;
            }
        }
    }
    if (path_count < 2)
    {
        fprintf(stdout, "ERROR: No output path specified.\n");
        print_usage(stdout);
        exit(EXIT_FAILURE);
    }

    // figure out the image processing parameters used to generate
    // the output DDS. this may involve loading and parsing JSON, 
    // and may also load the image file, if there's only one.
    dds_params_t params;
    image_info_t image0;
    if (params_from_path(stdout, argv[1], params, image0) == false)
    {   // params_from_path() outputs error messages.
        exit(EXIT_FAILURE);
    }
    modify_params(argc, argv, params);
    params.OutputFile = argv[last_path];

    // generate the DDS headers based on the image processing 
    // parameters describing the attributes of the DDS file.
    data::dds_header_t        dds;
    data::dds_header_dxt10_t dx10;

    // open up the output DDS. any existing file is overwritten.
    bool  res = true;
    FILE *fp  = fopen(params.OutputFile, "w+b");
    if (fp != NULL)
    {   // skip over the bytes reserved for header information.
        // the image data will be written first, because for 
        // complex image types (cubemaps, volumes, arrays) the 
        // information necessary to generate the header is not 
        // known until after the image data has been written.
        uint32_t magic = data::fourcc_le('D','D','S',' ');
        size_t  offset = sizeof(uint32_t) + sizeof(data::dds_header_t) + sizeof(data::dds_header_dxt10_t);
        if (fseek(fp, (long) offset, SEEK_SET) != 0)
        {
            fclose(fp);
            fprintf(stdout, "ERROR: Cannot seek past end-of-file.\n");
            free_image(image0);
            exit(EXIT_FAILURE);
        }

        // in the common case, we have only one image to load and output, 
        // possibly with mipmaps to generate. if this is the case, then 
        // we will have loaded the base image during parameter parsing.
        if (image0.Pixels != NULL)
        {   // generate and write the entire mipmap chain. the base level
            // is written first, followed by the downsampled miplevels.
            res = write_image_chain(stdout, fp, params, image0);
            free_image(image0);
        }
        else
        {   // we are generating either a cubemap (which can have mipmaps), 
            // a volume image (which currently cannot have mipmaps) or an 
            // image array (which can have mipmaps, but only be 1D/2D/Cubemap).
            // in all of these cases, we have not loaded any image, and so we
            // have to handle defaulting of any parameter values specified as
            // 'default to source image'.
            if (params.Volume) res = write_volume_image(stdout, fp, params);
            else res = write_array_image(stdout, fp, params);
        }

        // seek back to the start of the file and write the header data.
        fseek(fp, 0, SEEK_SET);
        init_dds_header(&dds, params);
        init_dds_header_dxt10(&dx10 , params);
        fwrite(&magic, sizeof(uint32_t), 1, fp);
        fwrite(&dds  , sizeof(data::dds_header_t), 1, fp);
        fwrite(&dx10 , sizeof(data::dds_header_dxt10_t), 1, fp);

        // the entire file has been written, so we're done.
        fclose(fp);
    }
    else
    {
        if (params.JsonBuffer != NULL) free(params.JsonBuffer);
        fprintf(stdout, "ERROR: Cannot open output file \'%s\'.\n", params.OutputFile);
        exit(EXIT_FAILURE);
    }
    
    if (params.JsonBuffer != NULL) free(params.JsonBuffer);
    exit(EXIT_SUCCESS);
}

