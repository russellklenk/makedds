/*/////////////////////////////////////////////////////////////////////////////
/// @summary Define some functions and types for parsing a limited set of data
/// formats so you can quickly get some data into your application. Currently
/// supported formats are DDS (for image data), WAV (for sound data) and JSON.
/// The data should be loaded into memory and passed to the parsing routines.
/// The data is typically parsed in-place, and may be modified.
/// @author Russell Klenk (contact@russellklenk.com)
///////////////////////////////////////////////////////////////////////////80*/

#ifndef LLDATAIN_HPP_INCLUDED
#define LLDATAIN_HPP_INCLUDED

/*////////////////
//   Includes   //
////////////////*/
#include <stddef.h>
#include <stdint.h>

/*////////////////////
//   Preprocessor   //
////////////////////*/
/// @summary Abstract away Windows 32-bit calling conventions and visibility.
#if defined(_WIN32) && defined(_MSC_VER)
    #define  LLDATAIN_CALL_C    __cdecl
    #if   defined(_MSC_VER)
    #define  LLDATAIN_IMPORT    __declspec(dllimport)
    #define  LLDATAIN_EXPORT    __declspec(dllexport)
    #elif defined(__GNUC__)
    #define  LLDATAIN_IMPORT    __attribute__((dllimport))
    #define  LLDATAIN_EXPORT    __attribute__((dllexport))
    #else
    #define  LLDATAIN_IMPORT
    #define  LLDATAIN_EXPORT
    #endif
#else
    #define  LLDATAIN_CALL_C
    #if __GNUC__ >= 4
    #define  LLDATAIN_IMPORT    __attribute__((visibility("default")))
    #define  LLDATAIN_EXPORT    __attribute__((visibility("default")))
    #endif
#endif

/// @summary Define import/export based on whether we're being used as a DLL.
#if defined(LLDATAIN_SHARED)
    #ifdef  LLDATAIN_EXPORTS
    #define LLDATAIN_PUBLIC     LLDATAIN_EXPORT
    #else
    #define LLDATAIN_PUBLIC     LLDATAIN_IMPORT
    #endif
#else
    #define LLDATAIN_PUBLIC
#endif

/*///////////////////////
//   Namespace Begin   //
///////////////////////*/
namespace data {

/*/////////////////
//   Constants   //
/////////////////*/
/// @summary The FourCC 'DDS ' using little-endian byte ordering.
#ifndef LLDATAIN_DDS_MAGIC_LE
#define LLDATAIN_DDS_MAGIC_LE      0x20534444U
#endif

/*/////////////////
//   Data Types  //
/////////////////*/
/// @summary Bitflags used for bmfont_char_t::Channel to indicate which color
/// channels of the glyph image contain the glyph data.
enum bmfont_channel_e
{
    BMFONT_CHANNEL_NONE         = (0 << 0),
    BMFONT_CHANNEL_BLUE         = (1 << 0),
    BMFONT_CHANNEL_GREEN        = (1 << 1),
    BMFONT_CHANNEL_RED          = (1 << 2),
    BMFONT_CHANNEL_ALPHA        = (1 << 3),
    BMFONT_CHANNEL_ALL          = BMFONT_CHANNEL_BLUE  |
                                  BMFONT_CHANNEL_GREEN |
                                  BMFONT_CHANNEL_RED   |
                                  BMFONT_CHANNEL_ALPHA
};

/// @summary Bitflags used for bmfont_info_block_t::Attributes.
enum bmfont_attributes_e
{
    BMFONT_ATTRIBUTE_NONE       = (0 << 0),
    BMFONT_ATTRIBUTE_SMOOTH     = (1 << 0),
    BMFONT_ATTRIBUTE_UNICODE    = (1 << 1),
    BMFONT_ATTRIBUTE_ITALIC     = (1 << 2),
    BMFONT_ATTRIBUTE_BOLD       = (1 << 3),
    BMFONT_ATTRIBUTE_FIXED      = (1 << 4)
};

/// @summary Values used for bmfont_common_block_t::[Alpha/Red/Green/Blue]Channel.
enum bmfont_content_e
{
    BMFONT_CONTENT_GLYPH        = 0,
    BMFONT_CONTENT_OUTLINE      = 1,
    BMFONT_CONTENT_COMBINED     = 2,
    BMFONT_CONTENT_ZERO         = 3,
    BMFONT_CONTENT_ONE          = 4
};

/// @summary Bitflags for dds_pixelformat_t::Flags. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx
/// for the DDS_PIXELFORMAT structure.
enum dds_pixelformat_flags_e
{
    DDPF_NONE                   = 0x00000000U,
    DDPF_ALPHAPIXELS            = 0x00000001U,
    DDPF_ALPHA                  = 0x00000002U,
    DDPF_FOURCC                 = 0x00000004U,
    DDPF_RGB                    = 0x00000040U,
    DDPF_YUV                    = 0x00000200U,
    DDPF_LUMINANCE              = 0x00020000U
};

/// @summary Bitflags for dds_header_t::Flags. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
/// for the DDS_HEADER structure.
enum dds_header_flags_e
{
    DDSD_NONE                   = 0x00000000U,
    DDSD_CAPS                   = 0x00000001U,
    DDSD_HEIGHT                 = 0x00000002U,
    DDSD_WIDTH                  = 0x00000004U,
    DDSD_PITCH                  = 0x00000008U,
    DDSD_PIXELFORMAT            = 0x00001000U,
    DDSD_MIPMAPCOUNT            = 0x00020000U,
    DDSD_LINEARSIZE             = 0x00080000U,
    DDSD_DEPTH                  = 0x00800000U,
    DDS_HEADER_FLAGS_TEXTURE    = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT,
    DDS_HEADER_FLAGS_MIPMAP     = DDSD_MIPMAPCOUNT,
    DDS_HEADER_FLAGS_VOLUME     = DDSD_DEPTH,
    DDS_HEADER_FLAGS_PITCH      = DDSD_PITCH,
    DDS_HEADER_FLAGS_LINEARSIZE = DDSD_LINEARSIZE
};

/// @summary Bitflags for dds_header_t::Caps. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
/// for the DDS_HEADER structure.
enum dds_caps_e
{
    DDSCAPS_NONE                = 0x00000000U,
    DDSCAPS_COMPLEX             = 0x00000008U,
    DDSCAPS_TEXTURE             = 0x00001000U,
    DDSCAPS_MIPMAP              = 0x00400000U,
    DDS_SURFACE_FLAGS_MIPMAP    = DDSCAPS_COMPLEX | DDSCAPS_MIPMAP,
    DDS_SURFACE_FLAGS_TEXTURE   = DDSCAPS_TEXTURE,
    DDS_SURFACE_FLAGS_CUBEMAP   = DDSCAPS_COMPLEX
};

/// @summary Bitflags for dds_header_t::Caps2. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
/// for the DDS_HEADER structure.
enum dds_caps2_e
{
    DDSCAPS2_NONE               = 0x00000000U,
    DDSCAPS2_CUBEMAP            = 0x00000200U,
    DDSCAPS2_CUBEMAP_POSITIVEX  = 0x00000400U,
    DDSCAPS2_CUBEMAP_NEGATIVEX  = 0x00000800U,
    DDSCAPS2_CUBEMAP_POSITIVEY  = 0x00001000U,
    DDSCAPS2_CUBEMAP_NEGATIVEY  = 0x00002000U,
    DDSCAPS2_CUBEMAP_POSITIVEZ  = 0x00004000U,
    DDSCAPS2_CUBEMAP_NEGATIVEZ  = 0x00008000U,
    DDSCAPS2_VOLUME             = 0x00200000U,
    DDS_FLAG_VOLUME             = DDSCAPS2_VOLUME,
    DDS_CUBEMAP_POSITIVEX       = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX,
    DDS_CUBEMAP_NEGATIVEX       = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX,
    DDS_CUBEMAP_POSITIVEY       = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY,
    DDS_CUBEMAP_NEGATIVEY       = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY,
    DDS_CUBEMAP_POSITIVEZ       = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ,
    DDS_CUBEMAP_NEGATIVEZ       = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ,
    DDS_CUBEMAP_ALLFACES        = DDSCAPS2_CUBEMAP |
                                  DDSCAPS2_CUBEMAP_POSITIVEX |
                                  DDSCAPS2_CUBEMAP_NEGATIVEX |
                                  DDSCAPS2_CUBEMAP_POSITIVEY |
                                  DDSCAPS2_CUBEMAP_NEGATIVEY |
                                  DDSCAPS2_CUBEMAP_POSITIVEZ |
                                  DDSCAPS2_CUBEMAP_NEGATIVEZ
};

/// @summary Bitflags for dds_header_t::Caps3. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
/// for the DDS_HEADER structure.
enum dds_caps3_e
{
    DDSCAPS3_NONE = 0x00000000U
};

/// @summary Bitflags for dds_header_t::Caps4. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
/// for the DDS_HEADER structure.
enum dds_caps4_e
{
    DDSCAPS4_NONE = 0x00000000U
};

/// @summary Values for dds_header_dxt10_t::Format. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
enum dxgi_format_e
{
    DXGI_FORMAT_UNKNOWN                     = 0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
    DXGI_FORMAT_R32G32B32A32_UINT           = 3,
    DXGI_FORMAT_R32G32B32A32_SINT           = 4,
    DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
    DXGI_FORMAT_R32G32B32_FLOAT             = 6,
    DXGI_FORMAT_R32G32B32_UINT              = 7,
    DXGI_FORMAT_R32G32B32_SINT              = 8,
    DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
    DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
    DXGI_FORMAT_R16G16B16A16_UINT           = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
    DXGI_FORMAT_R16G16B16A16_SINT           = 14,
    DXGI_FORMAT_R32G32_TYPELESS             = 15,
    DXGI_FORMAT_R32G32_FLOAT                = 16,
    DXGI_FORMAT_R32G32_UINT                 = 17,
    DXGI_FORMAT_R32G32_SINT                 = 18,
    DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
    DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
    DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
    DXGI_FORMAT_R10G10B10A2_UINT            = 25,
    DXGI_FORMAT_R11G11B10_FLOAT             = 26,
    DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    DXGI_FORMAT_R8G8B8A8_UINT               = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
    DXGI_FORMAT_R8G8B8A8_SINT               = 32,
    DXGI_FORMAT_R16G16_TYPELESS             = 33,
    DXGI_FORMAT_R16G16_FLOAT                = 34,
    DXGI_FORMAT_R16G16_UNORM                = 35,
    DXGI_FORMAT_R16G16_UINT                 = 36,
    DXGI_FORMAT_R16G16_SNORM                = 37,
    DXGI_FORMAT_R16G16_SINT                 = 38,
    DXGI_FORMAT_R32_TYPELESS                = 39,
    DXGI_FORMAT_D32_FLOAT                   = 40,
    DXGI_FORMAT_R32_FLOAT                   = 41,
    DXGI_FORMAT_R32_UINT                    = 42,
    DXGI_FORMAT_R32_SINT                    = 43,
    DXGI_FORMAT_R24G8_TYPELESS              = 44,
    DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
    DXGI_FORMAT_R8G8_TYPELESS               = 48,
    DXGI_FORMAT_R8G8_UNORM                  = 49,
    DXGI_FORMAT_R8G8_UINT                   = 50,
    DXGI_FORMAT_R8G8_SNORM                  = 51,
    DXGI_FORMAT_R8G8_SINT                   = 52,
    DXGI_FORMAT_R16_TYPELESS                = 53,
    DXGI_FORMAT_R16_FLOAT                   = 54,
    DXGI_FORMAT_D16_UNORM                   = 55,
    DXGI_FORMAT_R16_UNORM                   = 56,
    DXGI_FORMAT_R16_UINT                    = 57,
    DXGI_FORMAT_R16_SNORM                   = 58,
    DXGI_FORMAT_R16_SINT                    = 59,
    DXGI_FORMAT_R8_TYPELESS                 = 60,
    DXGI_FORMAT_R8_UNORM                    = 61,
    DXGI_FORMAT_R8_UINT                     = 62,
    DXGI_FORMAT_R8_SNORM                    = 63,
    DXGI_FORMAT_R8_SINT                     = 64,
    DXGI_FORMAT_A8_UNORM                    = 65,
    DXGI_FORMAT_R1_UNORM                    = 66,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
    DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
    DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
    DXGI_FORMAT_BC1_TYPELESS                = 70,
    DXGI_FORMAT_BC1_UNORM                   = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
    DXGI_FORMAT_BC2_TYPELESS                = 73,
    DXGI_FORMAT_BC2_UNORM                   = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
    DXGI_FORMAT_BC3_TYPELESS                = 76,
    DXGI_FORMAT_BC3_UNORM                   = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
    DXGI_FORMAT_BC4_TYPELESS                = 79,
    DXGI_FORMAT_BC4_UNORM                   = 80,
    DXGI_FORMAT_BC4_SNORM                   = 81,
    DXGI_FORMAT_BC5_TYPELESS                = 82,
    DXGI_FORMAT_BC5_UNORM                   = 83,
    DXGI_FORMAT_BC5_SNORM                   = 84,
    DXGI_FORMAT_B5G6R5_UNORM                = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
    DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
    DXGI_FORMAT_BC6H_TYPELESS               = 94,
    DXGI_FORMAT_BC6H_UF16                   = 95,
    DXGI_FORMAT_BC6H_SF16                   = 96,
    DXGI_FORMAT_BC7_TYPELESS                = 97,
    DXGI_FORMAT_BC7_UNORM                   = 98,
    DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
    DXGI_FORMAT_AYUV                        = 100,
    DXGI_FORMAT_Y410                        = 101,
    DXGI_FORMAT_Y416                        = 102,
    DXGI_FORMAT_NV12                        = 103,
    DXGI_FORMAT_P010                        = 104,
    DXGI_FORMAT_P016                        = 105,
    DXGI_FORMAT_420_OPAQUE                  = 106,
    DXGI_FORMAT_YUY2                        = 107,
    DXGI_FORMAT_Y210                        = 108,
    DXGI_FORMAT_Y216                        = 109,
    DXGI_FORMAT_NV11                        = 110,
    DXGI_FORMAT_AI44                        = 111,
    DXGI_FORMAT_IA44                        = 112,
    DXGI_FORMAT_P8                          = 113,
    DXGI_FORMAT_A8P8                        = 114,
    DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
    DXGI_FORMAT_FORCE_UINT                  = 0xFFFFFFFFU
};

/// @summary Values for dds_header_dxt10_t::Dimension. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
/// for the DDS_HEADER_DXT10 structure.
enum d3d11_resource_dimension_e
{
    D3D11_RESOURCE_DIMENSION_UNKNOWN        = 0,
    D3D11_RESOURCE_DIMENSION_BUFFER         = 1,
    D3D11_RESOURCE_DIMENSION_TEXTURE1D      = 2,
    D3D11_RESOURCE_DIMENSION_TEXTURE2D      = 3,
    D3D11_RESOURCE_DIMENSION_TEXTURE3D      = 4
};

/// @summary Values for dds_header_dxt10_t::Flags. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
/// for the DDS_HEADER_DXT10 structure.
enum d3d11_resource_misc_flag_e
{
    D3D11_RESOURCE_MISC_TEXTURECUBE         = 0x00000004U,
};

/// @summary Values for dds_header_dxt10_t::Flags2. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
/// for the DDS_HEADER_DXT10 structure.
enum dds_alpha_mode_e
{
    DDS_ALPHA_MODE_UNKNOWN                  = 0x00000000U,
    DDS_ALPHA_MODE_STRAIGHT                 = 0x00000001U,
    DDS_ALPHA_MODE_PREMULTIPLIED            = 0x00000002U,
    DDS_ALPHA_MODE_OPAQUE                   = 0x00000003U,
    DDS_ALPHA_MODE_CUSTOM                   = 0x00000004U
};

/// @summary An enumeration defining the types of JSON nodes that can be stored
/// within a JSON document. The type is stored as a 4-byte field.
enum json_item_type_e
{
    JSON_TYPE_UNKNOWN                       = 0,
    JSON_TYPE_OBJECT                        = 1,
    JSON_TYPE_ARRAY                         = 2,
    JSON_TYPE_STRING                        = 3,
    JSON_TYPE_INTEGER                       = 4,
    JSON_TYPE_NUMBER                        = 5,
    JSON_TYPE_BOOLEAN                       = 6,
    JSON_TYPE_NULL                          = 7,
    JSON_TYPE_FORCE_32BIT                   = 0x7FFFFFFFL
};

/// @summary Defines the different text encodings that can be detected by
/// inspecting the first four bytes of a text document for a byte order marker.
enum text_encoding_e
{
    TEXT_ENCODING_UNSURE                    = 0,
    TEXT_ENCODING_ASCII                     = 1,
    TEXT_ENCODING_UTF8                      = 2,
    TEXT_ENCODING_UTF16_BE                  = 3,
    TEXT_ENCODING_UTF16_LE                  = 4,
    TEXT_ENCODING_UTF32_BE                  = 5,
    TEXT_ENCODING_UTF32_LE                  = 6,
    TEXT_ENCODING_FORCE_32BIT               = 0x7FFFFFFFL
};

/// @summary Defines the color map types supported by the image format. This
/// library only supports loading of TGA_COLORMAPTYPE_NONE images.
enum tga_colormaptype_e
{
    TGA_COLORMAPTYPE_NONE                   = 0,
    TGA_COLORMAPTYPE_INCLUDED               = 1
};

/// @summary Defines the image types supported by the image format. 
enum tga_imagetype_e
{
    TGA_IMAGETYPE_NO_IMAGE_DATA             = 0,
    TGA_IMAGETYPE_UNCOMPRESSED_PAL          = 1,
    TGA_IMAGETYPE_UNCOMPRESSED_TRUE         = 2,
    TGA_IMAGETYPE_UNCOMPRESSED_GRAY         = 3,
    TGA_IMAGETYPE_RLE_PAL                   = 9,
    TGA_IMAGETYPE_RLE_TRUE                  = 10,
    TGA_IMAGETYPE_RLE_GRAY                  = 11
};

/// @summary Defines the recognized compression types.
enum wav_compression_type_e
{
    WAVE_COMPRESSION_UNKNOWN                = 0x0000,
    WAVE_COMPRESSION_PCM                    = 0x0001,
    WAVE_COMPRESSION_ADPCM                  = 0x0002,
    WAVE_COMPRESSION_MPEG                   = 0x0050,
    WAVE_COMPRESISON_EXPERIMENTAL           = 0xFFFF
};

/// @summary Defines the fields present on the main file header.
#pragma pack(push, 1)
struct bmfont_header_t
{
    char Magic[3];              /// The characters 'BMF'.
    char Version;               /// The file format version. Currently, 3.
};
#pragma pack(pop)

/// @summary Stores metadata relating to a single block within a BMFont binary file.
#pragma pack(push, 1)
struct bmfont_block_header_t
{
    char     Id;                /// The block type identifier.
    uint32_t DataSize;          /// The size of the block data, in bytes.
};
#pragma pack(pop)

/// @summary The data associated with an INFO block in a BMFont file. Most of
/// this information can be safely ignored and is used at design-time.
#pragma pack(push, 1)
struct bmfont_info_block_t
{
    int16_t  FontSize;          /// The font size, in points.
    uint8_t  Attributes;        /// A combination of bmfont_attributes_e.
    uint8_t  CharSet;           /// The name of the OEM charset (when not Unicode).
    uint16_t StretchH;          /// The font height stretch percentage; 100 = none.
    uint8_t  AA;                /// The supersampling level used, 1 = none.
    uint8_t  PaddingTop;        /// The padding for each character, top side.
    uint8_t  PaddingRight;      /// The padding for each character, right side.
    uint8_t  PaddingBottom;     /// The padding for each character, bottom side.
    uint8_t  PaddingLeft;       /// The padding for each character, left side.
    uint8_t  SpacingX;          /// The horizontal spacing for each character.
    uint8_t  SpacingY;          /// The vertical spacing for each character.
    uint8_t  Outline;           /// The outline thickness for the glyphs.
    char     FontName[1];       /// NULL-terminated ASCII font name string, variable.
};
#pragma pack(pop)

/// @summary The data associated with a COMMON block in a BMFont file.
#pragma pack(push, 1)
struct bmfont_common_block_t
{
    uint16_t LineHeight;        /// The distance between each line of text, in pixels.
    uint16_t BaseLine;          /// # of pixels from absolute top of line to glyph base.
    uint16_t ScaleWidth;        /// Width of a texture page, in pixels.
    uint16_t ScaleHeight;       /// Height of a texture page, in pixels.
    uint16_t PageCount;         /// The number of texture pages in the font.
    uint8_t  Attributes;        /// A combination of bmfont_attributes_e.
    uint8_t  AlphaChannel;      /// One of bmfont_content_e.
    uint8_t  RedChannel;        /// One of bmfont_content_e.
    uint8_t  GreenChannel;      /// One of bmfont_content_e.
    uint8_t  BlueChannel;       /// One of bmfont_content_e.
};
#pragma pack(pop)

/// @summary The data associated with a PAGES block in a BMFont file. This
/// block is essentially just a blob of character data. There is one NULL-
/// terminated string for each page, and bmfont_common_block_t::PageCount pages.
/// Each string is the same length.
#pragma pack(push, 1)
struct bmfont_pages_block_t
{
    char     PageNames[1];      /// Array of same-length NULL-terminated strings.
};
#pragma pack(pop)

/// @summary Describes a single glyph within a texture page.
#pragma pack(push, 1)
struct bmfont_char_t
{
    uint32_t Codepoint;         /// The Unicode codepoint associated with the glyph.
    uint16_t TextureX;          /// X-coordinate of the upper-left corner of the glyph.
    uint16_t TextureY;          /// Y-coordinate of the upper-left corner of the glyph.
    uint16_t Width;             /// Width of the glyph on the texture, in pixels.
    uint16_t Height;            /// Height of the glyph on the texture, in pixels.
    uint16_t OffsetX;           /// Horizontal offset when copying the glyph to the screen.
    uint16_t OffsetY;           /// Vertical offset when copying the glyph to the screen.
    uint16_t AdvanceX;          /// How much to advance the current position.
    uint8_t  PageIndex;         /// The index of the page containing the glyph data.
    uint8_t  Channel;           /// A combination of bmfont_channel_e indicating where glyph data is found.
};
#pragma pack(pop)

/// @summary The data associated with a CHARS block in a BMFont file. This
/// block is just a blob of bmfont_char_t instances, tightly packed. The number
/// of characters is bmfont_block_header_t::DataSize / sizeof(bmfont_char_t).
#pragma pack(push, 1)
struct bmfont_chars_block_t
{
    bmfont_char_t Char[1];      /// Array of bmfont_char_t.
};
#pragma pack(pop)

/// @summary Describes a kerning pair, which controls the spacing between a
/// specific pair of glyphs. Not every glyph pair will have custom kerning.
#pragma pack(push, 1)
struct bmfont_kerning_t
{
    uint32_t A;                 /// The codepoint of the first glyph.
    uint32_t B;                 /// The codepoint of the second glyph.
    int16_t  AdvanceX;          /// The amount to advance the current position when drawing the glyph pair.
};
#pragma pack(pop)

/// @summary The data associated with a KERNING block in a BMFont file. This
/// block is just a blob of bmfont_kerning_t instances, tightly packed. The
/// number of pairs is bmfont_block_header_t::DataSize / sizeof(bmfont_kerning_t).
#pragma pack(push, 1)
struct bmfont_kerning_block_t
{
    bmfont_kerning_t Pair[1];   /// Array of bmfont_kerning_t.
};
#pragma pack(pop)

/// @summary Stores a description of a BMfont. The block pointers point into
/// the source data directly. Blocks not present in the source data are NULL.
struct bmfont_desc_t
{
    typedef bmfont_info_block_t     bmfont_ib_t;
    typedef bmfont_common_block_t   bmfont_cb_t;
    typedef bmfont_kerning_block_t  bmfont_kb_t;
    typedef bmfont_pages_block_t    bmfont_pb_t;
    typedef bmfont_chars_block_t    bmfont_gb_t;

    size_t          Version;    /// The BMfont file format version.
    size_t          NumPages;   /// The number of pages defined on the font.
    size_t          PageLength; /// Length of a single page filename, including zero byte.
    size_t          NumGlyphs;  /// The number of glyphs defined on the font.
    size_t          NumKerning; /// The number of entries in the kerning table.
    bmfont_ib_t    *Info;       /// Pointer to the info block data, or NULL.
    bmfont_cb_t    *Common;     /// Pointer to the common block data, or NULL.
    bmfont_pb_t    *Pages;      /// Pointer to the page filenames block data, or NULL.
    bmfont_gb_t    *Chars;      /// Pointer to the chars block data, or NULL.
    bmfont_kb_t    *Kerning;    /// Pointer to the kerning block data, or NULL.
};

/// @summary The equivalent of the DDS_PIXELFORMAT structure. See MSDN at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943984(v=vs.85).aspx
#pragma pack(push, 1)
struct dds_pixelformat_t
{
    uint32_t Size;            /// The size of the structure (32 bytes).
    uint32_t Flags;           /// Combination of dds_pixelformat_flags_e.
    uint32_t FourCC;          /// DXT1, DXT2, DXT3, DXT4, DXT5 or DX10. See MSDN.
    uint32_t RGBBitCount;     /// The number of bits per-pixel.
    uint32_t BitMaskR;        /// Mask for reading red/luminance/Y data.
    uint32_t BitMaskG;        /// Mask for reading green/U data.
    uint32_t BitMaskB;        /// Mask for reading blue/V data.
    uint32_t BitMaskA;        /// Mask for reading alpha channel data.
};
#pragma pack(pop)

/// @summary The equivalent of the DDS_HEADER structure. See MSDN at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx
#pragma pack(push, 1)
struct dds_header_t
{
    uint32_t Size;            /// The size of the structure (124 bytes).
    uint32_t Flags;           /// Combination of dds_header_flags_e.
    uint32_t Height;          /// The surface height, in pixels.
    uint32_t Width;           /// The surface width, in pixels.
    uint32_t Pitch;           /// Bytes per-scanline, or bytes in top-level (compressed).
    uint32_t Depth;           /// The surface depth, in slices. For non-volume surfaces, 0.
    uint32_t Levels;          /// The number of mipmap levels, or 0 if there are no mipmaps.
    uint32_t Reserved1[11];   /// Reserved for future use.
    dds_pixelformat_t Format; /// Pixel format descriptor.
    uint32_t Caps;            /// Combination of dds_caps_e.
    uint32_t Caps2;           /// Combination of dds_caps2_e.
    uint32_t Caps3;           /// Combination of dds_caps3_e.
    uint32_t Caps4;           /// Combination of dds_caps4_e.
    uint32_t Reserved2;       /// Reserved for future use.
};
#pragma pack(pop)

/// @summary The equivalent of the DDS_HEADER_DXT10 structure. See MSDN at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943983(v=vs.85).aspx
#pragma pack(push, 1)
struct dds_header_dxt10_t
{
    uint32_t Format;          /// One of dxgi_format_e.
    uint32_t Dimension;       /// One of d3d11_resource_dimension_e.
    uint32_t Flags;           /// Combination of d3d11_resource_misc_flag_e.
    uint32_t ArraySize;       /// The number of of items in an array texture.
    uint32_t Flags2;          /// One of dds_alpha_mode_e.
};
#pragma pack(pop)

/// @summary Describes a single level within the mipmap pyramid in a DDS. Level
/// zero represents the highest-resolution surface (the base surface.)
struct dds_level_desc_t
{
    size_t   Index;           /// The zero-based index of the mip-level.
    size_t   Width;           /// The width of the surface.
    size_t   Height;          /// The height of the surface.
    size_t   Slices;          /// The depth of the surface.
    size_t   BytesPerElement; /// The number of bytes per-pixel or block.
    size_t   BytesPerRow;     /// The number of bytes between scanlines.
    size_t   BytesPerSlice;   /// The number of bytes between slices.
    size_t   DataSize;        /// The total size of the data for the level, in bytes.
    void    *LevelData;       /// Pointer to the start of the level data.
    uint32_t Format;          /// One of dxgi_format_e.
};

/// @summary Describes an error that was encountered while parsing a JSON document.
struct json_error_t
{
    char const  *Description; /// A brief error description. Do not free this string.
    char const  *Position;    /// A pointer to the error position within the document.
    size_t       Line;        /// The line number within the document.
};

/// @summary Represents a single item or key-value pair within a JSON document.
/// Specific values for strings, integers, numbers and booleans can be read
/// using their own parsing functions. Note that any strings point into the
/// document buffer directly, and do not need to be freed separately.
struct json_item_t
{
    json_item_t *Parent;      /// A pointer to the parent node, or NULL.
    json_item_t *Next;        /// A pointer to the next sibling node, or NULL.
    json_item_t *FirstChild;  /// A pointer to the first child node, or NULL.
    json_item_t *LastChild;   /// A pointer to the last child node, or NULL.
    char        *Key;         /// NULL-terminated string field name.
    int32_t      ValueType;   /// One of json_item_type_e.
    union        ValueUnion
    {
        char    *string;      /// NULL-terminated string value; ValueType = JSON_TYPE_STRING.
        int64_t  integer;     /// Signed 64-bit integer value;  ValueType = JSON_TYPE_INTEGER.
        double   number;      /// Floating-point value; ValueType = JSON_TYPE_NUMBER.
        bool     boolean;     /// Boolean value; ValueType = JSON_TYPE_BOOLEAN.
    }            Value;       /// The item value. Objects/Arrays look at FirstChild.
};

/// @summary Function signature for a user-defined function that allocates a new
/// JSON item node. Note that nodes are always a fixed size, sizeof(json_item_t).
/// @param size_in_bytes The number of bytes to allocate. Always sizeof(json_item_t).
/// @param context Opaque data associated with the allocator. May be NULL.
/// @return The newly allocated item record, or NULL.
typedef json_item_t* (LLDATAIN_CALL_C *json_alloc_fn)(size_t size_in_bytes, void *context);

/// @summary Function signature for a user-defined function that releases memory
/// allocated for a JSON document node. Note that nodes are always a fixed size,
/// sizeof(json_item_t). The function can release the memory, return to free pool, etc.
/// @param item The document node to free.
/// @param size_in_bytes The number of bytes allocated to the node. Always sizeof(json_item_t).
/// @param context Opaque data associated with the allocator. May be NULL.
typedef void (LLDATAIN_CALL_C *json_free_fn)(json_item_t *item, size_t size_in_bytes, void *context);

/// @summary Describes a custom allocator for JSON document nodes. Use of a
/// custom allocator is option. The default implementation uses malloc()/free().
struct json_allocator_t
{
    json_alloc_fn Allocate;   /// The callback used to allocate a document node.
    json_free_fn  Release;    /// The callback used to free a document node.
    void         *Context;    /// Opaque data associated with the allocator.
};

/// @summary Define the RIFF header that appears at the start of a WAVE file.
#pragma pack(push, 1)
struct riff_header_t
{
    uint32_t ChunkId;         /// 'RIFF' (0x52494646)
    uint32_t DataSize;        /// The file size, minus 8 (for the header).
    uint32_t RiffType;        /// The file type, 'WAVE' (0x57415645)
};
#pragma pack(pop)

/// @summary Define the header that appears at the start of each RIFF chunk.
/// Note that chunk headers start on even addresses only.
#pragma pack(push, 1)
struct riff_chunk_header_t
{
    uint32_t ChunkId;         /// Varies; 'fmt ' (0x666D7420) and 'data' (0x64617461)
    uint32_t DataSize;        /// The size of the chunk data, not including the header.
};
#pragma pack(pop)

/// @summary Define the data comprising the WAV format chunk, used to describe sample data.
#pragma pack(push, 1)
struct wave_format_t
{
    uint16_t CompressionType; /// One of wav_compression_type_e.
    uint16_t ChannelCount;    /// Number of channels (1 = mono, 2 = stereo).
    uint32_t SampleRate;      /// Number of samples per-second.
    uint32_t BytesPerSecond;  /// Average number of bytes per-second for streaming.
    uint16_t BlockAlignment;  /// Number of bytes per-sample.
    uint16_t BitsPerSample;   /// Number of bytes per-sample.
    uint16_t FormatDataSize;  /// Size of the format-specific data, in bytes.
    uint8_t  FormatData[1];   /// Optional extra format-specific data.
};
#pragma pack(pop)

/// @summary Describes a chunk containing uncompressed PCM sample data.
struct wave_data_t
{
    size_t   DataSize;        /// The size of the sample data, in bytes.
    size_t   SampleCount;     /// The number of samples in the clip.
    void    *SampleData;      /// Pointer to the start of the clip sample data.
    float    Duration;        /// The clip duration, in seconds.
};

/// @summary Describes the TGA file header.
#pragma pack (push, 1)
struct tga_header_t
{
    uint8_t  ImageIdLength;   /// The length of the image ID string following the header.
    uint8_t  ColormapType;    /// One of tga_colormaptype_e.
    uint8_t  ImageType;       /// One of tga_imagetype_e.
    uint16_t CmapFirstEntry;  /// The index of the first colormap entry.
    uint16_t CmapLength;      /// The number of entries in the colormap.
    uint8_t  CmapEntrySize;   /// The size of a single colormap entry, in bits.
    uint16_t ImageXOrigin;    /// The origin point of the image, 0 = lower left corner.
    uint16_t ImageYOrigin;    /// The origin point of the image, 0 = lower left corner.
    uint16_t ImageWidth;      /// The width of the image, in pixels.
    uint16_t ImageHeight;     /// The height of the image, in pixels.
    uint8_t  ImageBitDepth;   /// The number of bits per-pixel.
    uint8_t  ImageFlags;      /// Bits 0..3 = # of attribute bits. Bits 4,5 = origin.
};
#pragma pack (pop)

/// @summary Describes the TGA file footer.
#pragma pack (push, 1)
struct tga_footer_t
{
    uint32_t ExtOffset;       /// The byte offset of the extension area.
    uint32_t DevOffset;       /// The byte offset of the developer directory.
    char     Signature[16];   /// TRUEVISION-XFILE
    char     PeriodChar;      /// A single '.' character.
    char     ZeroByte;        /// A single zero byte.
};
#pragma pack (pop)

/// @summary Describes the important data from a TGA image. The application is
/// responsible for decoding the pixel data.
struct tga_desc_t
{
    uint8_t  ColormapType;    /// One of tga_colormaptype_e.
    uint8_t  ImageType;       /// One of tga_imagetype_e.
    uint16_t CmapFirstEntry;  /// The index of the first colormap entry.
    uint16_t CmapLength;      /// The number of entries in the colormap.
    uint16_t CmapEntrySize;   /// The size of a single colormap entry, in bits.
    bool     OriginBottom;    /// true if the origin is the lower left corner.
    size_t   ImageWidth;      /// The width of the image, in pixels.
    size_t   ImageHeight;     /// The height of the image, in pixels.
    size_t   BitsPerPixel;    /// The number of bits per-pixel, including alpha.
    size_t   PixelDataSize;   /// Buffer size required to store decoded pixel data.
    size_t   ColormapDataSize;/// The size of the colormap data block, in bytes.
    void    *ColormapData;    /// Pointer to the start of the colormap data.
    void    *PixelData;       /// Pointer to the start of the image data.
};

/*////////////////
//   Functions  //
////////////////*/
/// @summary Gets the bytes (up to four) representing the Unicode BOM associated
/// with a specific text encoding.
/// @param encoding One of the text encoding constants, specifying the
/// encoding for which the BOM will be returned.
/// @param out_BOM An array of four bytes that will hold the BOM
/// corresponding to the specified text encoding. Between zero and four
/// bytes will be written to this array.
/// @return The number of bytes written to @a out_BOM.
LLDATAIN_PUBLIC size_t bom(int32_t encoding, uint8_t out_BOM[4]);

/// @summary Given four bytes possibly representing a Unicode byte-order-marker,
/// attempts to determine the text encoding and actual size of the BOM.
/// @param BOM The array of four bytes potentially containing a BOM.
/// @param out_size If this value is non-NULL, on return it is updated
/// with the actual size of the BOM, in bytes.
/// @return One of the text_encoding_e constants, specifying the text encoding
/// indicated by the BOM.
LLDATAIN_PUBLIC int32_t encoding(uint8_t BOM[4], size_t *out_size);

/// @summary Computes the maximum number of bytes required to base64-encode a
/// binary data block. All data is assumed to be output on one line. One byte
/// is included for a trailing NULL.
/// @param binary_size The size of the binary data block, in bytes.
/// @param out_pad_size If non-NULL, on return this value is updated with
/// the number of padding bytes that will be added during encoding.
/// @return The maximum number of bytes required to base64-encode a data
/// block of the specified size.
LLDATAIN_PUBLIC size_t base64_size(size_t binary_size, size_t *out_pad_size);

/// @summary Computes the number of raw bytes required to store a block of
/// binary data when converted back from base64. All source data is assumed to be on one line.
/// @param base64_size The size of the base64-encoded data.
/// @param pad_size The number of bytes of padding data. If not known, specify a value of zero.
/// @return The number of bytes of binary data generated during decoding.
LLDATAIN_PUBLIC size_t binary_size(size_t base64_size, size_t pad_size);

/// @summary Computes the number of raw bytes required to store a block of
/// binary data when converted back from base64. All source data is assumed to
/// be on one line. This version of the function examines the source data
/// directly, and so can provide a precise value.
/// @param base64_source Pointer to the start of the base64-encoded data.
/// @param base64_length The number of ASCII characters in the base64 data
/// string. This value can be computed using the standard C library strlen
/// function if the length is not otherwise available.
/// @return The number of bytes of binary data that will be generated during decoding.
LLDATAIN_PUBLIC size_t binary_size(char const *base64_source, size_t base64_length);

/// @summary Base64-encodes a block of arbitrary data. All data is returned on
/// a single line; no newlines are inserted and no formatting is performed. The
/// output buffer is guaranteed to be NULL-terminated.
/// @param dst Pointer to the start of the output buffer.
/// @param dst_size The maximum number of bytes that can be written to
/// the output buffer. This value must be at least as large as the value
/// returned by the data::base64_size() function.
/// @param src Pointer to the start of the input data.
/// @param src_size The number of bytes of input data to encode.
/// @return The number of bytes written to the output buffer.
LLDATAIN_PUBLIC size_t base64_encode(char *dst, size_t dst_size, void const *src, size_t src_size);

/// @summary Decodes a base64-encoded block of text into the corresponding raw binary representation.
/// @param dst Pointer to the start of the output buffer.
/// @param dst_size The maximum number of bytes that can be written to
/// the output buffer. This value can be up to two bytes less than the
/// value returned by data::binary_size() depending on the amount of
/// padding added during encoding.
/// @param src Pointer to the start of the base64-encoded input data.
/// @param src_size The number of bytes of input data to read and decode.
/// @return The number of bytes written to the output buffer.
LLDATAIN_PUBLIC size_t base64_decode(void *dst, size_t dst_size, char const *src, size_t src_size);

/// @summary Loads the entire contents of a text file into a buffer using the
/// stdio buffered file I/O routines. The buffer is allocated with malloc() and
/// should be freed using the free() function. The buffer is guaranteed to be
/// terminated with a zero byte, and the BOM (if present) is stripped from the data.
/// The file is opened in binary mode, and no translation is performed.
/// @param path The NULL-terminated path of the file to load.
/// @param out_buffer_size On return, this location is updated with the size of
/// the the returned buffer, in bytes, and NOT including the extra zero-byte or BOM.
/// @param out_encoding On return, this location is updated with the encoding
/// of the text data, if it could be determined, or TEXT_ENCODING_UNSURE otherwise.
/// @return A buffer containing the loaded, zero-terminated data, or NULL.
LLDATAIN_PUBLIC char* load_text(char const *path, size_t *out_buffer_size, data::text_encoding_e *out_encoding);

/// @summary Loads the entire contents of a file into a buffer using the stdio
/// buffered file I/O routines. The buffer is allocated with malloc() and should
/// be freed using the free() function.
/// @param path The NULL-terminated path of the file to load.
/// @param out_buffer_size On return, this location is updated with the size of
/// the the returned buffer, in bytes.
/// @return A buffer containing the loaded data, or NULL.
LLDATAIN_PUBLIC void* load_binary(char const *path, size_t *out_buffer_size);

/// @summary Reads the surface header present in all DDS files.
/// @param data The buffer from which the header data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param out_header Pointer to the structure to populate.
/// @return true if the file appears to be a valid DDS file.
LLDATAIN_PUBLIC bool dds_header(void const *data, size_t data_size, data::dds_header_t *out_header);

/// @summary Reads the extended surface header from a DDS buffer, if present.
/// @param data The buffer from which the header data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param out_header Pointer to the structure to populate.
/// @return true if the file contains an extended surface header.
LLDATAIN_PUBLIC bool dds_header_dxt10(void const *data, size_t data_size, data::dds_header_dxt10_t *out_header);

/// @summary Determines the DXGI_FORMAT value based on data in DDS headers.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @retur  One of the values of the dxgi_format_e enumeration.
LLDATAIN_PUBLIC uint32_t dds_format(data::dds_header_t const *header, data::dds_header_dxt10_t const *header_ex);

/// @summary Calculates the correct pitch value for a scanline, based on the
/// data format and width of the surface. This is necessary because many DDS
/// writers do not correctly compute the pitch value. See MSDN documentation at:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943991(v=vs.85).aspx
/// @param format One of the values of the dxgi_format_e enumeration.
LLDATAIN_PUBLIC size_t dds_pitch(uint32_t format, size_t width);

/// @summary Determines if a DXGI format value is a block-compressed format.
/// @param format One of dxgi_format_e.
/// @return true if format is one of DXGI_FORMAT_BCn.
LLDATAIN_PUBLIC bool dds_block_compressed(uint32_t format);

/// @summary Determines if a DXGI fomrat value specifies a packed format.
/// @param format One of dxgi_format_e.
/// @return true if format is one of DXGI_FORMAT_R8G8_B8G8_UNORM or DXGI_FORMAT_G8R8_G8B8_UNORM.
LLDATAIN_PUBLIC bool dds_packed(uint32_t format);

/// @summary Determines if a DDS describes a cubemap surface.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @return true if the DDS describes a cubemap.
LLDATAIN_PUBLIC bool dds_cubemap(data::dds_header_t const *header, data::dds_header_dxt10_t const *header_ex);

/// @summary Determines if a DDS describes a volume surface.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @return true if the DDS describes a volume.
LLDATAIN_PUBLIC bool dds_volume(data::dds_header_t const *header, data::dds_header_dxt10_t const *header_ex);

/// @summary Determines if a DDS describes a surface array. Note that a volume
/// is not considered to be the same as a surface array.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @return true if the DDS describes a surface array.
LLDATAIN_PUBLIC bool dds_array(data::dds_header_t const *header, data::dds_header_dxt10_t const *header_ex);

/// @summary Determines if a DDS describes a mipmap chain.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @return true if the DDS describes a mipmap chain.
LLDATAIN_PUBLIC bool dds_mipmap(data::dds_header_t const *header, data::dds_header_dxt10_t const *header_ex);

/// @summary Calculate the number of bits-per-pixel for a given format. Block-
/// compressed formats are supported as well.
/// @param format One of dxgi_format_e.
/// @return The number of bits per-pixel.
LLDATAIN_PUBLIC size_t dds_bits_per_pixel(uint32_t format);

/// @summary Calculate the number of bytes per 4x4-pixel block.
/// @param format One of dxgi_format_e.
/// @return The number of bytes in a 4x4 pixel block, or 0 for non-block-compressed formats.
LLDATAIN_PUBLIC size_t dds_bytes_per_block(uint32_t format);

/// @summary Determines the number of elements in a surface array.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @return The number of elements in the surface array, or 1 if the DDS does not describe an array.
LLDATAIN_PUBLIC size_t dds_array_count(data::dds_header_t const *header, data::dds_header_dxt10_t const *header_ex);

/// @summary Determines the number of levels in the mipmap chain.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @return The number of levels in the mipmap chain, or 1 if the DDS describes the top level only.
LLDATAIN_PUBLIC size_t dds_level_count(data::dds_header_t const *header, data::dds_header_dxt10_t const *header_ex);

/// @summary Retrieves a description of and pointer to the start of the data for a mipmap level.
/// @param data The buffer from which the data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param header The base surface header of the DDS.
/// @param header_ex The extended surface header of the DDS, or NULL.
/// @param out_levels A buffer of dds_array_count() * dds_level_count() level
/// descriptors to populate with data (or max_levels, whichever is less.)
/// @param max_levels The maximum number of items to write to out_levels.
/// @return The number of level descriptors written to out_levels.
LLDATAIN_PUBLIC size_t dds_describe(
    void                     const *data,
    size_t                          data_size,
    data::dds_header_t       const *header,
    data::dds_header_dxt10_t const *header_ex,
    data::dds_level_desc_t         *out_levels,
    size_t                          max_levels);

/// @summary Describes the format of uncompressed PCM sound data stored in a
/// RIFF WAVE container. Compressed audio is not supported.
/// @param data The buffer from which data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param out_desc Pointer to a structure to populate with sound format information.
/// @param out_clips An array of clip descriptors to populate.
/// @param max_clips The maximum number of clip descriptors to write to out_clips.
/// @return The number of clip descriptors written to out_clips.
LLDATAIN_PUBLIC size_t wav_describe(
    void const          *data,
    size_t               data_size,
    data::wave_format_t *out_desc,
    data::wave_data_t   *out_clips,
    size_t               max_clips);

/// @summary Parses a string value representing a signed 64-bit base-10 integer.
/// @param first Pointer to the first character to inspect.
/// @param last Pointer to the last character to inspect.
/// @param out On return, this location is updated with the parsed value.
/// @return A pointer to the end of the value, or first if the number could not be parsed.
LLDATAIN_PUBLIC char* str_to_dec_s64(char *first, char *last, int64_t *out);

/// @summary Parses a string value representing a unsigned 32-bit base-16 integer.
/// @param first Pointer to the first character to inspect.
/// @param last Pointer to the last character to inspect.
/// @param out On return, this location is updated with the parsed value.
/// @return A pointer to the end of the value, or first if the number could not be parsed.
LLDATAIN_PUBLIC char* str_to_hex_u32(char *first, char *last, uint32_t *out);

/// @summary Parses a string value representing a unsigned 64-bit base-16 integer.
/// @param first Pointer to the first character to inspect.
/// @param last Pointer to the last character to inspect.
/// @param out On return, this location is updated with the parsed value.
/// @return A pointer to the end of the value, or first if the number could not be parsed.
LLDATAIN_PUBLIC char* str_to_hex_u64(char *first, char *last, uint64_t *out);

/// @summary Parses a string value representing a decimal value.
/// @param first Pointer to the first character to inspect.
/// @param last Pointer to the last character to inspect.
/// @param out On return, this location is updated with the parsed value.
/// @return A pointer to the end of the value, or first if the number could not be parsed.
LLDATAIN_PUBLIC char* str_to_num_f64(char *first, char *last, double *out);

/// @summary Initializes a JSON allocator instance.
/// @param allocator The allocator to initialize.
/// @param alloc_func The callback used to allocate a JSON document node. Required.
/// @param free_func The callback used to release the memory allocated to a JSON
/// document node. Optional. Defaults to a no-op if not specified.
/// @param context Opaque data associated with the allocator implementation. Optional.
LLDATAIN_PUBLIC void json_allocator_init(
    data::json_allocator_t *allocator,
    data::json_alloc_fn     alloc_func,
    data::json_free_fn      free_func,
    void                   *context);

/// @summary Initializes a JSON document node instance.
/// @param item The document node to initialize.
LLDATAIN_PUBLIC void json_item_init(data::json_item_t *item);

/// @summary Inserts a single item into a JSON document tree. This is a utility
/// function that can be used to manually construct a JSON document.
/// @param parent The parent item, or NULL if new_node is the document root.
/// @param new_node The new child node.
LLDATAIN_PUBLIC void json_document_append(data::json_item_t *parent, data::json_item_t *new_node);

/// @summary Performs in-place parsing and validation of a JSON document. Note
/// that string keys and values in the generated document nodes are pointers
/// directly into the document buffer. This function modifies the contents of
/// the document buffer by overwriting data with zero bytes for string termination.
/// @param document The buffer containing the JSON document.
/// @param document_size The size of the input document buffer, in bytes.
/// @param allocator The custom memory allocator implementation to use for creating
/// JSON document nodes. Pass NULL to use the default malloc() and free().
/// @param out_root On return, this location points to the root node of the JSON
/// document. If an error occurs, this location points to NULL.
/// @param out_error If the function returns false, this location is updated with
/// a details about the error that was encountered.
/// @return true if the document was parsed successfully.
LLDATAIN_PUBLIC bool json_parse(
    char                   *document,
    size_t                  document_size,
    data::json_allocator_t *allocator,
    data::json_item_t     **out_root,
    data::json_error_t     *out_error);

/// @summary Releases the memory associated with each node in the JSON document.
/// @param item The JSON document node to free. Pass the root node to free the entire document tree.
/// @param allocator The same allocator implementation passed to json_parse().
LLDATAIN_PUBLIC void json_free(data::json_item_t *item, data::json_allocator_t *allocator);

/// @summary Retrieves a description of a bitmap font stored in the BMfont binary format.
/// @param data The buffer from which the data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param out_desc On return, this structure is populated with pointers to the
/// various data blocks within the BMfont.
/// @return true if the font was parsed successfully.
LLDATAIN_PUBLIC bool bmfont_describe(void const *data, size_t data_size, data::bmfont_desc_t *out_desc);

/// @summary Updates in-place the file extensions for the page filenames of a BMfont.
/// This is useful if you have converted the image data to another format but don't 
/// want to re-write the font file (some platforms might only export PNG but you want DDS.)
/// @param desc The description populated by bmfont_describe().
/// @param new_ext The new extension, without leading period, for example, "dds". This must
/// be the same length (or less) than the existing extension.
/// @return true if the extensions were changed.
LLDATAIN_PUBLIC bool bmfont_change_extensions(data::bmfont_desc_t *desc, char const *new_ext);

/// @summary Reads the header present in all TGA files.
/// @param data The buffer from which the header data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param out_header Pointer to the structure to populate.
/// @return true if the file appears to be a valid TGA file.
LLDATAIN_PUBLIC bool tga_header(void const *data, size_t data_size, data::tga_header_t *out_header);

/// @summary Reads the footer present in TGA version 2 files.
/// @param data The buffer from which the header data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param out_footer Pointer to the structure to populate.
/// @return true if the file contains a valid footer and is thus a version 2 image.
LLDATAIN_PUBLIC bool tga_footer(void const *data, size_t data_size, data::tga_footer_t *out_footer);

/// @summary Retrieves a description of a TGA image.
/// @param data The buffer from which the data should be read.
/// @param data_size The maximum number of bytes to read from the input buffer.
/// @param out_desc On return, this structure is populated with information about the image.
/// @return true if the image was parsed successfully.
LLDATAIN_PUBLIC bool tga_describe(void const *data, size_t data_size, data::tga_desc_t *out_desc);

/// @summary Decodes 8-bit grayscale TGA data into a caller-managed buffer. The
/// TGA may have image type TGA_IMAGETYPE_UNCOMPRESSED_GRAY or TGA_IMAGETYPE_RLE_GRAY.
/// Any other image type will cause the call to fail.
/// @param dst The buffer to write to, of at least ImageWidth * ImageHeight bytes.
/// @param dst_size The maximum number of bytes to write to the destination buffer.
/// @param desc A description of the data in the TGA image.
/// @return true if the image data was decoded and written to the output buffer.
LLDATAIN_PUBLIC bool tga_decode_r8(void *dst, size_t dst_size, data::tga_desc_t const *desc);

/// @summary Decodes 16/24/32 bit TGA data into a caller-managed buffer. Pixels
/// are logically stored in ARGB order, but on a little-endian machine appear
/// in BGRA byte order. Use GL_BGRA/GL_UNSIGNED_INT_8_8_8_8_REV to upload this
/// data to OpenGL most efficiently. Uncompressed, palettized and RLE-encoded
/// images are supported, but grayscale images should use tga_decode_r8().
/// @param dst The buffer to write to, of at least ImageWidth * ImageHeight * 4 bytes.
/// @param dst_size The maximum number of bytes to write to the destination buffer.
/// @param desc A description of the data in the TGA image.
/// @return true if the image data was decoded and written to the output buffer.
LLDATAIN_PUBLIC bool tga_decode_argb32(void *dst, size_t dst_size, data::tga_desc_t const *desc);

/// @summary Generates a little-endian FOURCC.
/// @param a...d The four characters comprising the code.
/// @return The packed four-cc value, in little-endian format.
static inline uint32_t fourcc_le(char a, char b, char c, char d)
{
    uint32_t A = (uint32_t) a;
    uint32_t B = (uint32_t) b;
    uint32_t C = (uint32_t) c;
    uint32_t D = (uint32_t) d;
    return ((D << 24) | (C << 16) | (B << 8) | (A << 0));
}

/// @summary Generates a big-endian FOURCC.
/// @param a...d The four characters comprising the code.
/// @return The packed four-cc value, in big-endian format.
static inline uint32_t fourcc_be(char a, char b, char c, char d)
{
    uint32_t A = (uint32_t) a;
    uint32_t B = (uint32_t) b;
    uint32_t C = (uint32_t) c;
    uint32_t D = (uint32_t) d;
    return ((A << 24) | (B << 16) | (C << 8) | (D << 0));
}

/*/////////////////////
//   Namespace End   //
/////////////////////*/
}; /* end namespace data */

#endif /* !defined(LLDATAIN_HPP_INCLUDED) */
