void ReadObjectFile(char *FileName, mesh *Mesh)
{
    FILE *FilePointer = fopen(FileName, "r");
    
    if(FilePointer)
    {
        while(!feof(FilePointer))
        {
            unsigned char First = (unsigned char)getc(FilePointer);
            unsigned char Second = (unsigned char)getc(FilePointer);
            
            if(First == 'v' && Second == ' ')
            {
                ++Mesh->VertexCount;
            }
            else if(First == 'v' && Second == 't')
            {
                ++Mesh->TextureCount;
            }
            else if(First == 'f' && Second == ' ')
            {
                ++Mesh->TriangleCount;
            }
            else
            {
                fscanf_s(FilePointer, "%*[^\n]");
                getc(FilePointer);
            }
        }
        
        int32 FSeekReturnValue = fseek(FilePointer, 0L, SEEK_SET);
        
        if(!FSeekReturnValue)
        {
            Mesh->Vertices = (vec3 *)VirtualAlloc(0, sizeof(vec5) * Mesh->VertexCount, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            Mesh->Triangles = (triangle *)VirtualAlloc(0, sizeof(triangle) * Mesh->TriangleCount, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            Mesh->TextureCoords = (vec2 *)VirtualAlloc(0, sizeof(vec2) * Mesh->TextureCount, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            
            uint32 VertexIndex = 0;
            uint32 TriangleIndex = 0;
            uint32 TextureIndex = 0;
            
            while(!feof(FilePointer))
            {
                unsigned char First = (unsigned char)getc(FilePointer);
                unsigned char Second = (unsigned char)getc(FilePointer);
                
                if(First == 'v' && Second == ' ')
                {
                    vec3 *Vertex = &Mesh->Vertices[VertexIndex++];
                    fscanf_s(FilePointer, "%f%f%f", &Vertex->X, &Vertex->Y, &Vertex->Z);
                    getc(FilePointer);
                }
                else if(First == 'v' && Second == 't')
                {
                    vec2 *Texture = &Mesh->TextureCoords[TextureIndex++];
                    fscanf_s(FilePointer, "%f%f", &Texture->X, &Texture->Y);
                    getc(FilePointer);
                }
                else if(First == 'f' && Second == ' ')
                {
                    triangle *Triangle = &Mesh->Triangles[TriangleIndex++];
                    fscanf_s(FilePointer, "%d/%d/%*d%d/%d/%*d%d/%d/%*d", &Triangle->A, &Triangle->T1, &Triangle->B, &Triangle->T2, &Triangle->C, &Triangle->T3);
                    //fscanf_s(FilePointer, "%d/%d%d/%d%d/%d", &Triangle->A, &Triangle->T1, &Triangle->B, &Triangle->T2, &Triangle->C, &Triangle->T3);
                    //fscanf_s(FilePointer, "%d%d%d", &Triangle->A, &Triangle->B, &Triangle->C);
                    getc(FilePointer);
                }
                else
                {
                    fscanf_s(FilePointer, "%*[^\n]");
                    getc(FilePointer);
                }
            }
            
            fclose(FilePointer);
        }
    }
}