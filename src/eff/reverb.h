
#pragma once

#include <math.h>
#include <stdint.h>

class reverb
{
    private:
        int** Buf = new int* [2] { NULL, NULL };
        int Pos = 0;
        int Delta = 0;
        int currentCh = 0;
        int Chs = 0;
        int bufSize = 0;

        double* sl = new double[16] {
            0.0050000 , 0.0150000 , 0.0300000 , 0.0530000 ,
            0.0680000 , 0.0800000 , 0.0960000 , 0.1300000 ,
            0.2000000 , 0.3000000 , 0.4000000 , 0.5000000 ,
            0.6000000 , 0.7000000 , 0.8000000 , 0.9000000
        };

    public:
        double* SendLevel = NULL;

        reverb(int bufSize, int ch)
        {
            this->Buf = new int*[2] { new int[bufSize], new int[bufSize] };
            this->bufSize = bufSize;
            Chs = ch;
            initParams();
        }

        ~reverb()
        {
            delete Buf[0];
            delete Buf[1];
            delete[] Buf;

            delete[] sl;
        }

        void initParams()
        {
            this->Pos = 0;
            this->currentCh = 0;
            SetDelta(64);

            this->SendLevel = new double[Chs];
            for (int i = 0; i < Chs; i++)
            {
                SetSendLevel(i, 0);
            }

            this->Buf = new int*[2] { new int[bufSize], new int[bufSize] };
        }

        void SetDelta(int n)
        {
            this->Delta = (int)bufSize / 128 * fmax(fmin(n, 127), 0);
        }

        void SetSendLevel(int ch, int n)
        {
            if (n == 0)
            {
                SendLevel[ch] = 0;
                return;
            }
            //SendLevel[ch] = 1.0 / (2 << Math.Max(Math.Min((15 - n), 15), 0));
            n = fmax(fmin(n, 15), 0);
            SendLevel[ch] = 1.0 * sl[n];
            //Console.WriteLine("{0} {1}", ch, SendLevel[ch]);
        }

        int GetDataFromPos(int LorR)
        {
            if (LorR == 0) return Buf[0][Pos];
            return Buf[1][Pos];
        }

        int GetDataFromPosL()
        {
            return Buf[0][Pos];
        }

        int GetDataFromPosR()
        {
            return Buf[1][Pos];
        }

        void ClearDataAtPos()
        {
            Buf[0][Pos] = 0;
            Buf[1][Pos] = 0;
        }

        void UpdatePos()
        {
            Pos = (1 + Pos) % bufSize;
        }

        //public void StoreData(int ch, int v)
        //{
        //int ptr = (Delta + Pos) % Buf.Length;
        //Buf[ptr] += (int)(v * SendLevel[ch]);
        //}

        //[MethodImpl(MethodImplOptions.AggressiveInlining)]
        //public void StoreData(int LorR, int v)
        //{
        //    int ptr = (Delta + Pos) % Buf[0].Length;
        //    Buf[LorR][ptr] += (int)(v);
        //}

        void StoreDataL(int v)
        {
            int ptr = (Delta + Pos) % bufSize;
            Buf[0][ptr] += (int)(v);
        }

        void StoreDataR(int v)
        {
            int ptr = (Delta + Pos) % bufSize;
            Buf[1][ptr] += (int)(v);
        }

        void StoreDataC(int vL,int vR)
        {
            int ptr = (Delta + Pos) % bufSize;
            Buf[0][ptr] += (int)(vL);
            Buf[1][ptr] += (int)(vR);
        }

        void SetReg(uint32_t adr, uint8_t data)
        {
            if (adr == 0)
            {
                SetDelta(data & 0x7f);
            }
            else if (adr == 1)
            {
                currentCh = fmax(fmax(data & 0x3f, 38), 0);
                if ((data & 0x80) != 0) initParams();
            }
            else if (adr == 2)
            {
                SetSendLevel(currentCh, data & 0xf);
            }
        }
};

