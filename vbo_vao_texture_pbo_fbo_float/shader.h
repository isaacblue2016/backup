#ifndef __SHADER_H__
#define __SHADER_H__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <png.h>
#include <GL/glew.h>
#include <GL/glut.h>

#define MYLOG(x, y...) printf("[%s][%s][%04d]: "x"\n", __FILE__, __FUNCTION__, __LINE__, ##y)
#define printOpenGLError() printOglError(__FILE__, __LINE__)

typedef struct
{
    GLubyte    * imageData;
    GLuint    bpp;
    GLuint    width;
    GLuint    height;
    GLuint    texID;
    GLuint    type;
} Texture;

typedef struct
{
    GLubyte Header[12];
} TGAHeader;


typedef struct
{
    GLubyte        header[6];
    GLuint        bytesPerPixel;
    GLuint        imageSize;
    GLuint        temp;
    GLuint        type;
    GLuint        Height;
    GLuint        Width;
    GLuint        Bpp;
} TGA;

TGAHeader tgaheader;
TGA tga;

Texture gTexture;
GLubyte uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};    // Uncompressed TGA Header
GLubyte cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};    // Compressed TGA Header
bool LoadUncompressedTGA(Texture *, const char *, FILE *);    // Load an Uncompressed file
bool LoadCompressedTGA(Texture *, const char *, FILE *);        // Load a Compressed file
bool LoadTGA(Texture *texture, const char *filename);
GLuint LoadTGA(const char *filename);

#define BMP_Header_Length 54
int readingBmp(const char *file, GLubyte** pixels, GLint &width, GLint &height)
{
    GLint total_bytes;

    FILE* pFile = fopen(file, "rb");
    if(pFile == 0)
    {
        printf("Can not open file %s...", file);
        return -1;
    }

    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    {
        GLint line_bytes = width * 3;
        while(line_bytes % 4 != 0)
        {
            ++line_bytes;
        }
        total_bytes = line_bytes * height;
    }

    *pixels = (GLubyte*)malloc(total_bytes);
    if(*pixels == 0)
    {
        printf("Pixeles in file %s is NULL...", file);
        fclose(pFile);
        return 0;
    }

    if(fread(*pixels, total_bytes, 1, pFile) <= 0)
    {
        free(*pixels);
        fclose(pFile);
        return -1;
    }
    return 0;
}

GLuint loadTextureBMP(const char *file)
{
    GLint width, height, total_bytes;
    GLubyte* pixels = 0;
    GLuint texture_ID = 0;

    FILE* pFile = fopen(file, "rb");
    if(pFile == 0)
    {
        printf("Can not open file %s...", file);
        return 0;
    }

    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    {
        GLint line_bytes = width * 3;
        while(line_bytes % 4 != 0)
        {
            ++line_bytes;
        }
        total_bytes = line_bytes * height;
    }

    pixels = (GLubyte*)malloc(total_bytes);
    if(pixels == 0)
    {
        printf("Pixeles in file %s is NULL...", file);
        fclose(pFile);
        return 0;
    }

    if(fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    glGenTextures(1, &texture_ID);
    if(texture_ID == 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    free(pixels);
    return texture_ID;
}

#define TEXTURE_LOAD_ERROR -1
GLint format = -1;
GLint internalFormat = -1;
GLint getValidLength(int length)
{
    if (length >= 1024)
    {
        return 1024;
    }
    else if (length <= 1)
    {
        return 1;
    }

    GLint valid_length = 1;
    while (valid_length < length)
    {
        valid_length *= 2;
    }

    return valid_length;
}

void getPNGtextureInfo(int color_type)
{
    format = -1;
    internalFormat = -1;
    switch (color_type)
    {
      case PNG_COLOR_TYPE_GRAY:
        format = GL_LUMINANCE;
        internalFormat = 1;
        printf("Format type=[GL_LUMINANCE], internalFormat=[%d], format=[%d]\n", internalFormat, format);
        break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
        format = GL_LUMINANCE_ALPHA;
        internalFormat = 2;
        printf("Format type=[GL_LUMINANCE_ALPHA], internalFormat=[%d], format=[%d]\n", internalFormat, format);
        break;

      case PNG_COLOR_TYPE_RGB:
        format = GL_RGB;
        internalFormat = 3;
        printf("Format type=[GL_RGB], internalFormat=[%d], format=[%d]\n", internalFormat, format);
        break;

      case PNG_COLOR_TYPE_RGB_ALPHA:
        format = GL_RGBA;
        internalFormat = 4;
        printf("Format type=[GL_RGBA], internalFormat=[%d], format=[%d]\n", internalFormat, format);
        break;

      default:
        /* Badness */
        break;
    }
}

GLuint loadTexturePNG(const char *filename)
{
    png_byte magic[8];
    png_structp png_ptr;
    png_infop info_ptr;
    int bit_depth, color_type;
    FILE *fp = NULL;
    GLubyte *images = NULL;
    GLubyte *valid_images = NULL;
    png_bytep *row_pointers = NULL;
    png_uint_32 width, height;

    fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("Can not open file %s...\n", filename);
        return TEXTURE_LOAD_ERROR;
    }

    fread(magic, 1, sizeof (magic), fp);
    if (!png_check_sig(magic, sizeof (magic)))
    {
        printf("Check sig file[%s] failed.\n", filename);
        fclose(fp);
        return 0;
    }

    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        printf("Create read struct [%s] failed.\n", filename);
        fclose (fp);
        return 0;
    }

    info_ptr = png_create_info_struct (png_ptr);
    if (!info_ptr)
    {
        printf("Create info struct [%s] failed.\n", filename);
        fclose (fp);
        png_destroy_read_struct (&png_ptr, NULL, NULL);
        return 0;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("Set jmp [%s] failed.\n", filename);
        fclose (fp);
        png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

        if (row_pointers)
            delete[] row_pointers;

        if (images)
            delete[] images;

        if (valid_images)
            delete[] valid_images;
        return -1;
    }

    png_init_io (png_ptr, fp);
    png_set_sig_bytes (png_ptr, sizeof(magic));
    png_read_info (png_ptr, info_ptr);
    bit_depth = png_get_bit_depth (png_ptr, info_ptr);

    printf("bit_depth=[%d].\n", bit_depth);

    color_type = png_get_color_type (png_ptr, info_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
        printf("Color type=[PNG_COLOR_TYPE_PALETTE].\n");
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    {
        printf("Color type=[PNG_COLOR_TYPE_GRAY].\n");
        png_set_gray_1_2_4_to_8(png_ptr);
    }

    if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);
    else if (bit_depth < 8)
        png_set_packing(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    getPNGtextureInfo(color_type);

    int valid_width = getValidLength(width);
    int valid_height = getValidLength(height);

    float mU = ((float)width/valid_width);
    float mV = ((float)height/valid_height);

    images = new png_byte[sizeof(png_byte) * width * height * internalFormat];
    valid_images = new png_byte[sizeof(png_byte) * valid_width * valid_height * internalFormat];
    row_pointers = new png_bytep[sizeof(png_bytep) * height];

    //images = (png_byte *)malloc(sizeof(png_byte) * width * height * internalFormat);
    //row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);

    const int line_width = width * internalFormat;
    for (int i = 0; i < height; ++i)
    {
        row_pointers[height - 1 - i] = images + i * line_width;
    }

    png_read_image (png_ptr, row_pointers);

    const int valid_line_width = valid_width * internalFormat;
    for (int h = 0; h < valid_height; ++h)
    {
        for (int i = 0; i < valid_line_width; ++i)
        {
            int indexFrom = h * line_width + i;
            int indexTo = h * valid_line_width + i;
            if (h < height && i < line_width)
            {
                valid_images[indexTo] = images[indexFrom];
            }
            else
            {
                 valid_images[indexTo] = 0x00;
            }
        }
    }

    png_read_end (png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    GLuint textureId = 100;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, images);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, valid_width, valid_height, 0, format, GL_UNSIGNED_BYTE, valid_images);

    delete[] images;
    delete[] valid_images;
    delete[] row_pointers;

    return textureId;
}

GLuint LoadTGA(const char *filename)
{
    if (LoadTGA(&gTexture, filename)) {
        return gTexture.texID;
    } else {
        return 0;
    }
}

bool LoadTGA(Texture *texture, const char *filename)                // Load a TGA file
{
    FILE * fTGA;                                                // File pointer to texture file
    fTGA = fopen(filename, "rb");                                // Open file for reading

    if(fTGA == NULL)                                            // If it didn't open....
    {
        MYLOG("Could not open texture file");    // Display an error message
        return false;                                                        // Exit function
    }

    if(fread(&tgaheader, sizeof(TGAHeader), 1, fTGA) == 0)                    // Attempt to read 12 byte header from file
    {
        MYLOG("Could not read file header");        // If it fails, display an error message 
        if(fTGA != NULL)                                                    // Check to seeiffile is still open
        {
            fclose(fTGA);                                                    // If it is, close it
        }
        return false;                                                        // Exit function
    }

    if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)                // See if header matches the predefined header of 
    {                                                                        // an Uncompressed TGA image
        LoadUncompressedTGA(texture, filename, fTGA);                        // If so, jump to Uncompressed TGA loading code
    }
    else if(memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)        // See if header matches the predefined header of
    {                                                                        // an RLE compressed TGA image
        LoadCompressedTGA(texture, filename, fTGA);                            // If so, jump to Compressed TGA loading code
    }
    else                                                                    // If header matches neither type
    {
        MYLOG("TGA file be type 2 or type 10, Invalid Image ");    // Display an error
        fclose(fTGA);
        return false;                                                                // Exit function
    }

    glGenTextures(1, &texture->texID);                // Create The Texture ( CHANGE )
    glBindTexture(GL_TEXTURE_2D, texture->texID);
    glTexImage2D(GL_TEXTURE_2D, 0, texture->bpp / 8, texture->width, texture->height, 0, texture->type, GL_UNSIGNED_BYTE, texture->imageData);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    if (texture->imageData)                        // If Texture Image Exists ( CHANGE )
    {
        free(texture->imageData);                    // Free The Texture Image Memory ( CHANGE )
    }

    return true;                                                            // All went well, continue on
}

bool LoadUncompressedTGA(Texture * texture, const char * filename, FILE * fTGA)    // Load an uncompressed TGA (note, much of this code is based on NeHe's 
{
    MYLOG();                                                                            // TGA Loading code nehe.gamedev.net)
    if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)                    // Read TGA header
    {                                        
        MYLOG("Could not read info header");        // Display error
        if(fTGA != NULL)                                                    // if file is still open
        {
            fclose(fTGA);                                                    // Close it
        }
        return false;                                                        // Return failular
    }    

    texture->width  = tga.header[1] * 256 + tga.header[0];                    // Determine The TGA Width    (highbyte*256+lowbyte)
    texture->height = tga.header[3] * 256 + tga.header[2];                    // Determine The TGA Height    (highbyte*256+lowbyte)
    texture->bpp    = tga.header[4];                                        // Determine the bits per pixel
    tga.Width        = texture->width;                                        // Copy width into local structure                        
    tga.Height        = texture->height;                                        // Copy height into local structure
    tga.Bpp            = texture->bpp;                                            // Copy BPP into local structure

    if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32)))    // Make sure all information is valid
    {
        MYLOG("Invalid texture information");    // Display Error
        if(fTGA != NULL)                                                    // Check if file is still open
        {
            fclose(fTGA);                                                    // If so, close it
        }
        return false;                                                        // Return failed
    }

    if(texture->bpp == 24)                                                    // If the BPP of the image is 24...
        texture->type    = GL_RGB;                                            // Set Image type to GL_RGB
    else                                                                    // Else if its 32 BPP
        texture->type    = GL_RGBA;                                            // Set image type to GL_RGBA

    tga.bytesPerPixel    = (tga.Bpp / 8);                                    // Compute the number of BYTES per pixel
    tga.imageSize        = (tga.bytesPerPixel * tga.Width * tga.Height);        // Compute the total amout ofmemory needed to store data
    texture->imageData    = (GLubyte *)malloc(tga.imageSize);                    // Allocate that much memory

    if(texture->imageData == NULL)                                            // If no space was allocated
    {
        MYLOG("Could not allocate memory for image");    // Display Error
        fclose(fTGA);                                                        // Close the file
        return false;                                                        // Return failed
    }

    if(fread(texture->imageData, 1, tga.imageSize, fTGA) != tga.imageSize)    // Attempt to read image data
    {
        MYLOG("Could not read image data");        // Display Error
        if(texture->imageData != NULL)                                        // If imagedata has data in it
        {
            free(texture->imageData);                                        // Delete data from memory
        }
        fclose(fTGA);                                                        // Close file
        return false;                                                        // Return failed
    }

    // Byte Swapping Optimized By Steve Thomas
    for(GLuint cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel)
    {
        texture->imageData[cswap] ^= texture->imageData[cswap+2] ^=
        texture->imageData[cswap] ^= texture->imageData[cswap+2];
    }

    fclose(fTGA);                                                            // Close file
    return true;                                                            // Return success
}

bool LoadCompressedTGA(Texture * texture, const char * filename, FILE * fTGA)        // Load COMPRESSED TGAs
{
    MYLOG(); 
    if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)                    // Attempt to read header
    {
        MYLOG("Could not read info header");        // Display Error
        if(fTGA != NULL)                                                    // If file is open
        {
            fclose(fTGA);                                                    // Close it
        }
        return false;                                                        // Return failed
    }

    texture->width  = tga.header[1] * 256 + tga.header[0];                    // Determine The TGA Width    (highbyte*256+lowbyte)
    texture->height = tga.header[3] * 256 + tga.header[2];                    // Determine The TGA Height    (highbyte*256+lowbyte)
    texture->bpp    = tga.header[4];                                        // Determine Bits Per Pixel
    tga.Width        = texture->width;                                        // Copy width to local structure
    tga.Height        = texture->height;                                        // Copy width to local structure
    tga.Bpp            = texture->bpp;                                            // Copy width to local structure

    if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32)))    //Make sure all texture info is ok
    {
        MYLOG("Invalid texture information");    // If it isnt...Display error
        if(fTGA != NULL)                                                    // Check if file is open
        {
            fclose(fTGA);                                                    // Ifit is, close it
        }
        return false;                                                        // Return failed
    }

    if(texture->bpp == 24)                                                    // If the BPP of the image is 24...
        texture->type    = GL_RGB;                                            // Set Image type to GL_RGB
    else                                                                    // Else if its 32 BPP
        texture->type    = GL_RGBA;                                            // Set image type to GL_RGBA

    tga.bytesPerPixel    = (tga.Bpp / 8);                                    // Compute BYTES per pixel
    tga.imageSize        = (tga.bytesPerPixel * tga.Width * tga.Height);        // Compute amout of memory needed to store image
    texture->imageData    = (GLubyte *)malloc(tga.imageSize);                    // Allocate that much memory

    if(texture->imageData == NULL)                                            // If it wasnt allocated correctly..
    {
        MYLOG("Could not allocate memory for image");    // Display Error
        fclose(fTGA);                                                        // Close file
        return false;                                                        // Return failed
    }

    GLuint pixelcount    = tga.Height * tga.Width;                            // Nuber of pixels in the image
    GLuint currentpixel    = 0;                                                // Current pixel being read
    GLuint currentbyte    = 0;                                                // Current byte 
    GLubyte * colorbuffer = (GLubyte *)malloc(tga.bytesPerPixel);            // Storage for 1 pixel

    do
    {
        GLubyte chunkheader = 0;                                            // Storage for "chunk" header

        if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)                // Read in the 1 byte header
        {
            MYLOG("Could not read RLE header");    // Display Error
            if(fTGA != NULL)                                                // If file is open
            {
                fclose(fTGA);                                                // Close file
            }
            if(texture->imageData != NULL)                                    // If there is stored image data
            {
                free(texture->imageData);                                    // Delete image data
            }
            return false;                                                    // Return failed
        }

        if(chunkheader < 128)                                                // If the ehader is < 128, it means the that is the number of RAW color packets minus 1
        {                                                                    // that follow the header
            chunkheader++;                                                    // add 1 to get number of following color values
            for(short counter = 0; counter < chunkheader; counter++)        // Read RAW color values
            {
                if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel) // Try to read 1 pixel
                {
                    MYLOG("Could not read image data");        // IF we cant, display an error

                    if(fTGA != NULL)                                                    // See if file is open
                    {
                        fclose(fTGA);                                                    // If so, close file
                    }

                    if(colorbuffer != NULL)                                                // See if colorbuffer has data in it
                    {
                        free(colorbuffer);                                                // If so, delete it
                    }

                    if(texture->imageData != NULL)                                        // See if there is stored Image data
                    {
                        free(texture->imageData);                                        // If so, delete it too
                    }

                    return false;                                                        // Return failed
                }
                                                                                        // write to memory
                texture->imageData[currentbyte        ] = colorbuffer[2];                    // Flip R and B vcolor values around in the process 
                texture->imageData[currentbyte + 1    ] = colorbuffer[1];
                texture->imageData[currentbyte + 2    ] = colorbuffer[0];

                if(tga.bytesPerPixel == 4)                                                // if its a 32 bpp image
                {
                    texture->imageData[currentbyte + 3] = colorbuffer[3];                // copy the 4th byte
                }

                currentbyte += tga.bytesPerPixel;                                        // Increase thecurrent byte by the number of bytes per pixel
                currentpixel++;                                                            // Increase current pixel by 1

                if(currentpixel > pixelcount)                                            // Make sure we havent read too many pixels
                {
                    MYLOG("Too many pixels read", "ERROR", NULL);            // if there is too many... Display an error!

                    if(fTGA != NULL)                                                    // If there is a file open
                    {
                        fclose(fTGA);                                                    // Close file
                    }    

                    if(colorbuffer != NULL)                                                // If there is data in colorbuffer
                    {
                        free(colorbuffer);                                                // Delete it
                    }

                    if(texture->imageData != NULL)                                        // If there is Image data
                    {
                        free(texture->imageData);                                        // delete it
                    }

                    return false;                                                        // Return failed
                }
            }
        }
        else                                                                            // chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
        {
            chunkheader -= 127;                                                            // Subteact 127 to get rid of the ID bit
            if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel)        // Attempt to read following color values
            {    
                MYLOG("Could not read from file");            // If attempt fails.. Display error (again)

                if(fTGA != NULL)                                                        // If thereis a file open
                {
                    fclose(fTGA);                                                        // Close it
                }

                if(colorbuffer != NULL)                                                    // If there is data in the colorbuffer
                {
                    free(colorbuffer);                                                    // delete it
                }

                if(texture->imageData != NULL)                                            // If thereis image data
                {
                    free(texture->imageData);                                            // delete it
                }

                return false;                                                            // return failed
            }

            for(short counter = 0; counter < chunkheader; counter++)                    // copy the color into the image data as many times as dictated 
            {                                                                            // by the header
                texture->imageData[currentbyte        ] = colorbuffer[2];                    // switch R and B bytes areound while copying
                texture->imageData[currentbyte + 1    ] = colorbuffer[1];
                texture->imageData[currentbyte + 2    ] = colorbuffer[0];

                if(tga.bytesPerPixel == 4)                                                // If TGA images is 32 bpp
                {
                    texture->imageData[currentbyte + 3] = colorbuffer[3];                // Copy 4th byte
                }

                currentbyte += tga.bytesPerPixel;                                        // Increase current byte by the number of bytes per pixel
                currentpixel++;                                                            // Increase pixel count by 1

                if(currentpixel > pixelcount)                                            // Make sure we havent written too many pixels
                {
                    MYLOG("Too many pixels read", "ERROR", NULL);            // if there is too many... Display an error!

                    if(fTGA != NULL)                                                    // If there is a file open
                    {
                        fclose(fTGA);                                                    // Close file
                    }    

                    if(colorbuffer != NULL)                                                // If there is data in colorbuffer
                    {
                        free(colorbuffer);                                                // Delete it
                    }

                    if(texture->imageData != NULL)                                        // If there is Image data
                    {
                        free(texture->imageData);                                        // delete it
                    }

                    return false;                                                        // Return failed
                }
            }
        }
    }

    while(currentpixel < pixelcount);                                                    // Loop while there are still pixels left
    fclose(fTGA);                                                                        // Close the file
    return true;                                                                        // return success
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)
int printOglError(char *file, int line)
{
    GLenum glErr;
    int retCode = 0;
    
    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @line %d: %s.\n", file, line, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

void printInfoLog(GLhandleARB obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                         &infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
        printf("%s.\n",infoLog);
        free(infoLog);
    }
}

char *textFileRead(const char *fn)
{
    printf("Read file [%s]\n", fn);
    FILE *fp;
    char *content = NULL;
    int count=0;
    if (fn != NULL) {
        fp = fopen(fn, "rt");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);
            if (count > 0) {
                content = (char *)malloc(sizeof(char) * (count+1));
                count = fread(content,sizeof(char),count,fp);
                content[count] = '\0';
            }
            fclose(fp);                            
        }
    }
    return content;
}

void setShaders(const char *vert, const char *frag, GLhandleARB &v, GLhandleARB &f, GLhandleARB &p)
{
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_DEPTH_TEST);

    char *vs = NULL,*fs = NULL;
    v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    vs = textFileRead(vert);
    fs = textFileRead(frag);

    //printf("test.vert=[%s]\n", vs);
    //printf("test.frag=[%s]\n", fs);

    const char *vv = vs;
    const char *ff = fs;

    glShaderSourceARB(v, 1, &vv,NULL);
    glShaderSourceARB(f, 1, &ff,NULL);

    free(vs);
    free(fs);
    
    glCompileShaderARB(v);
    glCompileShaderARB(f);

    printInfoLog(v);
    printInfoLog(f);

    p = glCreateProgramObjectARB();

    glAttachObjectARB(p,v);
    glAttachObjectARB(p,f);

    glLinkProgramARB(p);
    printInfoLog(p);

    glUseProgramObjectARB(p);

    printInfoLog(v);
    printInfoLog(f);
    printInfoLog(p);
}

#endif
