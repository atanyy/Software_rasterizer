#include "windows.h"
#include "stdint.h"
#include "stdio.h"
#include "math.h"

#include "main.h"
#include "renderer_utilities.c"

#include "obj_parser.c"
#include "vector.c"
#include "line.c"
#include "random.h"
#include "perspective_texture_map.c"

static bool32 GlobalRunning;
static win32_pixel_buffer GlobalPixelBuffer;
static LARGE_INTEGER GlobalPerfFrequency;

static void 
InitializeBitmapInfo(win32_pixel_buffer *Buffer, int32 Width, int32 Height)
{
    BITMAPINFOHEADER *BitmapInfoHeader = &Buffer->BitmapInfo.bmiHeader;
    
    BitmapInfoHeader->biSize = sizeof(BITMAPINFOHEADER);
    BitmapInfoHeader->biWidth = Width;
    BitmapInfoHeader->biHeight = -Height;
    BitmapInfoHeader->biPlanes = 1;
    BitmapInfoHeader->biBitCount = 32;
    BitmapInfoHeader->biCompression = BI_RGB;
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
    Buffer->Stride = Buffer->Width * Buffer->BytesPerPixel;
    
    uint32 BufferSize = Buffer->Height * Buffer->Stride;
    Buffer->Memory = VirtualAlloc(0, BufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static void DrawPixelOnly(win32_pixel_buffer *Buffer, vec2 Vector, uint32 Color)
{
    uint32 X = RoundReal32ToUInt32(Vector.X);
    uint32 Y = RoundReal32ToUInt32(Vector.Y);
    
    uint32 *Pixel = (uint32 *)((uint8 *)Buffer->Memory + (Y * Buffer->Stride) + (X * Buffer->BytesPerPixel));
    
    *Pixel = Color;
}

static void
DrawFloatLineBresenham(win32_pixel_buffer *Buffer, vec2 VectorA, vec2 VectorB, uint32 Color)
{
    vec2 SourceCoord = Vec2CoordToScreenCoord(Buffer, VectorA);
    vec2 DestCoord = Vec2CoordToScreenCoord(Buffer, VectorB);
    
    int32 Scale = 100;
    
    uint32 SourceX = RoundReal32ToUInt32(SourceCoord.X * (real32)Scale);
    uint32 SourceY = RoundReal32ToUInt32(SourceCoord.Y * (real32)Scale);
    uint32 DestX = RoundReal32ToUInt32(DestCoord.X * (real32)Scale);
    uint32 DestY = RoundReal32ToUInt32(DestCoord.Y * (real32)Scale);
    
    uint32 deltaX = abs(DestX - SourceX);
    uint32 deltaY = abs(DestY - SourceY);
    
    bool32 IsSteep = false;
    if(deltaY > deltaX)
    {
        SwapUInt32(&SourceX, &SourceY);
        SwapUInt32(&DestX, &DestY);
        SwapUInt32(&deltaX, &deltaY);
        IsSteep = true;
    }
    
    if(DestX < SourceX)
    {
        SwapUInt32(&SourceX, &DestX);
        SwapUInt32(&SourceY, &DestY);
    }
    
    //real32 Error = (0.5f + fmodf(SourceX, 1.0f) * (deltaY / deltaX)) + (fmodf(SourceY, 1.0f)) - 0.5f;
    
    int32 Error = (Scale - 2 * (SourceX % Scale)) * deltaY + (2 * deltaX * (SourceY % Scale)) - (deltaX * Scale);
    
    //real32 Slope = deltaY / deltaX;
    
    uint32 Y = SourceY;
    for(uint32 X = SourceX; X <= DestX; X += Scale)
    {
        //Draw Pixel
        if(IsSteep)
        {
            DrawPixelOnly(Buffer, (vec2){Y / (real32)Scale, X / (real32)Scale}, Color);
        }
        else
        {
            DrawPixelOnly(Buffer, (vec2){X / (real32)Scale, Y / (real32)Scale}, Color);
        }
        
        //char OutputBuffer[256];
        //sprintf_s(OutputBuffer, 256, "X:%f, Y:%f\n", X / (real32)Scale, Y / (real32)Scale);
        //OutputDebugStringA(OutputBuffer);
        
        //
        Error += (Scale * 2 * deltaY);
        
        if(Error > (Scale * (int32)deltaX))
        {
            Y += (DestY < SourceY) ? -Scale : Scale;
            Error -= (Scale * 2 * deltaX);
        }
    }
}



static void DrawPixel(win32_pixel_buffer *Buffer, vec2 Vector, uint32 Color)
{
    vec2 ScreenCoord = Vec2CoordToScreenCoord(Buffer, Vector);
    
    uint32 X = RoundReal32ToUInt32(ScreenCoord.X);
    uint32 Y = RoundReal32ToUInt32(ScreenCoord.Y);
    
    uint32 *Pixel = (uint32 *)((uint8 *)Buffer->Memory + (Y * Buffer->Stride) + (X * Buffer->BytesPerPixel));
    
    *Pixel = Color;
}

static void DrawRect(win32_pixel_buffer *Buffer, vec2 LeftTop, vec2 RightBottom, uint32 Color)
{
    int32 MinX = (int32)LeftTop.X;
    int32 MinY = (int32)LeftTop.Y;
    int32 MaxX = (int32)RightBottom.X;
    int32 MaxY = (int32)RightBottom.Y;
    
    uint8 *Row = (uint8 *)Buffer->Memory + (MinY * Buffer->Stride) + (MinX * Buffer->BytesPerPixel);
    for(int32 Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int32 X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }
        Row += Buffer->Stride;
    }
}


static void 
DrawTriangle(win32_pixel_buffer *Buffer, vec2 PointA, vec2 PointB, vec2 PointC, uint32 Color)
{
    DrawLineBresenham(Buffer, PointA, PointB, Color);
    DrawLineBresenham(Buffer, PointB, PointC, Color);
    DrawLineBresenham(Buffer, PointC, PointA, Color);
}

static void
Win32UpdateWindow(win32_pixel_buffer *Buffer, HDC DeviceContext, uint32 WindowWidth, uint32 WindowHeight)
{
    StretchDIBits(
                  DeviceContext,
                  0,
                  0,
                  Buffer->Width,
                  Buffer->Height,
                  0,
                  0,
                  Buffer->Width,
                  Buffer->Height,
                  Buffer->Memory,
                  &Buffer->BitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY
                  );
}

LRESULT Win32WindowProcedure(
                             HWND WindowHandle,
                             UINT Message,
                             WPARAM WParam,
                             LPARAM LParam
                             )
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }break;
        default:
        {
            Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }break;
    }
    
    return Result;
}

static void ReadBitmap(file *File, char *FileName)
{
    HANDLE FileHandle = CreateFileA(FileName,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    0,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_READONLY,
                                    0);
    
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER Size;
        
        if(GetFileSizeEx(FileHandle, &Size))
        {
            File->Size = (uint32)Size.QuadPart;
            
            File->Contents = VirtualAlloc(0, File->Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            DWORD BytesRead;
            if(ReadFile(FileHandle, File->Contents, File->Size, &BytesRead, 0) && (File->Size == BytesRead))
            {
                CloseHandle(FileHandle);
            }
        }
        
        
    }
}

static void
WriteBitmap(bitmap_header *BitmapHeader, uint32 *BitmapBits, char *FileName)
{
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_READONLY, 0);
    
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        WriteFile(FileHandle, BitmapHeader, sizeof(bitmap_header), &BytesWritten, 0);
        
        if(sizeof(bitmap_header) == BytesWritten)
        {
            DWORD BitmapBytesWritten;
            WriteFile(FileHandle, BitmapBits, BitmapHeader->SizeOfBitmap, &BitmapBytesWritten, 0);
            
            if(BitmapHeader->SizeOfBitmap == BitmapBytesWritten)
            {
                CloseHandle(FileHandle);
            }
        }
    }
}

static void 
SaveBitmap(win32_pixel_buffer *Buffer, bitmap_header *BitmapHeader, uint32 *BitmapBits, char *FileName)
{
    uint32 *Bits = BitmapBits;
    uint8 *Row = (uint8 *)Buffer->Memory;
    for(int32 Y = 0; Y < BitmapHeader->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int32 X = 0; X < BitmapHeader->Width; ++X)
        {
            *Bits++ = *Pixel++;
        }
        Row += Buffer->Stride;
    }
    
    WriteBitmap(BitmapHeader, BitmapBits, FileName);
}



static void
DrawGrid(win32_pixel_buffer *Buffer, uint32 GridWidth, uint32 GridHeight, uint32 SourceColor)
{
    uint8* Row = (uint8*)Buffer->Memory;
    
    uint32 AlphaBitScan = 24;
    uint32 RedBitScan = 16;
    uint32 GreenBitScan = 8;
    uint32 BlueBitScan = 0;
    
    uint32 AlphaMask = (0xFF << AlphaBitScan);
    uint32 RedMask = (0xFF << RedBitScan);
    uint32 GreenMask = (0xFF << GreenBitScan);
    uint32 BlueMask = (0xFF << BlueBitScan);
    
    for(uint32 Y = 1; Y <= Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row; 
        for(uint32 X = 1; X <= Buffer->Width; ++X)
        {
            if(X % GridWidth == 0 || Y % GridHeight == 0)
            {
                uint32 DestColor = *Pixel;
                uint32 DestRed = (DestColor & RedMask) >> RedBitScan;
                uint32 DestGreen = (DestColor & GreenMask) >> GreenBitScan;
                uint32 DestBlue = (DestColor & BlueMask) >> BlueBitScan;
                
                uint32 SourceRed = (SourceColor & RedMask) >> RedBitScan;
                uint32 SourceGreen = (SourceColor & GreenMask) >> GreenBitScan;
                uint32 SourceBlue = (SourceColor & BlueMask) >> BlueBitScan;
                
                real32 Alpha = ((SourceColor & AlphaMask) >> AlphaBitScan) / 255.0f;
                uint32 Red = (uint32)((1 - Alpha) * (real32)DestRed + (Alpha * (real32)SourceRed));
                uint32 Green = (uint32)((1 - Alpha) * (real32)DestGreen + (Alpha * (real32)SourceGreen));
                uint32 Blue = (uint32)((1 - Alpha) * (real32)DestBlue + (Alpha * (real32)SourceBlue));
                
                uint32 Color = (0xFF << AlphaBitScan) |
                (Red << RedBitScan) | (Green << GreenBitScan) | (Blue << BlueBitScan);
                
                *Pixel = Color;
            }
            ++Pixel;
        }
        Row += Buffer->Stride;
    }
}



void SortVerticesVec2(vec2 *Vertices)
{
    if(Vertices[1].Y <= Vertices[0].Y && Vertices[1].Y <= Vertices[2].Y)
    {
        if(Vertices[0].Y <= Vertices[2].Y)
        {
            SwapVec2(&Vertices[1], &Vertices[0]);
        }
        else
        {
            SwapVec2(&Vertices[1], &Vertices[0]);
            SwapVec2(&Vertices[1], &Vertices[2]);
        }
    }
    else if(Vertices[2].Y <= Vertices[0].Y && Vertices[2].Y <= Vertices[1].Y)
    {
        if(Vertices[1].Y <= Vertices[0].Y)
        {
            SwapVec2(&Vertices[2], &Vertices[0]);
        }
        else
        {
            SwapVec2(&Vertices[2], &Vertices[0]);
            SwapVec2(&Vertices[2], &Vertices[1]);
        }
    }
    else
    {
        if(Vertices[2].Y <= Vertices[1].Y)
        {
            SwapVec2(&Vertices[2], &Vertices[1]);
        }
        else
        {
            //Do Nothing
        }
    }
}

vec2 FindVertexSideIntersection(vec2 V1, vec2 V2, vec2 V3)
{
    vec2 Result;
    
    Result.Y = V2.Y;
    
    real32 IntermediateResult = (V3.X - V1.X) * (V1.Y - V2.Y) / (V1.Y - V3.Y);
    
    Result.X = IntermediateResult + V1.X;
    
    return Result;
}

void DrawHorizontalLine(win32_pixel_buffer *Buffer, uint32 X1, uint32 X2, uint32 Y, uint32 Color)
{
    if(X2 < X1)
    {
        SwapUInt32(&X2, &X1);
    }
    
    uint32 *Pixel = (uint32 *)((uint8 *)Buffer->Memory + (Y * Buffer->Stride) + (X1 * Buffer->BytesPerPixel));
    
    for(uint32 X = X1; X < X2; ++X)
    {
        *Pixel++ = Color;
    }
}

void FillFlatBottomTriangle(win32_pixel_buffer *Buffer, vec2 V1, vec2 V2, vec2 V3, uint32 Color)
{
    uint32 ScreenV1Y = (uint32)ceilf(V1.Y);
    uint32 ScreenV2Y = (uint32)ceilf(V2.Y);
    uint32 ScreenV3Y = (uint32)ceilf(V3.Y);
    
    real32 deltaX1 = V2.X - V1.X;
    real32 deltaY1 = V2.Y - V1.Y;
    real32 deltaX2 = V3.X - V1.X;
    real32 deltaY2 = V3.Y - V1.Y;
    
    real32 YPrestep = ScreenV1Y - V1.Y;
    real32 XStart = ((YPrestep) * deltaX1 / deltaY1) + V1.X;
    real32 XEnd = ((YPrestep) * deltaX2 / deltaY2) + V1.X;
    
    uint32 X1 = (uint32)ceilf(XStart);
    uint32 X2 = (uint32)ceilf(XEnd);
    
    real32 InvSlope1 = (real32)deltaX1 / (real32)deltaY1;
    real32 InvSlope2 = (real32)deltaX2 / (real32)deltaY2;
    
    int32 YStart = ScreenV1Y;
    int32 YEnd = ScreenV3Y;
    
    for(int32 Y = YStart; Y < YEnd; ++Y)
    {
        if(X1 < (V1.X - 2) && X1 < (V2.X - 2) && X1 < (V3.X - 2))
        {
            OutputDebugStringA("Uh\n");
        }
        
        DrawHorizontalLine(Buffer, X1, X2, Y, Color);
        
        XStart += InvSlope1;
        XEnd += InvSlope2;
        
        X1 = (uint32)ceilf(XStart);
        X2 = (uint32)ceilf(XEnd);
    }
}

void FillFlatTopTriangle(win32_pixel_buffer *Buffer, vec2 V1, vec2 V2, vec2 V3, uint32 Color)
{
    uint32 ScreenV1Y = (uint32)ceilf(V1.Y);
    uint32 ScreenV2Y = (uint32)ceilf(V2.Y);
    uint32 ScreenV3Y = (uint32)ceilf(V3.Y);
    
    real32 deltaX1 = V3.X - V1.X;
    real32 deltaY1 = V3.Y - V1.Y;
    real32 deltaX2 = V3.X - V2.X;
    real32 deltaY2 = V3.Y - V2.Y;
    
    real32 YPrestep = ScreenV1Y - V1.Y;
    real32 XStart = ((YPrestep) * deltaX1 / deltaY1) + V1.X;
    real32 XEnd = ((YPrestep) * deltaX2 / deltaY2) + V2.X;
    
    uint32 X1 = (uint32)ceilf(XStart);
    uint32 X2 = (uint32)ceilf(XEnd);
    
    real32 InvSlope1 = (real32)deltaX1 / (real32)deltaY1;
    real32 InvSlope2 = (real32)deltaX2 / (real32)deltaY2;
    
    int32 YStart = ScreenV1Y;
    int32 YEnd = ScreenV3Y;
    
    for(int32 Y = YStart; Y < YEnd; ++Y)
    {
        if(X1 < (V1.X - 2) && X1 < (V2.X - 2) && X1 < (V3.X - 2))
        {
            OutputDebugStringA("Uh\n");
        }
        
        DrawHorizontalLine(Buffer, X1, X2, Y, Color);
        
        XStart += InvSlope1;
        XEnd += InvSlope2;
        
        X1 = (uint32)ceilf(XStart);
        X2 = (uint32)ceilf(XEnd);
    }
}


static void FillTriangle(win32_pixel_buffer *Buffer, vec2 V1, vec2 V2, vec2 V3, uint32 Color)
{
    vec2 SortedVertices[3];
    SortedVertices[0] = Vec2CoordToScreenCoord(Buffer, V1);
    SortedVertices[1] = Vec2CoordToScreenCoord(Buffer, V2);
    SortedVertices[2] = Vec2CoordToScreenCoord(Buffer, V3);
    
    SortVerticesVec2(SortedVertices);
    
    if(SortedVertices[0].Y == SortedVertices[1].Y)
    {
        FillFlatTopTriangle(Buffer, SortedVertices[0], SortedVertices[1], SortedVertices[2], Color);
    }
    else if(SortedVertices[1].Y == SortedVertices[2].Y)
    {
        FillFlatBottomTriangle(Buffer, SortedVertices[0], SortedVertices[1], SortedVertices[2], Color);
    }
    else
    {
        
        vec2 IntersectPoint = FindVertexSideIntersection(SortedVertices[0], SortedVertices[1], SortedVertices[2]);
        
        FillFlatBottomTriangle(Buffer, SortedVertices[0], SortedVertices[1], IntersectPoint, Color);
        //DrawTriangle(Buffer, SortedVertices[0], SortedVertices[1], IntersectPoint, 0xFFFFFFFF);
        
        FillFlatTopTriangle(Buffer, SortedVertices[1], IntersectPoint, SortedVertices[2], Color);
        //DrawTriangle(Buffer, SortedVertices[1], IntersectPoint, SortedVertices[2], 0xFFFFFFFF);
    }
}

void InsertionSort(int32 *Array, uint32 Left, uint32 Right)
{
    uint32 Size = Right - Left;
    for(uint32 I = Left; I < Size; I++)
    {
        for(uint32 J = I; J > 0 && Array[J-1] < Array[J]; J--)
        {
            SwapInt32(&Array[J-1], &Array[J]);
        }
    }
}

void StackPush(stack *Stack, uint32 Value)
{
    Assert((Stack->Pointer + 1) < (int32)Stack->Size);
    
    ++Stack->Pointer;
    Stack->Elements[Stack->Pointer] = Value;
}

uint32 StackPop(stack *Stack)
{
    Assert(Stack->Pointer != -1);
    
    uint32 Result = Stack->Elements[Stack->Pointer];
    --Stack->Pointer;
    
    return Result;
}

void QuickSort(memory_arena *Arena, triangle *Array, uint32 ArraySize)
{
    stack Stack;
    Stack.Size = ((uint32)log2(ArraySize) * 2) + 2;
    Stack.Pointer = 2;
    Stack.Elements = PushArray(Arena, Stack.Size);
    
    
    int32 L, R;
    L = 0;
    R = ArraySize - 1;
    do
    {
        if(R > L)
        {
            int32 I, J;
            
            I = L - 1;
            real32 Pivot = Array[R].AverageZ;
            J = R;
            
            while(I < J)
            {
                do
                {
                    ++I;
                }while(I <= J && Array[I].AverageZ < Pivot);
                
                do
                {
                    --J;
                }while(J >= I && Array[J].AverageZ > Pivot);
                
                if(I < J)
                {
                    triangle Temp = Array[I];
                    Array[I] = Array[J];
                    Array[J] = Temp;
                }
            }
            
            triangle Temp = Array[I];
            Array[I] = Array[R];
            Array[R] = Temp;
            
            if((I - L) > (R - I))
            {
                StackPush(&Stack, L);
                StackPush(&Stack, I - 1);
                L = I + 1;
            }
            else
            {
                StackPush(&Stack, I + 1);
                StackPush(&Stack, R);
                R = I - 1;
            }
        }
        else
        {
            R = StackPop(&Stack);
            L = StackPop(&Stack);
        }
    } while(Stack.Pointer > 0);
}

mat4 CreatePerspectiveMatrix(real32 AngleOfView, real32 InvAspectRatio, real32 NearZ, real32 FarZ)
{
    mat4 Result = {{0}};
    
    real32 FOVScale = 1.0f / tanf(AngleOfView / 2.0f);
    Result.M[0][0] = FOVScale * InvAspectRatio;
    Result.M[1][1] = FOVScale;
    Result.M[2][2] = (FarZ + NearZ) / (FarZ - NearZ);
    Result.M[2][3] = (-2.0f * NearZ * FarZ) / (FarZ - NearZ);
    Result.M[3][2] = 1.0f;
    
    return Result;
}

static void 
DrawMesh(memory_arena *Arena, win32_pixel_buffer *Buffer, mesh *Mesh, texture *Texture, real32 AngleX, real32 AngleY, real32 AngleZ, bool32 ToFillTriangle, uint32 Color)
{
    vec3 CameraPos = {0.0f, 0.0f, -10.0f};
    light Light;
    Light.Direction = (vec3){3.0f, -5.0f, 0.0f};
    
    real32 LightMagnitude = GetMagnitudeVec3(Light.Direction);
    Light.NormalizedDirection = NormalizeVec3(Light.Direction);
    
    real32 HalfWidth = Buffer->Width / 2.0f;
    real32 HalfHeight = Buffer->Height / 2.0f;
    
    for(uint32 VertexIndex = 0; VertexIndex < Mesh->VertexCount; ++VertexIndex)
    {
        vec3 RotatedX = RotateAlongX(Mesh->Vertices[VertexIndex], AngleX);
        vec3 RotatedY = RotateAlongY(RotatedX, AngleY);
        vec3 RotatedZ = RotateAlongZ(RotatedY, AngleZ);
        
        Mesh->Vertices[VertexIndex] = RotatedZ;
    }
    
    for(uint32 TriangleIndex = 0; TriangleIndex < Mesh->TriangleCount; ++TriangleIndex)
    {
        triangle *Triangle = &Mesh->Triangles[TriangleIndex];
        
        vec3 Vertices[3];
        Vertices[0] = Mesh->Vertices[Triangle->A - 1];
        Vertices[1] = Mesh->Vertices[Triangle->B - 1];
        Vertices[2] = Mesh->Vertices[Triangle->C - 1];
        
        Triangle->AverageZ = (Vertices[0].Z + Vertices[1].Z + Vertices[2].Z) / 3.0f;
    }
    
    QuickSort(Arena, Mesh->Triangles, Mesh->TriangleCount);
    
    real32 AngleOfView = (PI / 3.0f); //In radians
    real32 InvAspectRatio = 9.0f / 16.0f;
    real32 NearZ = 5.0f;
    real32 FarZ = 500.0f;
    
    mat4 PerspectiveMatrix = CreatePerspectiveMatrix(AngleOfView, InvAspectRatio, NearZ, FarZ);
    
    for(uint32 TriangleIndex = 0; TriangleIndex < Mesh->TriangleCount; ++TriangleIndex)
    {
        triangle Triangle = Mesh->Triangles[TriangleIndex];
        
        vec3 Vertices[3];
        Vertices[0] = Mesh->Vertices[Triangle.A - 1];
        Vertices[1] = Mesh->Vertices[Triangle.B - 1];
        Vertices[2] = Mesh->Vertices[Triangle.C - 1];
        
        vec2 TextureCoords[3];
        TextureCoords[0] = Mesh->TextureCoords[Triangle.T1 - 1];
        TextureCoords[1] = Mesh->TextureCoords[Triangle.T2 - 1];
        TextureCoords[2] = Mesh->TextureCoords[Triangle.T3 - 1];
        
        vec3 Side1 = SubtractVec3(Vertices[1], Vertices[0]);
        vec3 Side2 = SubtractVec3(Vertices[2], Vertices[0]);
        
        vec3 Normal = CrossVec3(Side1, Side2);
        vec3 NormalizedNormal = NormalizeVec3(Normal);
        vec3 CameraRay = SubtractVec3(CameraPos, Vertices[0]);
        
        real32 CullValue = DotVec3(NormalizedNormal, CameraRay);
        uint32 NewColor = GetFlatShadingColor(NormalizedNormal, Light, Color);
        
        vec5 TextureVertices[3];
        vec2 RasterVertices[3];
        
        for(uint32 VertexIndex = 0; VertexIndex < 3; ++VertexIndex)
        {
            vec3 RotatedVector = Vertices[VertexIndex];
            RotatedVector.Z -= CameraPos.Z;
            
            vec4 ProjectedVector = MultiplyMat4Vec3(PerspectiveMatrix, RotatedVector);
            
            vec2 NDCVertices = (vec2){ProjectedVector.X / ProjectedVector.W, ProjectedVector.Y / ProjectedVector.W};
            
            real32 ScaleX = 25.0f;
            real32 ScaleY = 25.0f;
            
            real32 RasterX = (ScaleX * NDCVertices.X + 1) * HalfWidth;
            real32 RasterY = (ScaleY * -NDCVertices.Y + 1) * HalfHeight;
            TextureVertices[VertexIndex] = (vec5){RasterX, RasterY, RotatedVector.Z, TextureCoords[VertexIndex].X, TextureCoords[VertexIndex].Y};
            RasterVertices[VertexIndex] = (vec2){RasterX, RasterY};
        }
        
        if(CullValue > 0.0f)
        {
            if(ToFillTriangle)
            {
                //char OutputBuffer[256];
                //sprintf_s(OutputBuffer, ArrayCount(OutputBuffer), "{X:%f,Y:%f}, {X:%f,Y:%f}, {X:%f,Y:%f}\n", RasterVertices[0].X, RasterVertices[0].Y, RasterVertices[1].X, RasterVertices[1].Y, RasterVertices[2].X, RasterVertices[2].Y);
                //OutputDebugStringA(OutputBuffer);
                
                TextureMap(Buffer, Texture, TextureVertices[0], TextureVertices[1], TextureVertices[2]);
                //FillTriangle(Buffer, RasterVertices[0], RasterVertices[1], RasterVertices[2], NewColor);
                
                //DrawTriangle(Buffer, RasterVertices[0], RasterVertices[1], RasterVertices[2], 0xFFFFFFFF);
            }
            else
            {
                //DrawTriangle(Buffer, RasterVertices[0], RasterVertices[1], RasterVertices[2], 0x22222222);
            }
        }
    }
    
    //char OutputBuffer[256];
    //sprintf_s(OutputBuffer, ArrayCount(OutputBuffer), "%f\n", Mesh->Vertices[0].Y);
    //OutputDebugStringA("SET\n");
    
}

static LARGE_INTEGER Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    
    return Result;
}

static real32 Win32GetSecondsElapsed(LARGE_INTEGER StartCounter, LARGE_INTEGER EndCounter)
{
    real32 Result = (real32)(EndCounter.QuadPart - StartCounter.QuadPart) / (real32)GlobalPerfFrequency.QuadPart;
    
    return Result;
}

int WINAPI WinMain(HINSTANCE Instance, 
                   HINSTANCE PrevInstance, 
                   PSTR CommandLine, 
                   int ShowCode)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    
    //Set schedular granularity
    bool32 SleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);
    
    uint32 WindowHz = 30;
    real32 TargetSecondsPerFrame = 1.0f / (real32)WindowHz;
    
    WNDCLASSA WindowClass = {0};
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = &Win32WindowProcedure;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "Win32 Renderer Class";
    
    if(RegisterClassA(&WindowClass))
    {
        HWND WindowHandle = CreateWindowExA(
                                            0,
                                            WindowClass.lpszClassName,
                                            "Renderer",
                                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            0,
                                            0,
                                            Instance,
                                            0);
        
        if(WindowHandle)
        {
            QueryPerformanceFrequency(&GlobalPerfFrequency);
            
            InitializeBitmapInfo(&GlobalPixelBuffer, 2560, 1440);
            GlobalPixelBuffer.tPerFrame = TargetSecondsPerFrame;
            
            memory_arena Arena;
            Arena.Size = MEGABYTE(64);
            Arena.Base = VirtualAlloc((void *)MEGABYTE(64), MEGABYTE(64), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            Arena.Used = 0;
            
            bitmap_header BitmapHeader = {0};
            BitmapHeader.FileType = 0x4D42;
            BitmapHeader.FileSize = 4096070;
            BitmapHeader.Reserved1 = 0;
            BitmapHeader.Reserved2 = 0;
            BitmapHeader.BitmapOffset = 70;
            BitmapHeader.Size = 56;
            BitmapHeader.Width = 2560;
            BitmapHeader.Height = 1600;
            BitmapHeader.Planes = 1;
            BitmapHeader.BitsPerPixel = 32;
            BitmapHeader.Compression = 3;
            BitmapHeader.SizeOfBitmap = (BitmapHeader.Width * BitmapHeader.Height * 4);
            BitmapHeader.HorzResolution = 2834;
            BitmapHeader.VertResolution = 2834;
            BitmapHeader.ColorsUsed = 0;
            BitmapHeader.ColorsImportant = 0;
            BitmapHeader.RedMask = 0x00FF0000;
            BitmapHeader.GreenMask = 0x0000FF00;
            BitmapHeader.BlueMask = 0x000000FF;
            BitmapHeader.AlphaMask = 0xFF000000;
            
            uint32 *BitmapBits = (uint32 *)VirtualAlloc(0, BitmapHeader.SizeOfBitmap, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            
#define VERTEX_COUNT 8
            vec3 Vertices[VERTEX_COUNT] = 
            {
                {-1.0f, -1.0f, -1.0f},
                {-1.0f, 1.0f, -1.0f},
                {1.0f, 1.0f, -1.0f},
                {1.0f, -1.0f, -1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, -1.0f, 1.0f},
                {-1.0f, 1.0f, 1.0f},
                {-1.0f, -1.0f, 1.0f}
            };
            
#define TRIANGLE_COUNT 12
            triangle Triangles[TRIANGLE_COUNT] =
            {
                {1,2,3, 0XFFFF0000},
                {1,3,4, 0XFFFF0000},
                
                {6,5,7, 0XFFFF00FF},
                {6,7,8, 0XFFFF00FF},
                
                {8,7,2, 0XFF00FF00},
                {8,2,1, 0XFF00FF00},
                
                {4,3,5, 0XFFFFFF00},
                {4,5,6, 0XFFFFFF00},
                
                {2,7,5, 0XFF0000FF},
                {2,5,3, 0XFF0000FF},
                
                {6,8,1, 0XFF00FFFF},
                {6,1,4, 0XFF00FFFF}
            };
            
            mesh Mesh;
            Mesh.VertexCount = VERTEX_COUNT;
            Mesh.Vertices = (vec3 *)Vertices;
            Mesh.TriangleCount = TRIANGLE_COUNT;
            Mesh.Triangles = (triangle *)Triangles;
            
            mesh NewMesh = {0};
            char *FileName = "./data/scaled_down_bunny.obj";
            ReadObjectFile(FileName, &NewMesh);
            
            file TextureFile;
            ReadBitmap(&TextureFile, "./data/bunny_atlas.bmp");
            
            bitmap_header *TextureHeader = (bitmap_header *)TextureFile.Contents;
            
            
            uint32 *TextureBytes = (uint32 *)((uint8 *)TextureFile.Contents + TextureHeader->BitmapOffset);
            
            texture Texture;
            Texture.Width = TextureHeader->Width;
            Texture.Height = TextureHeader->Height;
            Texture.Bytes = TextureBytes;
            Texture.BytesPerTexel = (TextureHeader->BitsPerPixel / 8);
            
            real32 AngleX = 0.0f;
            real32 AngleY = 0.0f;
            real32 AngleZ = 0.0f;
            
            vec2 PointA = {.X=100.0f,.Y=200.0f};
            vec2 PointB = {.X=150.345f,.Y=800.222f};
            vec2 PointC = {.X=25.1121f,.Y=800.332f};
            
            vec2 PointD = {.X=1875.917480f,.Y=731.790039f};
            vec2 PointE = {.X=1530.850586f,.Y=470.931793f};
            vec2 PointF = {.X=1093.654663f,.Y=209.333496f};
            
            vec5 TA = {.X=1861.56506f, .Y=800.012756f, .Z=8.89288139f, .U=0.375f, .V=0.5f};
            vec5 TB = {.X=1136.92053f, .Y=171.319427f, .Z=9.44466972f, .U=0.125f, .V=0.75f};
            vec5 TC = {.X=1396.96484f, .Y=487.072693f, .Z=5.8458271f, .U=0.125f, .V=0.5f};
            
            vec5 T1 = {.X=1711.06628f, .Y=957.995972f, .Z=10.784934f, .U=0.375f, .V=0.5f};
            vec5 T2 = {.X=1407.41943f, .Y=135.297882f, .Z=7.9643631f, .U=0.125f, .V=0.75f};
            vec5 T3 = {.X=1682.51892f, .Y=880.986084f, .Z=6.25552893f, .U=0.125f, .V=0.5f};
            
            
            
            uint32 Color = 0xC8A2C8;
            bool32 FillTriangles = true;
            
            DrawMesh(&Arena, &GlobalPixelBuffer, &NewMesh, &Texture, AngleX, AngleY, AngleZ, FillTriangles, Color);
            
            //FillFlatBottomTriangle(&GlobalPixelBuffer, PointA, PointB,  PointC, Color);
            
            //FillTriangle(&GlobalPixelBuffer, PointA, PointB, PointC, Color);
            //DrawTriangle(&GlobalPixelBuffer, PointA, PointB, PointC, 0xFFFF0000);
            //DrawTriangle(&GlobalPixelBuffer, PointD, PointE, PointF, 0xFFFF0000);
            //TextureMap(&GlobalPixelBuffer, T1, T2, T3);
            
            
            //SaveBitmap(&GlobalPixelBuffer, &BitmapHeader, BitmapBits, "line.bmp");
            
            LARGE_INTEGER FlipWallClock = Win32GetWallClock();
            GlobalRunning = true;
            
            while(GlobalRunning)
            {
                MSG Message;
                while(PeekMessageA(&Message,0,0,0,PM_REMOVE))
                {
                    switch(Message.message)
                    {
                        case WM_QUIT:
                        {
                            GlobalRunning = false;
                        }break;
                        default:
                        {
                            TranslateMessage(&Message);
                            DispatchMessage(&Message);
                        }break;
                    }
                }
                
                //DrawRect(&GlobalPixelBuffer, (vec2){0, 0}, (vec2){(real32)GlobalPixelBuffer.Width, (real32)GlobalPixelBuffer.Height}, 0x00000000);
                DrawPixel(&GlobalPixelBuffer, (vec2){0, 0}, 0xFFFF0000);
                
                // NOTE(not-set): This functions is frame dependent, might want to change it to frame independent later!
                
                //DrawMesh(&Arena, &GlobalPixelBuffer, &NewMesh, &Texture, AngleX, AngleY, AngleZ, FillTriangles, Color);
                
                //DrawMesh(&Arena, &GlobalPixelBuffer, &Mesh, &Texture, AngleX, AngleY, AngleZ, FillTriangles, Color);
                
                AngleX = 0.01f;
                AngleY = 0.01f;
                AngleZ = 0.01f;
                
                uint32 GridWidth = 10;
                uint32 GridHeight = 10;
                //DrawGrid(&GlobalPixelBuffer, GridWidth, GridHeight, 0x77333333);
                
                
                
                LARGE_INTEGER WorkWallClock = Win32GetWallClock();
                real32 WorkSeconds = Win32GetSecondsElapsed(FlipWallClock, WorkWallClock);
                //Sleep
                if(WorkSeconds < TargetSecondsPerFrame)
                {
                    if(SleepIsGranular)
                    {
                        uint32 SleepMS = (uint32)(1000.0f * (TargetSecondsPerFrame - WorkSeconds)) - 1;
                        Sleep(SleepMS);
                    }
                    
                    WorkWallClock = Win32GetWallClock();
                    WorkSeconds = Win32GetSecondsElapsed(FlipWallClock, WorkWallClock);
                    
                    /*char OutputBuffer[256];
                    sprintf_s(OutputBuffer, ArrayCount(OutputBuffer), "B: %.2fms, ", (1000.0f * WorkSeconds));
                    OutputDebugStringA(OutputBuffer);*/
                    
                    while(WorkSeconds < TargetSecondsPerFrame)
                    {
                        WorkWallClock = Win32GetWallClock();
                        WorkSeconds = Win32GetSecondsElapsed(FlipWallClock, WorkWallClock);
                    }
                    
                    /*sprintf_s(OutputBuffer, ArrayCount(OutputBuffer), "A: %.2fms\n", (1000.0f * WorkSeconds));
                    OutputDebugStringA(OutputBuffer);*/
                }
                
                //Flip
                RECT WindowDimensions;
                GetClientRect(WindowHandle, &WindowDimensions);
                
                uint32 WindowWidth = WindowDimensions.right - WindowDimensions.left;
                uint32 WindowHeight = WindowDimensions.bottom - WindowDimensions.top;
                
                HDC DeviceContext = GetDC(WindowHandle);
                Win32UpdateWindow(&GlobalPixelBuffer, DeviceContext, WindowWidth, WindowHeight);
                ReleaseDC(WindowHandle, DeviceContext);
                
                //After Flip
                FlipWallClock = Win32GetWallClock();
                
                
            }
        }
    }
    
    
    return 0;
}