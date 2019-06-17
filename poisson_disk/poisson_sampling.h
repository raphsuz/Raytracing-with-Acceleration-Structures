#ifndef poisson_disk_sampling
#define poisson_disk_sampling

#define EPSILON 1e-9f

#include <algorithm>
#include <bits/stdc++.h>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <numeric>
#include <random>
#include <stdint.h>
#include <sstream>
#include <time.h>
#include <vector>
#include "3d.h"

namespace Poisson_sampling
{
    class c11RNG
    {
    public:
        c11RNG(): gen(std::random_device()()), dis(0.0f, 1.0f)
        {
            gen.seed(time(nullptr));
        }

        explicit c11RNG(uint32_t seed): gen(seed), dis(0.0f, 1.0f){}

        float RNGFloat() {return static_cast<float>(dis(gen));}

        int RNGInt(int max)
        {
            std::uniform_int_distribution<> disInt(0, max);
            return disInt(gen);
        }
    private:
        std::mt19937 gen;
        std::uniform_real_distribution<float> dis;
    };

    struct samplePoint
    {
        samplePoint(): x(0), y(0), m_Verified(false){}
        samplePoint(float X, float Y): x(X), y(Y), m_Verified(true){}
        float x;
        float y;
        bool m_Verified;
        bool InRectangle() const {return x >= 0 && y >= 0 && x <= 1 && y <= 1;}
        bool InCircle() const
        {
            float cx = x - 0.5f;
            float cy = y - 0.5f;
            return ( cx * cx + cy * cy) <= 0.25f;
        }
    };

    struct sampleGridPoint
    {
        sampleGridPoint(int X, int Y) : x(X), y(Y) {}
        int x;
        int y;
    };
	
	float getDistance(const samplePoint& P1, const samplePoint& P2) {return sqrt( (P1.x - P2.x) * (P1.x - P2.x) + (P1.y - P2.y) * (P1.y - P2.y) );}
    sampleGridPoint ImageToGrid(const samplePoint P, float CellSize) {return sampleGridPoint( (int)(P.x / CellSize), (int)(P.y / CellSize));}
	
    struct sampleGrid
    {
        sampleGrid(int Weight, int Height, float CellSize): m_W(Weight), m_H(Height), m_CellSize(CellSize)
        {
            m_Grid.resize( m_H );
            for (auto i = m_Grid.begin(); i < m_Grid.end(); i++) { i->resize(m_W);}
        }
        void Insert(const samplePoint& P)
        {
            sampleGridPoint G = ImageToGrid(P, m_CellSize);
            m_Grid[G.x][G.y] = P;
        }
        bool IsAnyNeighbor(samplePoint P, float minDist, float CellSize)
        {
            sampleGridPoint G = ImageToGrid(P, CellSize);
            //檢查多少個鄰近samples
            const int D = 25;
            //開始搜尋和鄰居samples的距離
            for (int i = G.x - D; i < G.x + D; i++)
            {
                for (int j = G.y - D; j < G.y + D; j++)
                {
                    //落在畫布範圍內的才真正需要考慮
                    if (i >= 0 && i < m_W && j >= 0 && j < m_H)
                    {
                        samplePoint Compared = m_Grid[i][j];
                        if (Compared.m_Verified && getDistance(Compared, P) < minDist) {return true;}
                    }
                }
            }
            return false;
        }
    private:
        int m_W;
        int m_H;
        float m_CellSize;
        std::vector<std::vector<samplePoint>> m_Grid;
    };

    template <typename PRNG>
    samplePoint popRNG(std::vector<samplePoint>& Points, PRNG& Generator)
    {
        const int Index = Generator.RNGInt(Points.size()-1);
        const samplePoint P = Points[Index];
        Points.erase(Points.begin() + Index);
        return P;
    }

    template <typename PRNG>
    samplePoint GenerateRandomPointAround(const samplePoint& P, float MinDist, PRNG& Generator)
    {
        //non-uniform distribution
        float R1 = Generator.RNGFloat();
        float R2 = Generator.RNGFloat();

        float Radius = MinDist * (R1 + 1.0f);

        float Angle = 2 * PI * R2;

        float X = P.x + cos(Angle) * Radius;
        float Y = P.y + sin(Angle) * Radius;

        return samplePoint(X, Y);
    }

    template <typename PRNG = c11RNG>
    std::vector<samplePoint> GeneratePoissonPoints(
            size_t NumPoints,
            PRNG& Generator,
            int NewPointsCount = 30,
            bool Circle = true,
            float minDist = -1.0f
            )
    {
        //minDist初值設定
        if (minDist < 0.0f) {minDist = sqrt(float(NumPoints)) / float(NumPoints);}
        std::cout << "minDist："<< minDist << std::endl;

    std::vector<samplePoint> ActiveSamplePoints;
    std::vector<samplePoint> ProcessingList;

    //grid
    float CellSize = minDist / sqrt(2.0f);
    int GridW = (int)ceil(1.0f / CellSize);
    int GridH = (int)ceil(1.0f / CellSize);

    sampleGrid Grid(GridW, GridH, CellSize);

    samplePoint StartingPoint;
    do {StartingPoint = samplePoint(Generator.RNGFloat(), Generator.RNGFloat());} while (!(Circle ? StartingPoint.InCircle() : StartingPoint.InRectangle()));

    ProcessingList.push_back(StartingPoint);
    ActiveSamplePoints.push_back(StartingPoint);
    Grid.Insert(StartingPoint);

    while(!ProcessingList.empty() && ActiveSamplePoints.size() < NumPoints)
    {
        samplePoint Point = popRNG<PRNG>(ProcessingList, Generator);
        for (int i = 0; i < NewPointsCount; i++)
        {
            samplePoint NewPoint = GenerateRandomPointAround(Point, minDist, Generator);
            bool Fits = Circle ? NewPoint.InRectangle() : NewPoint.InCircle();
            if (Fits && !Grid.IsAnyNeighbor(NewPoint, minDist, CellSize))
            {
                ProcessingList.push_back(NewPoint);
                ActiveSamplePoints.push_back(NewPoint);
                Grid.Insert(NewPoint);
                continue;
            }
        }
    }
    return ActiveSamplePoints;
    }
}
// namespace Poisson_Sampling
#endif
