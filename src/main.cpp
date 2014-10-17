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
#define stricmp_fn  _stricmp
#endif

#ifdef __GNUC__
#include <strings.h>
#define stricmp_fn  strcasecmp
#endif

/*/////////////////
//   Constants   //
/////////////////*/
/// @summary Define the maximum number of source images. Normally this wouldn't
/// be so large, but volume images can have many slices.
static size_t const MAX_SOURCE_IMAGES = 4096;

/*//////////////////
//   Data Types   //
//////////////////*/
/// @summary Define the set of input parameters to the application.
struct dds_params_t
{
    size_t   Width;        /// Desired width, in pixels. Default = width of source.
    size_t   Height;       /// Desired height, in pixels. Default = height of source.
    size_t   MaxMipLevels; /// The maximum number of mipmap levels to generate. Default = 1.
    size_t   ArraySize;    /// Number of items in an array. Default depends on other options.
    uint32_t Format;       /// One of data::dxgi_format_e. Default = format of source.
    bool     Mipmaps;      /// true if the output has mipmaps.
    bool     Cubemap;      /// true if the output is a cubemap or cubemap array. Default = false.
    bool     Volume;       /// true if the output is a volume image. Default = false.
    char    *OutputFile;   /// The path or filename of the output file to generate.
    char    *JsonBuffer;   /// The buffer containing the input JSON data, or NULL.
    size_t   SourceCount;  /// The number of items in SourceFiles.
    char    *SourceFiles[MAX_SOURCE_IMAGES]; /// The filenames of all input files.
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
    params.MaxMipLevels  = 1;
    params.ArraySize     = 1;
    params.Format        = data::DXGI_FORMAT_B8G8R8A8_UNORM;
    params.Mipmaps       = false;
    params.Cubemap       = false;
    params.Volume        = false;
    params.OutputFile    = NULL;
    params.JsonBuffer    = buffer;
    params.SourceCount   = 0;
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
                {
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
                    fprintf(fp, "ERROR: Unexpected array element \'%s\'.", node->Key);
                    return false;
                }
                params.SourceCount         = 0;
                data::json_item_t *element = node->FirstChild;
                while (element != NULL)
                {   // all child elements must be strings.
                    if (element->ValueType != data::JSON_TYPE_STRING)
                    {
                        fprintf(fp, "ERROR: Expect only strings in SourceFiles array.\n");
                        return false;
                    }
                    params.SourceFiles[params.SourceCount++] = element.Value.string;
                    element = element->Next;
                }
            }
            break;

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
                    // @TODO: convert Format value into data::dxgi_format_e.
                }
                if (0 == stricmp_fn(node->Key, "AlphaMode"))
                {
                    // @TODO: convert AlphaMode value into data::dds_alpha_mode_e.
                }
            }
            break;

        case data::JSON_TYPE_INTEGER:
            {   // Width, Height, MaxMipLevels, and ArraySize may be integers.
                if (0 != stricmp_fn(node->Key, "Width"       ) && 
                    0 != stricmp_fn(node->Key, "Height"      ) && 
                    0 != stricmp_fn(node->Key, "MaxMipLevels") && 
                    0 != stricmp_fn(node->Key, "ArraySize"   ))
                {
                    fprintf(fp, "ERROR: Unexpected Integer field \'%s\'.\n", node->Key);
                    return false;
                }

                if (0 == stricmp_fn(node->Key, "Width"))
                {
                    params.Width = size_t(node->Value.integer);
                }
                if (0 == stricmp_fn(node->Key, "Height"))
                {
                    params.Height = size_t(node->Value.integer);
                }
                if (0 == stricmp_fn(node->Key, "MaxMipLevels"))
                {
                    params.MaxMipLevels = size_t(node->Value.integer);
                }
                if (0 == stricmp_fn(node->Key, "ArraySize"))
                {
                    params.ArraySize = size_t(node->Value.integer);
                }
            }
            return true;

        case data::JSON_TYPE_NUMBER:
            {   // No Number values are expected currently.
                fprintf(fp, "ERROR: Unexpected Number field \'%s\'.\n", node->Key);
            }
            return false;

        case data::JSON_TYPE_BOOLEAN:
            {
                // Cubemap, Volume and Mipmaps may be booleans.
                if (0 != stricmp_fn(node->Key, "Cubemap") && 
                    0 != stricmp_fn(node->Key, "Mipmaps") && 
                    0 != stricmp_fn(node->Key, "Volume" ))
                {
                    fprintf(fp, "ERROR: Unexpected Boolean field \'%s\'.\n", node->Key);
                    return false;
                }

                if (0 == stricmp_fn(node->Key, "Cubemap"))
                {
                    params.Cubemap = node->Value.boolean;
                }
                if (0 == stricmp_fn(node->Key, "Mipmaps"))
                {
                    params.Mipmaps = node->Value.boolean;
                }
                if (0 == stricmp_fn(node->Key, "Volume"))
                {
                    params.Volume = node->Value.boolean;
                }
            }
            return true;

        case data::JSON_TYPE_NULL:
            {   // any supported value may be null, and assumes the default.
            }
            return true;
    }
}

/// @summary Parses a JSON string to extract DDS output parameters.
/// @param fp The stream to which parsing errors will be written.
/// @param json Pointer to a NULL-terminated buffer containing the input JSON.
/// @param json_size The number of bytes of JSON to parse, or 0 to parse the entire string.
/// @param free_buffer Specify true to save a reference to the input JSON buffer
/// so it can be freed when program execution has completed.
/// @param params On return, 
static bool params_from_json(FILE *fp, char *json, size_t json_size, bool free_buffer, dds_params_t &params)
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

    data::json_item_t  *node = NULL;
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
    node = root;
    while (node != NULL)
    {
        switch (node->ValueType)
        {
        }
    }

    // free all of the JSON nodes; they are no longer needed.
    data::json_free(root, NULL);
}

static bool params_from_path(FILE *fp, char const *inpath, dds_params_t &params)
{
    size_t      ext_len = 0;
    char const *ext_str = extpart(inpath, ext_len);

    if (ext_len == 0)
    {
        fprintf(fp, "ERROR: No file extension on \'%s\'.\nUnable to determine file type.\n", inpath);
        return false;
    }

    if (0 == stricmp_fn(ext_str, "json"))
    {
        size_t  nb = 0;
        char *json = data::load_text(inpath, &nb, NULL);
        return params_from_json(fp, json, nb, true, params);
    }
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
    ddspf->Flags       = data::DDPF_RGB | data::DDPF_ALPHA | data::DDPF_FOURCC;
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
    head->Height    = uint32_t(h);
    head->Width     = uint32_t(w);
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

