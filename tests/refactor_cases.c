/* Included after either the original or refactored production source. */
static void Queue(const uint8_t *data, unsigned count)
{
    assert(rx_count+count <= sizeof(rx_data));
    memcpy(rx_data+rx_count,data,count); rx_count+=count;
}
static void Poll(const char *label)
{
    TestPoll();
    assert(rx_read==rx_count); rx_count=rx_read=0;
    Snapshot(label);
}
static void Command(unsigned cmd, unsigned sub, unsigned axis, unsigned direction, unsigned speed, uint32_t distance)
{
    uint8_t f[14]={0x55,0x55,0,0,0,0,0,0,0,0,0,0,0xaa,0xaa};
    f[2]=(uint8_t)cmd; f[3]=(uint8_t)sub; f[4]=(uint8_t)axis; f[5]=(uint8_t)direction;
    f[6]=(uint8_t)(speed>>8); f[7]=(uint8_t)speed;
    f[8]=(uint8_t)(distance>>24); f[9]=(uint8_t)(distance>>16);
    f[10]=(uint8_t)(distance>>8); f[11]=(uint8_t)distance;
    Queue(f,14);
}
int main(int argc, char **argv)
{
    unsigned n=argc>1 ? (unsigned)atoi(argv[1]) : 0, i;
    static const unsigned speeds[]={1000,1001,3000,10000};
    static const unsigned distances[]={1,2,3,100,5000,12000};
    registers_tmc[0x04]=0x30000000; registers_tmc[0x6c]=0x10400050;
    TestInit(); Snapshot("init");
    if(n==0) {
        Poll("idle"); tick=9; Poll("9ms"); tick=10; Poll("10ms");
        tick=499; Poll("499ms"); tick=500; Poll("500ms");
        tick=0xfffffff8; Poll("wrap-before"); tick=3; Poll("wrap-after");
    } else if(n==1) {
        uint8_t first[]={0,0x12,0x55,0,0x55};
        uint8_t rest[]={0x55,1,0,0,0,0,0,0,0,0,0,0xaa,0xaa};
        ore=1; Queue(first,sizeof(first)); Poll("noise-partial");
        Queue(rest,sizeof(rest)); Poll("echo-rest");
        Command(1,0,0,0,0,0); Command(1,0,0,0,0,0); Poll("echo-burst");
        Command(1,0,0,0,0,1); Poll("echo-invalid");
    } else if(n==2) {
        for(i=0;i<8;i++) {
            Command(2,0,i==0?2:1,i==1?2:0,i==2?999:i==3?10001:1000,
                    i==4?0:i==5?12001:i==6?5001:10);
            if(i==7) rx_data[rx_count-1]=0;
            Poll("invalid-start");
        }
        Command(2,2,1,0,0,0); Poll("stop-idle");
        Command(2,2,2,0,0,0); Poll("stop-axis");
        Command(2,2,1,0,0,0); rx_data[rx_count-1]=0; Poll("stop-tail");
        Command(2,3,99,99,99,99); Poll("status-ignored-data");
        Command(2,3,0,0,0,0); rx_data[rx_count-1]=0; Poll("status-tail");
    } else if(n==3) {
        Command(2,0,1,0,1000,10); Command(2,3,0,0,0,0);
        Command(2,2,1,0,0,0); Poll("start-query-stop-one-drain");
        Command(2,3,0,0,0,0); Poll("status-stopped");
    } else if(n==4) {
        Command(2,0,1,1,3000,100); Command(2,0,1,0,1000,10); Poll("busy-second-start");
        for(i=0;i<20;i++) MockPulse();
        Command(2,3,0,0,0,0); Poll("status-active");
        Command(2,2,1,0,0,0); Poll("stop-active");
    } else if(n==5) {
        Command(2,0xf0,1,0,10000,0x12345678); Poll("continuous-start");
        for(i=0;i<6000;i++) MockPulse();
        Command(2,3,0,0,0,0); Poll("continuous-cruise");
        Command(2,2,1,0,0,0); Poll("continuous-stop");
    } else if(n>=6 && n<=8) {
        if(n==6) input_levels[2]&=~GPIO_PIN_6;
        if(n==7) input_levels[1]&=~GPIO_PIN_15;
        if(n==8) input_levels[1]&=~GPIO_PIN_14;
        Command(2,0,1,0,1000,10); Poll("limit-before-start");
    } else if(n>=9 && n<=16) {
        fault=n-8;
        Command(2,0,1,0,1000,10); Poll("prepare-fault");
        fault=0; input_levels[2]=65535;
        Command(2,0,1,0,1000,10); Poll("retry-after-fault");
    } else if(n==17) {
        Command(2,0,1,0,1000,2); Poll("two-step-start");
        MockPulse(); MockPulse(); input_levels[2]&=~GPIO_PIN_6;
        Command(2,2,1,0,0,0); Poll("done-stop-limit-same-poll");
    } else if(n==18) {
        for(i=3;i<=4;i++) {
            Command(i,0,1,0,0,0); Poll("aux-on");
            Command(i,0,2,0,0,0); Poll("aux-off");
            Command(i,0,3,0,0,0); Poll("aux-bad-action");
            Command(i,0,1,0,0,1); Poll("aux-bad-data");
            Command(i,0,1,0,0,0); rx_data[rx_count-1]=0; Poll("aux-bad-tail");
        }
        Command(5,0,1,0,256,0); Poll("light-remains-unimplemented");
        TestLightStubs(); Snapshot("light-stubs-no-effects");
    } else if(n==19) {
        tx_failure=1;
        Command(1,0,0,0,0,0); Command(3,0,1,0,0,0); Command(4,0,1,0,0,0);
        Command(2,0,1,0,1000,1); Poll("tx-failure-does-not-cancel-start");
        MockPulse(); Poll("tx-failure-completion");
    } else if(n==20) {
        Command(2,0,1,0,1000,100); Poll("profile-error-start");
        serial_motion_target_speed_hz=0; MockPulse(); Poll("profile-error");
    } else if(n==21) {
        Command(2,0,1,0,1000,1); Poll("irq-in-poll-start");
        irq_after_gpio=1; tick+=10; Poll("irq-during-limit-sampling");
    } else if(n==22) {
        static const unsigned p[]={2,1,1,2,2,2,0,0,0};
        static const unsigned b[]={6,15,14,9,8,7,12,11,10};
        for(i=0;i<9;i++) {
            input_levels[0]=input_levels[1]=input_levels[2]=65535;
            input_levels[p[i]] &= ~(1U<<b[i]); tick+=10; Poll("individual-limit");
        }
    } else if(n==23) {
        uint32_t random=0x13572468;
        for(i=0;i<1000;i++) {
            uint8_t f[14]; unsigned j;
            for(j=0;j<14;j++) { random=random*1664525+1013904223; f[j]=(uint8_t)(random>>24); }
            if(i%2==0) { f[0]=f[1]=0x55; f[12]=f[13]=0xaa; }
            Queue(f,14); Poll("deterministic-fuzz");
        }
    } else if(n>=24 && n<48) {
        unsigned speed=speeds[(n-24)/6], distance=distances[(n-24)%6];
        Command(2,0,1,(n&1),speed,distance); Poll("profile-start");
        for(i=0;i<distance;i++) MockPulse(); Poll("profile-completion");
    } else if(n==48) {
        for(i=0;i<7100;i++) { tick=i; TestPoll(); }
        Snapshot("legacy-self-test-7100ms");
    }
    Snapshot("final"); return 0;
}
