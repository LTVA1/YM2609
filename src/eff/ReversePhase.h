#pragma once

#include <math.h>
#include <stdint.h>

class ReversePhase
{
    public:
        int*** SSG;
        int*** FM;
        int** Rhythm;
        int** AdpcmA;
        int** Adpcm;

        ReversePhase()
        {
            Init();
        }

        ~ReversePhase()
        {
            for(int i = 0; i < 4; i++)
            {
                for(int j = 0; j < 3; j++)
                {
                    delete SSG[i][j];
                }
            }

            for(int i = 0; i < 2; i++)
            {
                for(int j = 0; j < 6; j++)
                {
                    delete FM[i][j];
                }
            }
            
            for(int i = 0; i < 6; i++)
            {
                delete Rhythm[i];
            }

            for(int i = 0; i < 6; i++)
            {
                delete AdpcmA[i];
            }

            for(int i = 0; i < 3; i++)
            {
                delete Adpcm[i];
            }
        }

        void SetReg(uint32_t adr, uint8_t data)
        {
            switch (adr)
            {
                case 0://$CC
                    for (int i = 0; i < 6; i++)
                        SSG[0][i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 1://$CD
                    for (int i = 0; i < 6; i++)
                        SSG[1][i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 2://$CE
                    for (int i = 0; i < 6; i++)
                        SSG[2][i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 3://$CF
                    for (int i = 0; i < 6; i++)
                        SSG[3][i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;

                case 4://$D0
                    for (int i = 0; i < 6; i++)
                        FM[0][i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 5://$D1
                    for (int i = 0; i < 6; i++)
                        FM[0][i / 2 + 3][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 6://$D2
                    for (int i = 0; i < 6; i++)
                        FM[1][i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 7://$D3
                    for (int i = 0; i < 6; i++)
                        FM[1][i / 2 + 3][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;

                case 8://$D4
                    for (int i = 0; i < 6; i++)
                        Rhythm[i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 9://$D5
                    for (int i = 0; i < 6; i++)
                        Rhythm[i / 2 + 3][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;

                case 10://$D6
                    for (int i = 0; i < 6; i++)
                        AdpcmA[i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
                case 11://$D7
                    for (int i = 0; i < 6; i++)
                        AdpcmA[i / 2 + 3][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;

                case 12://$D8
                    for (int i = 0; i < 6; i++)
                        Adpcm[i / 2][(i + 1) & 1] = (data & (1 << i)) != 0 ? -1 : 1;
                    break;
            }
        }

    private:
        void Init()
        {
            SSG = new int**[4]{
                new int*[3] { new int[2], new int[2], new int[2] },
                new int*[3] { new int[2], new int[2], new int[2] },
                new int*[3] { new int[2], new int[2], new int[2] },
                new int*[3] { new int[2], new int[2], new int[2] }
            };
            FM = new int**[2]{
                new int*[6] { new int[2], new int[2], new int[2], new int[2], new int[2], new int[2] },
                new int*[6] { new int[2], new int[2], new int[2], new int[2], new int[2], new int[2] }
            };
            Rhythm = new int*[6] { new int[2], new int[2], new int[2], new int[2], new int[2], new int[2] };
            AdpcmA = new int*[6] { new int[2], new int[2], new int[2], new int[2], new int[2], new int[2] };
            Adpcm = new int*[3] { new int[2], new int[2], new int[2] };

            for (int i = 0; i < 6; i++)
            {
                SSG[0][i / 2][i % 2] = 1;
                SSG[1][i / 2][i % 2] = 1;
                SSG[2][i / 2][i % 2] = 1;
                SSG[3][i / 2][i % 2] = 1;

                FM[0][i / 2][i % 2] = 1;
                FM[0][i / 2 + 3][i % 2] = 1;
                FM[1][i / 2][i % 2] = 1;
                FM[1][i / 2 + 3][i % 2] = 1;

                Rhythm[i / 2][i % 2] = 1;
                Rhythm[i / 2 + 3][i % 2] = 1;

                AdpcmA[i / 2][i % 2] = 1;
                AdpcmA[i / 2 + 3][i % 2] = 1;

                Adpcm[i / 2][i % 2] = 1;
            }
        }
};
