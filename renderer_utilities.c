inline void 
SwapUInt32(uint32 *A, uint32 *B)
{
    int32 Temp = *A;
    *A = *B;
    *B = Temp;
}

inline void 
SwapInt32(int32 *A, int32 *B)
{
    int32 Temp = *A;
    *A = *B;
    *B = Temp;
}

inline void
SwapReal32(real32 *A, real32 *B)
{
    real32 Temp = *A;
    *A = *B;
    *B = Temp;
}

inline uint32 RoundReal32ToUInt32(real32 Value)
{
    uint32 Result = (uint32)(Value + 0.5f);
    
    return Result;
}

inline int32 RoundReal32ToInt32(real32 Value)
{
    int32 Result = (int32)roundf(Value);
    
    return Result;
}