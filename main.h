/* date = August 3rd 2025 7:21 pm */

#ifndef MAIN_H
#define MAIN_H

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;
typedef int32 bool32;

#define true 1
#define false 0
#define PI 3.14159265359f

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))
#define Assert(Expression) if(Expression == 0) { *(uint32 *)0 = 0;}

#define KILOBYTE(Value) (Value << 10)
#define MEGABYTE(Value) (Value << 20)
#define GIGABYTE(Value) (Value << 30)

typedef struct
{
    void *Memory;
    uint32 Width;
    uint32 Height;
    uint32 Stride;
    uint32 BytesPerPixel;
    BITMAPINFO BitmapInfo;
    real32 tPerFrame;
}win32_pixel_buffer;


#pragma pack(push, 1)
typedef struct
{
    //14 Bytes
    WORD   FileType;     /* File type, always 4D42h ("BM") */
	DWORD  FileSize;     /* Size of the file in bytes */
	WORD   Reserved1;    /* Always 0 */
	WORD   Reserved2;    /* Always 0 */
	DWORD  BitmapOffset; /* Starting position of image data in bytes */
    
    //56 Bytes
    DWORD Size;            /* Size of this header in bytes */
	LONG  Width;           /* Image width in pixels */
	LONG  Height;          /* Image height in pixels */
	WORD  Planes;          /* Number of color planes */
	WORD  BitsPerPixel;    /* Number of bits per pixel */
	DWORD Compression;     /* Compression methods used */
	DWORD SizeOfBitmap;    /* Size of bitmap in bytes */
	LONG  HorzResolution;  /* Horizontal resolution in pixels per meter */
	LONG  VertResolution;  /* Vertical resolution in pixels per meter */
	DWORD ColorsUsed;      /* Number of colors in the image */
	DWORD ColorsImportant; /* Minimum number of important colors */
	/* Fields added for Windows 4.x follow this line */
    
	DWORD RedMask;       /* Mask identifying bits of red component */
	DWORD GreenMask;     /* Mask identifying bits of green component */
	DWORD BlueMask;      /* Mask identifying bits of blue component */
    DWORD AlphaMask;
}bitmap_header;
#pragma pack(pop)

typedef struct
{
    real32 X;
    real32 Y;
}vec2;

typedef struct
{
    real32 X;
    real32 Y;
    real32 Z;
}vec3;

typedef struct
{
    real32 X;
    real32 Y;
    real32 Z;
    real32 W;
}vec4;

typedef struct
{
    real32 X;
    real32 Y;
    real32 Z;
    real32 U;
    real32 V;
}vec5;

typedef struct
{
    real32 M[4][4];
}mat4;

typedef struct
{
    uint32 A;
    uint32 B;
    uint32 C;
    uint32 T1;
    uint32 T2;
    uint32 T3;
    uint32 Color;
    real32 AverageZ;
}triangle;

typedef struct
{
    vec3 *Vertices;
    uint32 VertexCount;
    triangle *Triangles;
    uint32 TriangleCount;
    vec2 *TextureCoords;
    uint32 TextureCount;
}mesh;

typedef struct
{
    vec3 Direction;
    vec3 NormalizedDirection;
}light;

typedef struct
{
    void *Base;
    uint32 Size;
    uint32 Used;
}memory_arena;

typedef struct
{
    uint32 Size;
    uint32 *Elements;
    int32 Pointer;
}stack;

typedef struct
{
    uint32 Size;
    void *Contents;
}file;

typedef struct
{
    uint32 Width;
    uint32 Height;
    uint32 BytesPerTexel;
    uint32 *Bytes;
}texture;

//
#define PushStruct(Arena, Size) PushSize(Arena, Size)
#define PushArray(Arena, Array) PushSize(Arena, sizeof(Array))

inline void *PushSize(memory_arena *Arena, uint32 Size)
{
    void *Result = (uint8 *)Arena->Base + Arena->Used;
    Arena->Used += Size;
    return Result;
}

#endif //MAIN_H
