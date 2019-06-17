#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <memory.h>
#include <time.h>
#include "poisson_sample.h"

int ImageSize = 64;

//調用bmp儲存
#if defined( __GNUC__ )
#	define GCC_PACK(n) __attribute__((packed,aligned(n)))
#else
#	define GCC_PACK(n) __declspec(align(n))
#endif // __GNUC__

#pragma pack(push, 1)
struct GCC_PACK( 1 ) tagBITMAPFILEHEADER
{
    // BITMAPFILEHEADER
    unsigned short    bfType;   //2
    uint32_t          bfSize;   //4, file size in bytes
    unsigned short    bfReserved1;  //2
    unsigned short    bfReserved2;  //2
    uint32_t          bfOffBits;    //4, offset to image data, bytes
    // BITMAPINFOHEADER
    uint32_t          biSize;   //4, header size in bytes
    uint32_t          biWidth;  //4, width of image
    uint32_t          biHeight; //4, height of image
    unsigned short    biPlanes; //2, number of color planes
    unsigned short    biBitCount;   //2, bits per pixel
    uint32_t          biCompression;    //4, compression type
    uint32_t          biSizeImage;  //4, image size in bytes
    uint32_t          biXPelsPerMeter;  //4
    uint32_t          biYPelsPerMeter;  //4
    uint32_t          biClrUsed;    //4, number of colors
    uint32_t          biClrImportant;   //4, important colors
};
#pragma pack(pop)

#pragma pack(4)
typedef   struct   tagRGBQUAD   {
    unsigned   char   rgbBlue;  //Blue value
    unsigned   char   rgbGreen; //Green value
    unsigned   char   rgbRed;   //Red value
    unsigned   char   rgbReserved;
}   RGBQUAD;
#pragma pack()

float* DensityMap = nullptr;

void SaveBMP( const char* FileName, const void* RawBGRImage, int Width, int Height )
{
    tagBITMAPFILEHEADER Header;

    int ImageSize = Width * Height * 3;

    Header.bfType          = 0x4D * 256 + 0x42;
    Header.bfSize          = ImageSize + sizeof( Header );
    Header.bfReserved1     = 0;
    Header.bfReserved2     = 0;
    Header.bfOffBits       = 0x36;
    Header.biSize          = 40;
    Header.biWidth         = Width;
    Header.biHeight        = Height;
    Header.biPlanes        = 1;
    Header.biBitCount      = 24;
    Header.biCompression   = 0;
    Header.biSizeImage     = ImageSize;
    Header.biXPelsPerMeter = 6000;
    Header.biYPelsPerMeter = 6000;
    Header.biClrUsed       = 0;
    Header.biClrImportant  = 0;

    std::ofstream File( FileName, std::ios::out | std::ios::binary );

    File.write( (const char*)&Header, sizeof(Header) );
    File.write( (const char*)RawBGRImage, ImageSize );

    std::cout << "輸出BMP：" << FileName << std::endl;
}

unsigned char* LoadBMP(const char* FileName, int* OutWidth, int* OutHeight)
{
    tagBITMAPFILEHEADER Header;
    std::ifstream File(FileName, std::ifstream::binary);
    *OutWidth = Header.biWidth;
    *OutHeight = Header.biHeight;

    size_t DataSize = 3 * Header.biWidth * Header.biHeight;
    unsigned char* Img = new unsigned char[DataSize];
    File.read( (char*)Img, DataSize );
    return Img;
}

void LoadDensityMap(const char* FileName)
{
    int W, H;
    unsigned char* Data = LoadBMP(FileName, &W, &H);
    if (W!= ImageSize || H != ImageSize) {std::cout << "error: 現在的density map設定為："<< W << " x " << H << "，但必須設定為" << ImageSize << " x " << ImageSize << std::endl; exit(255);}
    DensityMap = new float[W * H];

    for(int y = 0; y < H; y++)
    {
        for(int x = 0; x < W; x++)
        {
            DensityMap[x + y * W] = float(Data[3 * (x + y * W)]) / 255.0f;
        }
    }
    delete[](Data);
}

int main(int argc, char** argv)
{
    int SampleNum = 0.0f;
    float minDist = -1.0f;
    std::cout << "請輸入欲選取的樣本數目(number)：" << std::endl;
    std::cin >> SampleNum;
    std::cout << "請輸入範圍尺寸的長度(正方形)：" << std::endl;
    std::cin >> ImageSize;
    std::cout << "D value設定（0至1之間）：" << std::endl;
    std::cin >> minDist;
    std::cout << "欲選取的樣本數目為 " << SampleNum << " ，範圍尺寸為 " << ImageSize << " x " << ImageSize << std::endl;
    std::cout << "最小距離："<< minDist << " ，取樣中，請稍待片刻..." << std::endl;

    //計時開始
    std::chrono::milliseconds three_milliseconds{3};
    auto t1 = std::chrono::system_clock::now();

    Poisson_sampling::c11RNG PRNG;
    const auto Points = Poisson_sampling::GeneratePoissonPoints(SampleNum, PRNG, 30, 0, minDist);

    //計時結束
    auto t2 = std::chrono::system_clock::now();
    std::cout << "取樣流程耗時總計："
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
              << " milliseconds。" << std::endl;

    size_t DataSize = 3 * ImageSize * ImageSize;
    unsigned char* Img = new unsigned char[DataSize];
    memset(Img, 0, DataSize);

    for (auto i = Points.begin(); i < Points.end(); i++)
    {
        int x = int(i->x * ImageSize);
        int y = int(i->y * ImageSize);
        if(DensityMap)
        {
            float R = PRNG.RNGFloat();
            float P = DensityMap[x + y * ImageSize];
            if (R > P) continue;
        }
        int Base = 3 * (x + y * ImageSize);
        Img[Base + 0] = Img[Base + 1] = Img[Base + 2] = 255;
    }

    SaveBMP("Poisson_Samples_TestOutput.bmp", Img, ImageSize, ImageSize);
    delete[](Img);

    std::ofstream File( "Sample_List.txt", std::ios::out );
    std::cout << "取樣統計："<< Points.size() << std::endl;
    File << "樣本數目："<< Points.size() << "\r\n" << std::endl;
    for ( const auto& p : Points )
    {
        File << "X: " << p.x << ", Y: " << p.y << "\r\n" << std::endl;
    }
    std::cout << "取樣點存入Sample_List.txt"  << std::endl;

    return 0;
}
