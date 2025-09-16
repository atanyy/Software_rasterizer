static void DrawLineBresenham(win32_pixel_buffer *Buffer, vec2 VectorA, vec2 VectorB, uint32 Color)
{
    //real32 Slope = (real32)(DestY - SourceY) / (real32)(DestX - SourceX);
    
    vec2 SourceCoord = Vec2CoordToScreenCoord(Buffer, VectorA);
    vec2 DestCoord = Vec2CoordToScreenCoord(Buffer, VectorB);
    
    uint32 SourceX = RoundReal32ToUInt32(SourceCoord.X);
    uint32 SourceY = RoundReal32ToUInt32(SourceCoord.Y);
    uint32 DestX = RoundReal32ToUInt32(DestCoord.X);
    uint32 DestY = RoundReal32ToUInt32(DestCoord.Y);
    
    if(SourceY == DestY)
    {
        if(SourceX > DestX)
        {
            SwapUInt32(&SourceX, &DestX);
        }
        
        for(uint32 X = SourceX; X <= DestX; ++X)
        {
            uint32 *Pixel = (uint32 *)((uint8 *)Buffer->Memory + (SourceY * Buffer->Stride) + (X * Buffer->BytesPerPixel));
            *Pixel = Color;
        }
        
        return;
    }
    
    uint32 deltaY = abs(DestY - SourceY);
    uint32 deltaX = abs(DestX - SourceX);
    
    bool32 IsSteep = false;
    if(deltaY > deltaX)
    {
        IsSteep = true;
        SwapUInt32(&deltaX, &deltaY);
        SwapUInt32(&SourceX, &SourceY);
        SwapUInt32(&DestX, &DestY);
    }
    
    if(DestX < SourceX)
    {
        SwapUInt32(&DestX, &SourceX);
        SwapUInt32(&DestY, &SourceY);
    }
    
    
    int32 Error = 0;
    //real32 RealY = (real32)SourceY;
    uint32 PixelY = SourceY;
    
    for(uint32 PixelX = SourceX; PixelX <= (uint32)DestX; ++PixelX)
    {
        uint32 *Pixel;
        if(IsSteep)
        {
            Pixel = (uint32 *)((uint8 *)Buffer->Memory + (PixelX * Buffer->Stride) + (PixelY * Buffer->BytesPerPixel));
        }
        else
        {
            Pixel = (uint32 *)((uint8 *)Buffer->Memory + (PixelY * Buffer->Stride) + (PixelX * Buffer->BytesPerPixel));
        }
        
        *Pixel = Color;
        
        //char OutputBuffer[256];
        //sprintf_s(OutputBuffer, 256, "X:%d, Y:%d\n", PixelX, PixelY);
        //OutputDebugStringA(OutputBuffer);
        
        //RealY += Slope;
        //Error = RealY - PixelY;
        
        Error += (2 * deltaY);
        
        if(Error > (int32)deltaX)
        {
            PixelY += (DestY < SourceY) ? -1 : 1;
            Error -= (2 * deltaX);
        }
    }
}

static void
DrawLineMidPoint(win32_pixel_buffer *Buffer, vec2 VectorA, vec2 VectorB,
                 uint32 Color)
{
    vec2 SourceCoord = Vec2CoordToScreenCoord(Buffer, VectorA);
    vec2 DestCoord = Vec2CoordToScreenCoord(Buffer, VectorB);
    
    
    uint32 SourceX = RoundReal32ToUInt32(SourceCoord.X);
    uint32 SourceY = RoundReal32ToUInt32(SourceCoord.Y);
    uint32 DestX = RoundReal32ToUInt32(DestCoord.X);
    uint32 DestY = RoundReal32ToUInt32(DestCoord.Y);
    
    //Assuming deltaY < deltaX => slope between 0 and 1
    
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
    
    // d = a + (b/2)
    //real32 d = (deltaY - (deltaX / 2));
    
    // d = 2a + b
    int32 d = (2 * deltaY) - deltaX;
    uint32 Y = SourceY;
    
    
    for(uint32 X = SourceX; X <= DestX; ++X)
    {
        uint32 *Pixel;
        
        if(IsSteep)
        {
            Pixel = (uint32 *)((uint8 *)Buffer->Memory + (X * Buffer->Stride) + (Y * Buffer->BytesPerPixel));
        }
        else
        {
            Pixel = (uint32 *)((uint8 *)Buffer->Memory + (Y * Buffer->Stride) + (X * Buffer->BytesPerPixel));
        }
        
        *Pixel = Color;
        
        if(d > 0)
        {
            //d += a - b
            //d += (deltaY - (-deltaX));
            
            //d += 2a - 2b
            d += (2 * (deltaY - deltaX));
            Y += (SourceY > DestY) ? -1 : 1;
        }
        else
        {
            //d += a
            //d += deltaY;
            
            //d += 2a
            d += (2 * deltaY);
        }
    }
}