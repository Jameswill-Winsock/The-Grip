## Plan for first prototype
1. after RESET is released 486 begins its first memory-read transaction at physical address 0xFFFFFFF0
2. fpga accepts req
3. recognises address for reset location as bios rom
4. read four bytes from bios rom
5. put them on cpu data bus
6. assert ready
7. cpu accepts data, proceeds from bios onwards

quick note: assert ready or similar stuff here will assume active low because thats how am486 works unless stated otherwise

### For now, we wont start with real SDRAM
why? because then i'll have to initialise it (what i call as potty training the ram), refresh, row addressing, timing, clock crossover bullshit

so we setup a fake ram and boot rom directly in the fpga for now, and any requests for the ram or rom will be intercepted by the fpga (which is our address decoder), and processed with our fake shit for now

### this leaves us with four tasks 
(or i.e., four rtls):
1. **CPU Bus:**
    - check for transaction start from CPU
    - remember requested address
    - determine if a read or write was requested
    - pass job to memory and wait
    - assert RDY#
2. **Address decoder:**
    - Current simple memory map:
        1. 00000000–0009FFFF   RAM
        2. 000A0000–000EFFFF   unused for now
        3. 000F0000–000FFFFF   BIOS ROM
        4. 00100000–03FFFFFF   RAM
        5. FFFF0000–FFFFFFFF   BIOS ROM mirror
    - if request is sent in for top of block 5, or anywhere between block 3, we point cpu to the bios rom
    - if request is sent for block 1 or block 4, we point cpu to ram
    - any other request, we mark invalid (i.e return 0xFFFFFFFF and assert RDY#) and optionally record the invalid access for debugging
3. **Boot ROM:** 
    - for now, a simple diagnostic program. doesn't have to be full bios, can even just do a simple unconditional jump to another location or do some simple arithmetic to test the cpu. (yes that makes it an overkill calculator but who cares)
4. **RAM:**
    - our current very own fake ram. ram prices so bad have to resort to faking ram lmfao.
    - supposed to be a simulated ram array that supports 32 bit r/w ops and eventually byte ops later

### first sim goal
1. reset CPU model
2. cpu requests address 0xFFFFFFF0 (block 5 in our memory map)
3. address decoder selects bios
4. bios returns expected instruction bytes
5. fpga asserts RDY# 

In the bios, we do a simple program:
1. write 0x12345678 to ram address 0x00100000
2. read address 0x00100000
3. verify that 0x12345678 comes back

initially i embed the diagnostic boot rom into the fpga bitstream later we can read the bios from flash into bram or sdram before releasing RESET

### later goal after physical ram chips arrive
direct the fake ram rtl to instead the actual sdram controller inside our fpga, which deals with the ram then (and loads teh bios by copying it from flash over to the designated ram block)
during sim, ram will be represented by a behavioural memory array. during the first physical bring-up, a small amount of fpga block ram will act
as temporary diagnostic ram. the full 64 mb memory map will become available after the sdram controller is implemented.


## Reset sequence
1. power stabilises
2. cpu clk starts running
3. hold RESET high
4. Wait at least 1 ms
5. pull RESET low
6. cpu internally initializes itself
7. cpu places 0xFFFFFFF0 on its address bus
8. cpu pulls ADS# low
9. fpga sees ADS# and answers with bios data
10. fpga pulls RDY# low
11. cpu accepts the instruction

### Minimal pinout for signals involved in the first bus transaction
```text
RESET       fpga output to cpu
ADS#        cpu output to fpga 
A31–A2      cpu address output
BE3#–BE0#   cpu byte selection
M/IO        memory or i/o
D/C         data or control/code
W/R         read or write
D31–D0      data bus
RDY#        fpga output to cpu to assert ready
```

for first instruction fetch we'll prob see sum shi like:
1. ADS# = 0          
    - new bus transaction
2. M/IO = 1          
    - memory transaction
3. D/C  = 0          
    - code/control read
4. W/R  = 0          
    - read
5. Address = FFFFFFF0

i think they classify a code fetch as a control cycle so M/IO = 1 is memory req and W/R = 0 means read request
and 486 has 32 bit data bus so one bus transaction moves upto 4 bytes in one go, depending on which byte enable pin is active
so for some accursed reason which i'll explain below it doesn't use A1,A0 and outputs only A31-A2 (apparently it doesnt even have physical A1 and A0 pins in existence????? the fuck???????)

### the dank ass reason there's no A1 and A0
A31–A2 select a group of four bytes
then BE0#–BE3# select which byte or bytes inside that group the cpu wants

notice the two letters B and E together? yeah thats the instruction for "Byte Enable"

example group:

- A31–A2 select base address 0x1000
- BE0# selects 0x1000
- BE1# selects 0x1001
- BE2# selects 0x1002
- BE3# selects 0x1003

note that BE0# through BE3# are four seperate pins altogether, not two wires like you'd typically see for A1 and A0 to do the byte selection work.