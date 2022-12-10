#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <verilated_vcd_c.h>
#include "verilated.h"
#include "Vspi.h"
#include "spislave.h"

#define LOG(...) {printf("%llu: ", tickcount); printf(__VA_ARGS__); }

Vspi *pCore;
VerilatedVcdC *pTrace = NULL;
uint64_t tickcount = 0;
uint64_t ts = 1000;

void opentrace(const char *vcdname) {
    if (!pTrace) {
        pTrace = new VerilatedVcdC;
        pCore->trace(pTrace, 99);
        pTrace->open(vcdname);
    }
}

void tick(int t = 3) {
    if (t&1) {
        pCore->i_clk = 0;
        pCore->eval();
        if(pTrace) pTrace->dump(static_cast<vluint64_t>(tickcount));
        tickcount += ts / 2;
    }
    if (t&2) {
        pCore->i_clk = 1;
        pCore->eval();
        if(pTrace) pTrace->dump(static_cast<vluint64_t>(tickcount));
        tickcount += ts / 2;
    }
}

void halftick() {
    pCore->i_clk = !pCore->i_clk;
    pCore->eval();
    if(pTrace) pTrace->dump(static_cast<vluint64_t>(tickcount));
    tickcount += ts / 2;
}


void reset() {
    pCore->i_reset = 1;
    tick();
    pCore->i_reset = 0;
}

void handle(Vspi *pCore) {
    int ret = spislave_handle();
    if (ret>=0) {
        LOG("mosi: %02x\n", (uint8_t)ret);
    }
}

void write_reg(Vspi *pCore, uint8_t a, uint8_t d) {
    pCore->i_dat = d;
    pCore->i_addr = a;
    pCore->i_cs = 1;
    pCore->i_we = 1;
    tick();
    pCore->i_dat = 0;
    pCore->i_addr = 0;
    pCore->i_cs = 0;
    pCore->i_we = 0;
}

int read_reg(Vspi *pCore, uint8_t a) {
    pCore->i_dat = 0;
    pCore->i_addr = a;
    pCore->i_cs = 1;
    pCore->i_we = 0;
    tick();
    pCore->i_addr = 0;
    pCore->i_cs = 0;
    return pCore->o_dat;
}

int main(int argc, char *argv[]) {
    printf("spi simulation\n\n");

    pCore = new Vspi();

#ifdef TRACE
    Verilated::traceEverOn(true);
    opentrace("trace.vcd");
    printf("Trace enabled.\n");
#endif

    reset();

    for( int i = 0; i<10; i++) {
        tick();
    }

    uint8_t mosi_data[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t miso_data[] = {0xaa, 0xbb, 0xcc, 0xdd};
    uint8_t idx = 0;

    spislave_init(&pCore->o_sck, &pCore->i_miso, &pCore->o_mosi, &pCore->o_ss);
    for (int i = 0; i < sizeof(miso_data); i++) {
        spislave_set_miso(miso_data[i]);
    }

    write_reg(pCore, 0, 1);
    write_reg(pCore, 1, mosi_data[idx]);

    idx++;
    while( !Verilated::gotFinish() ) {
        handle(pCore);
        if(pCore->o_irq) {
            // printf("miso: %02x\n", read_reg(pCore, 1));
            tick(); tick();
            if (idx < sizeof(mosi_data)) {
                write_reg(pCore, 1, mosi_data[idx]);
                idx++;
            } else {
                write_reg(pCore, 0, 0);
                break;
            }
        }
        tick();
        if(tickcount > 10000*ts) {
            break;
        }
    }

    tick(); tick();

    pCore->final();
    delete pCore;

    if (pTrace) {
        pTrace->close();
        delete pTrace;
    }
}
