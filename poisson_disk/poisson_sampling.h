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

using namespace std;
const double PI = acos(-1);

namespace Poisson_sampling
{
    class c11RNG
    {
    public:
        c11RNG(): gen( std::random_device()() ), dis(0.0f, 1.0f)
        {
            gen.seed( time(nullptr) );
        }

        explicit c11RNG(uint32_t seed): gen(seed), dis(0.0f, 1.0f){}

        float RNGFloat() {return static_cast<float>(dis(gen));}

        int RNGInt(int max)
        {
            std::uniform_int_distribution<> disInt(0, Max);
            return disInt(gen);
        }
    private:
        std::mt19937 gen;
        std::uniform_real_distribution<float> dis;
    };

    struct samplePoint
    {
        samplePoint(): x(0), y(0), m_Verified(false){}
        samplePoint(): x(X), y(Y), m_Verified(true){}
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
            const int D = 125;
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
        std::vector< std::vector<samplePoint> > m_Grid;
    };

    template <typename PRNG>
    samplePoint popRNG(std::vector<samplePoint>& Points, PRNG& Generator)
    {
        const int Index = Generator.RNGInt(Points.size()-1);
        const samplePoint P = Points[Index];
        Points.erase(Points.begin() + Idx);
        return P;
    }

    template <typename PRNG>
    samplePoint GenerateSurroundingPoint(const samplePoint& P, float minDist, PRNG& Generator)
    {
        //non-uniform distribution
        float R1 = Generator.RNGFloat();
        float R2 = Generator.RNGFloat();

        //Radius在D~2D之間
        float Radius = (R1 + 1.0f) * minDist;
        float Angle = 2 * PI * R2;

        //產生在(x, y)週遭的其它點
        float X = P.x + cos(Angle) * Radius;
        float Y = P.x + sin(Angle) * Radius;

        return samplePoint(X, Y);
    }

    float getDistance(const samplePoint& P1, const samplePoint& P2) {return sqrt( (P1.x - P2.x) * (P1.x - P2.x) + (P1.y - P2.y) * (P1.y - P2.y) );}
    sampleGridPoint ImageToGrid(const samplePoint P, float CellSize) {return sampleGridPoint( (int)(P.x / CellSize), (int)(P.y / CellSize));}

    template <typename PRNG = c11RNG>
    std::vector<samplePoint> GeneratePoissonPoints(
            size_t NumPoints,
            PRNG& Generator,
            int NewPointsCount = 20,
            bool Circle = true,
            float minDist = -1.0f
            )
    {
        //minDist初值設定
        if (minDist < 0.0f) {minDist = sqrt(float(NumPoints) / float(NumPoints));}

    std::vector<samplePoint> ActiveSamplePoints;
    std::vector<samplePoint> ProcessingList;

    //grid
    float CellSize = minDist / 2;
    int GridW = (int)ceil(1.0f / CellSize);
    int GridH = (int)ceil(1.0f / CellSize);

    sampleGrid Grid(GridW, GridH, CellSize);

    samplePoint StartingPoint;
    do {StartingPoint = samplePoint(Generator.RNGFloat(), Generator.RNGFloat());}
    while (!(Circle ? StartingPoint.InCircle():StartingPoint.InRectangle()));

    ProcessingList.push_back(StartingPoint);
    ActiveSamplePoints.push_back(StartingPoint);
    Grid.Insert(StartingPoint);

    while(!ProcessingList.empty() && ActiveSm < NumPoints)
    {
        if (ActiveSamplePoints.size() % 100 == 0) std::cout << ".";

        samplePoint Point = popRNG<PRNG>(ProcessingList, Generator);
        for (int i = 0; i < NewPointsCount; i++)
        {
            samplePoint NewPoint = GenerateSurroundingPoint(Point, minDist, Generator);
            bool Fits = Circle ? NewPoint.InCircle() : NewPoint.InRectangle();
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
