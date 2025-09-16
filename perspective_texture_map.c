#include "perspective_texture_map.h"

void CreateTexture(uint32 *Texture)
{
    uint32 Color1 = 0XFFFF0000;
    uint32 Color2 = 0XFF0000FF;
    
    for(uint32 Y = 0; Y < TEXTURE_HEIGHT; ++Y)
    {
        for(uint32 X = 0; X < TEXTURE_WIDTH; ++X)
        {
            uint32 TextureColor = (X % 2 == 0) && (Y % 2 == 0) ? Color1 : Color2;
            
            Texture[(Y * TEXTURE_WIDTH) + X] = TextureColor;
        }
    }
}

void SwapVec5(vec5 *V1, vec5 *V2)
{
    vec5 Temp = *V1;
    *V1 = *V2;
    *V2 = Temp;
}

void SortVerticesVec5(vec5 *Vertices)
{
    if(Vertices[1].Y <= Vertices[0].Y && Vertices[1].Y <= Vertices[2].Y)
    {
        if(Vertices[0].Y <= Vertices[2].Y)
        {
            SwapVec5(&Vertices[1], &Vertices[0]);
        }
        else
        {
            SwapVec5(&Vertices[1], &Vertices[0]);
            SwapVec5(&Vertices[1], &Vertices[2]);
        }
    }
    else if(Vertices[2].Y <= Vertices[0].Y && Vertices[2].Y <= Vertices[1].Y)
    {
        if(Vertices[1].Y <= Vertices[0].Y)
        {
            SwapVec5(&Vertices[2], &Vertices[0]);
        }
        else
        {
            SwapVec5(&Vertices[2], &Vertices[0]);
            SwapVec5(&Vertices[2], &Vertices[1]);
        }
    }
    else
    {
        if(Vertices[2].Y <= Vertices[1].Y)
        {
            SwapVec5(&Vertices[2], &Vertices[1]);
        }
        else
        {
            //Do Nothing
        }
    }
}

void CalculateGradients(gradient *Gradients, vec5 *Vertices)
{
    for(uint32 Index = 0; Index < ArrayCount(Gradients->OneOverZ); ++Index)
    {
        Gradients->OneOverZ[Index] = 1.0f / Vertices[Index].Z;
        Gradients->UOverZ[Index] = Vertices[Index].U / Vertices[Index].Z;
        Gradients->VOverZ[Index] = Vertices[Index].V / Vertices[Index].Z;
    }
    
    real32 OneOverdX = 1.0f / (((Vertices[0].Y - Vertices[2].Y) * (Vertices[1].X - Vertices[2].X)) - ((Vertices[0].X - Vertices[2].X) * (Vertices[1].Y - Vertices[2].Y)));
    
    real32 OneOverdY = -OneOverdX;
    
    Gradients->dOneOverZdX = OneOverdX * (((Gradients->OneOverZ[1] - Gradients->OneOverZ[2]) *
                                           (Vertices[0].Y - Vertices[2].Y)) -
                                          ((Gradients->OneOverZ[0] - Gradients->OneOverZ[2]) *
                                           (Vertices[1].Y - Vertices[2].Y)));
    
    Gradients->dOneOverZdY = OneOverdY * (((Gradients->OneOverZ[1] - Gradients->OneOverZ[2]) *
                                           (Vertices[0].X - Vertices[2].X)) -
                                          ((Gradients->OneOverZ[0] - Gradients->OneOverZ[2]) *
                                           (Vertices[1].X - Vertices[2].X)));
    
    Gradients->dUOverZdX = OneOverdX * (((Gradients->UOverZ[1] - Gradients->UOverZ[2]) *
                                         (Vertices[0].Y - Vertices[2].Y)) -
                                        ((Gradients->UOverZ[0] - Gradients->UOverZ[2]) *
                                         (Vertices[1].Y - Vertices[2].Y)));
    
    Gradients->dUOverZdY = OneOverdY * (((Gradients->UOverZ[1] - Gradients->UOverZ[2]) *
                                         (Vertices[0].X - Vertices[2].X)) -
                                        ((Gradients->UOverZ[0] - Gradients->UOverZ[2]) *
                                         (Vertices[1].X - Vertices[2].X)));
    
    Gradients->dVOverZdX = OneOverdX * (((Gradients->VOverZ[1] - Gradients->VOverZ[2]) *
                                         (Vertices[0].Y - Vertices[2].Y)) -
                                        ((Gradients->VOverZ[0] - Gradients->VOverZ[2]) *
                                         (Vertices[1].Y - Vertices[2].Y)));
    
    Gradients->dVOverZdY = OneOverdY * (((Gradients->VOverZ[1] - Gradients->VOverZ[2]) *
                                         (Vertices[0].X - Vertices[2].X)) -
                                        ((Gradients->VOverZ[0] - Gradients->VOverZ[2]) *
                                         (Vertices[1].X - Vertices[2].X)));
}

void InitializeEdge(edge *Edge, gradient Gradients, vec5 V1, uint32 V1Index, vec5 V2, uint32 V2index)
{
    Edge->V1 = V1;
    Edge->V2 = V2;
    
    Edge->Y = (uint32)ceilf(Edge->V1.Y);
    Edge->IntY1 = Edge->Y;
    Edge->IntY2 = (uint32)ceilf(Edge->V2.Y);
    
    Edge->Height = Edge->IntY2 - Edge->IntY1;
    
    real32 YPreStep = Edge->IntY1 - Edge->V1.Y;
    
    real32 DeltaX = Edge->V2.X - Edge->V1.X;
    real32 DeltaY = Edge->V2.Y - Edge->V1.Y;
    
    Edge->X = (YPreStep * (DeltaX) / DeltaY) + Edge->V1.X;
    
    Edge->XStep = DeltaX / DeltaY;
    
    real32 XPreStep = Edge->X - Edge->V1.X;
    
    Edge->UOverZ = Gradients.UOverZ[V1Index] + (YPreStep * Gradients.dUOverZdY) + (XPreStep * Gradients.dUOverZdX);
    Edge->UOverZStep = (Edge->XStep * Gradients.dUOverZdX) + Gradients.dUOverZdY;
    
    Edge->VOverZ = Gradients.VOverZ[V1Index] + (YPreStep * Gradients.dVOverZdY) + (XPreStep * Gradients.dVOverZdX);
    Edge->VOverZStep = (Edge->XStep * Gradients.dVOverZdX) + Gradients.dVOverZdY;
    
    Edge->OneOverZ = Gradients.OneOverZ[V1Index] + (YPreStep * Gradients.dOneOverZdY) + (XPreStep * Gradients.dOneOverZdX);
    Edge->OneOverZStep = (Edge->XStep * Gradients.dOneOverZdX) + Gradients.dOneOverZdY;
}

void DrawHorizontalScanline(win32_pixel_buffer *Buffer, texture *Texture, edge *Left, edge *Right, gradient Gradients)
{
    uint32 XStart = (uint32)ceilf(Left->X);
    uint32 XEnd = (uint32)ceilf(Right->X);
    
    real32 XPreStep = XStart - Left->X;
    
    real32 OneOverZ = Left->OneOverZ + (XPreStep * Gradients.dOneOverZdX);
    
    real32 UOverZ = Left->UOverZ + (XPreStep * Gradients.dUOverZdX);
    
    real32 VOverZ = Left->VOverZ + (XPreStep * Gradients.dVOverZdX);
    
    uint32 *Pixel = (uint32 *)((uint8 *)Buffer->Memory + (Left->Y * Buffer->Stride) + (XStart * Buffer->BytesPerPixel));
    
    uint32 TextureWidth = Texture->Width;
    uint32 TextureHeight = Texture->Height;
    
    //Assert(Gradients.dOneOverZdX >= 0);
    //Assert(OneOverZ >= 0);
    //Assert(UOverZ < OneOverZ);
    for(uint32 X = XStart; X <= XEnd; ++X)
    {
        uint32 U = (uint32)((UOverZ / OneOverZ) * TextureWidth);
        uint32 V = (uint32)((VOverZ / OneOverZ) * TextureHeight);
        
        // NOTE(not-set): This operation might be hiding an error
        U = U % TextureWidth;
        V = V % TextureHeight;
        
        //24-bit to 32-bit conversion of bmp
        uint8 *TexByte = (uint8 *)Texture->Bytes + (V * TextureWidth * Texture->BytesPerTexel) + (U * Texture->BytesPerTexel);
        
        //uint32 Alpha = *TexByte++;
        uint32 Blue = *TexByte++;
        uint32 Green = *TexByte++;
        uint32 Red = *TexByte;
        uint32 Color = (0xFF << 24) | (Red << 16) | (Green << 8) | (Blue << 0);
        *Pixel++ = Color;
        
        OneOverZ += Gradients.dOneOverZdX;
        UOverZ += Gradients.dUOverZdX;
        VOverZ += Gradients.dVOverZdX;
    }
}

void Step(edge *Edge)
{
    Edge->Y += 1;
    Edge->X += Edge->XStep;
    
    Edge->UOverZ += Edge->UOverZStep;
    Edge->VOverZ += Edge->VOverZStep;
    Edge->OneOverZ += Edge->OneOverZStep;
    
    Edge->Height -= 1;
}

void TextureMap(win32_pixel_buffer *Buffer, texture *Texture, vec5 V1, vec5 V2, vec5 V3)
{
    //CreateTexture((uint32 *)TextureBytes);
    
    vec5 SortedVertices[3] = {V1, V2, V3};
    SortVerticesVec5(SortedVertices);
    
    gradient Gradients;
    CalculateGradients(&Gradients, SortedVertices);
    
    edge TopToBottom;
    edge TopToMiddle;
    edge MiddleToBottom;
    
    InitializeEdge(&TopToBottom, Gradients, SortedVertices[0], 0, SortedVertices[2], 2);
    InitializeEdge(&TopToMiddle, Gradients, SortedVertices[0], 0, SortedVertices[1], 1);
    InitializeEdge(&MiddleToBottom, Gradients, SortedVertices[1], 1, SortedVertices[2], 2);
    
    edge *Left;
    edge *Right;
    bool32 MiddleIsLeft = false;
    
    /*
Finding if middle is left or right is more complex:
- First compare the middle and bottom vertex X
- Depending on whether middle is to the left or right of bottom, check if top is to the left or right,
- Depending on whether top is to the left or right of middle, check whether the TopToMiddle line is steeper than the MiddleToBottom line.
- This information will accurately tell you whether the middle vertex points to the left or the right
*/
    
    real32 SlopeTopMiddle = fabsf((SortedVertices[1].Y - SortedVertices[0].Y) / (SortedVertices[1].X - SortedVertices[0].X));
    real32 SlopeMiddleBottom = fabsf((SortedVertices[2].Y - SortedVertices[1].Y) / (SortedVertices[2].X - SortedVertices[1].X));
    
    if(SortedVertices[1].X < SortedVertices[2].X)
    {
        if(SortedVertices[0].X < SortedVertices[1].X)
        {
            if(SlopeTopMiddle > SlopeMiddleBottom)
            {
                MiddleIsLeft = true;
                Left = &TopToMiddle;
                Right = &TopToBottom;
            }
            else
            {
                Left = &TopToBottom;
                Right = &TopToMiddle;
            }
        }
        else
        {
            MiddleIsLeft = true;
            Left = &TopToMiddle;
            Right = &TopToBottom;
        }
        
    }
    else
    {
        if(SortedVertices[0].X > SortedVertices[1].X)
        {
            if(SlopeTopMiddle > SlopeMiddleBottom)
            {
                Left = &TopToBottom;
                Right = &TopToMiddle;
            }
            else
            {
                MiddleIsLeft = true;
                Left = &TopToMiddle;
                Right = &TopToBottom;
            }
        }
        else
        {
            Left = &TopToBottom;
            Right = &TopToMiddle;
        }
    }
    
    uint32 Height = (uint32)(ceilf(TopToMiddle.V2.Y) - ceilf(TopToMiddle.V1.Y));
    
    while(Height--)
    {
        DrawHorizontalScanline(Buffer, Texture, Left, Right, Gradients);
        Step(&TopToBottom);
        Step(&TopToMiddle);
    }
    
    if(MiddleIsLeft)
    {
        Left = &MiddleToBottom;
        Right = &TopToBottom;
    }
    else
    {
        Left = &TopToBottom;
        Right = &MiddleToBottom;
    }
    
    Height = (uint32)(ceilf(MiddleToBottom.V2.Y) - ceilf(MiddleToBottom.V1.Y));
    
    while(Height--)
    {
        DrawHorizontalScanline(Buffer, Texture, Left, Right, Gradients);
        Step(&TopToBottom);
        Step(&MiddleToBottom);
    }
}





