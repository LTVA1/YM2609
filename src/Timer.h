#pragma once

#include <cmath>
#include <stdint.h>
#include <stdbool.h>

class Timer
{
    public:
        void Reset()
        {
            timera_count = 0;
            timerb_count = 0;
        }

        bool Count(int us)
        {
            bool Event = false;

            if (timera_count != 0)
            {
                timera_count -= us << 16;
                if (timera_count <= 0)
                {
                    Event = true;
                    TimerA();

                    while (timera_count <= 0)
                        timera_count += timera;

                    if ((regtc & 4) != 0)
                        SetStatus(1);
                }
            }
            if (timerb_count != 0)
            {
                timerb_count -= us << 12;
                if (timerb_count <= 0)
                {
                    Event = true;
                    while (timerb_count <= 0)
                        timerb_count += timerb;

                    if ((regtc & 8) != 0)
                        SetStatus(2);
                }
            }
            return Event;
        }

        int GetNextEvent()
        {
            uint32_t ta = (uint32_t)(((timera_count + 0xffff) >> 16) - 1);
            uint32_t tb = (uint32_t)(((timerb_count + 0xfff) >> 12) - 1);
            return (uint32_t)((ta < tb ? ta : tb) + 1);
        }

    //protected:
        void SetStatus(uint32_t bit)
        {
        }

        void ResetStatus(uint32_t bit)
        {
        }

        void SetTimerBase(uint32_t clock)
        {
            timer_step = (int)(1000000.0 * 65536 / clock);
        }

        void SetTimerA(uint32_t addr, uint32_t data)
        {
            uint32_t tmp;
            regta[addr & 1] = (uint8_t)(data);
            tmp = (uint32_t)((regta[0] << 2) + (regta[1] & 3));
            timera = (int)((1024 - tmp) * timer_step);
        }

        void SetTimerB(uint32_t data)
        {
            timerb = (int)((256 - data) * timer_step);
        }

        void SetTimerControl(uint32_t data)
        {
            uint32_t tmp = regtc ^ data;
            regtc = (uint8_t)(data);

            if ((data & 0x10) != 0)
                ResetStatus(1);
            if ((data & 0x20) != 0)
                ResetStatus(2);

            if ((tmp & 0x01) != 0)
                timera_count = (data & 1) != 0 ? timera : 0;
            if ((tmp & 0x02) != 0)
                timerb_count = (data & 2) != 0 ? timerb : 0;
        }


        uint8_t status;
        uint8_t regtc;

    private:
        void TimerA()
        {
        }

        uint8_t regta[2];

        int timera, timera_count;
        int timerb, timerb_count;
        int timer_step;
};
