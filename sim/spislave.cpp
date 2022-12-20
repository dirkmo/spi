#include "spislave.h"
#include <stdio.h>
#include <queue>

using namespace std;

static uint8_t *clk;
static uint8_t *sck;
static uint8_t *miso;
static uint8_t *mosi;
static uint8_t *ss;

static queue<uint8_t> v_miso_data;

void spislave_init(uint8_t *_sck, uint8_t *_miso, uint8_t *_mosi, uint8_t *_ss) {
    sck = _sck;
    miso = _miso;
    mosi = _mosi;
    ss = _ss;
}

// Mode 0: Data sampled on rising edge and shifted out on the falling edge

int spislave_handle() {
    static uint8_t old_sck = -1;
    static uint8_t mosi_dat;
    static uint8_t miso_dat;
    static uint8_t old_ss = 0;
    static int bitidx = 0;
    bool posedge = (old_sck == 0) && (*sck == 1);
    bool negedge = (old_sck == 1) && (*sck == 0);
    int ret = -1;
    if (*ss) {
        if (posedge) {
            if (bitidx == 0) {
                miso_dat = 0;
                if (v_miso_data.size() > 0) {
                    miso_dat = v_miso_data.front();
                    v_miso_data.pop();
                }
            }
            mosi_dat = (*mosi | (mosi_dat << 1)) & 0xff;
            if (bitidx == 7) {
                ret = mosi_dat;
            }
        }
        if (negedge) {
            miso_dat = (miso_dat << 1) & 0xff;
            bitidx = (bitidx + 1) % 8;
        }
        *miso = (miso_dat>>7) & 1;
        // printf("bit %d\n",*miso);
    } else {
        bitidx = 0;
    }
    old_sck = *sck;
    old_ss = *ss;
    return ret;
}

void spislave_set_miso(uint8_t dat) {
    v_miso_data.push(dat);
}
