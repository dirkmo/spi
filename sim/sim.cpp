#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <verilated_vcd_c.h>
#include "verilated.h"
#include "Vspi.h"

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

void reset() {
    pCore->i_reset = 1;
    tick();
    pCore->i_reset = 0;
}

void handle(Vspi *pCore) {
    tick();
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

    write_reg(pCore, 0, 1);
    write_reg(pCore, 1, 123);

    while( !Verilated::gotFinish()) {
        handle(pCore);
        if(tickcount > 10000*ts) {
            break;
        }
    }

    pCore->final();
    delete pCore;

    if (pTrace) {
        pTrace->close();
        delete pTrace;
    }
}
