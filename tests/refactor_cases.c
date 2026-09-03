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
static void LightCommand(unsigned action, unsigned channel, unsigned extra)
{
    uint8_t f[14]={0x55,0x55,5,0,0,0,0,0,0,0,0,0,0xaa,0xaa};
    f[4]=(uint8_t)action; f[6]=(uint8_t)channel; f[8]=(uint8_t)extra;
    Queue(f,14);
}
int main(int argc, char **argv)
{
    unsigned n=argc>1 ? (unsigned)atoi(argv[1]) : 0, i;
    static const unsigned speeds[]={1000,1001,3000,10000};
    static const unsigned distances[]={1,2,3,100,5000,12000};
    registers_tmc[0x04]=0x30000000; registers_tmc[0x6c]=0x10400050;
#ifdef SERIAL_MOTION_AXIS_Y
    registers_tmc_y[0x04]=registers_tmc_z[0x04]=0x30000000;
    registers_tmc_y[0x6c]=registers_tmc_z[0x6c]=0x10400050;
#endif
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
            Command(2,0,i==0?4:1,i==1?2:0,i==2?999:i==3?10001:1000,
                    i==4?0:i==5?12001:i==6?5001:10);
            if(i==7) rx_data[rx_count-1]=0;
            Poll("invalid-start");
        }
        Command(2,2,1,0,0,0); Poll("stop-idle");
        Command(2,2,4,0,0,0); Poll("stop-axis");
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
        for(i=1;i<=4;i++) {
            LightCommand(1,i,0); Poll("light-on");
            LightCommand(2,i,0); Poll("light-off");
        }
        LightCommand(3,1,0); Poll("light-bad-action");
        LightCommand(1,1,1); Poll("light-bad-data");
        LightCommand(1,5,0); Poll("light-bad-channel");
        LightCommand(1,1,0); rx_data[rx_count-1]=0; Poll("light-bad-tail");
        TestLightDriver(); Snapshot("light-driver-effects");
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
#ifdef SERIAL_MOTION_AXIS_Y
    } else if(n==49) {
        static const unsigned axes[]={SERIAL_MOTION_AXIS_X,SERIAL_MOTION_AXIS_Y,SERIAL_MOTION_AXIS_Z};
        static const unsigned channels[]={TIM_CHANNEL_2,TIM_CHANNEL_4,TIM_CHANNEL_1};
        static GPIO_TypeDef *const dir_ports[]={X_DIR_GPIO_Port,Y_DIR_GPIO_Port,Z_DIR_GPIO_Port};
        static const unsigned dir_pins[]={X_DIR_Pin,Y_DIR_Pin,Z_DIR_Pin};
        static GPIO_TypeDef *const step_ports[]={X_STEP_GPIO_Port,Y_STEP_GPIO_Port,Z_STEP_GPIO_Port};
        static const unsigned step_pins[]={X_STEP_Pin,Y_STEP_Pin,Z_STEP_Pin};
        for(i=0;i<3;i++) {
            Command(2,0,axes[i],1,1000,2); Poll("axis-start");
            assert(runtime.motion.serial_motion_busy==1 && serial_motion_active==1);
            assert(serial_motion_axis==axes[i] && pwm_axis==i && pwm_channel==channels[i]);
            assert((output_levels[dir_ports[i]->id]&dir_pins[i])!=0);
            assert(step_mode_port==step_ports[i]->id && step_mode_pin==step_pins[i] && step_mode==GPIO_MODE_AF_PP);
            assert(runtime.motion.axis_state[i].spi_ok==1 && runtime.motion.axis_state[i].enable_ok==1);
            MockPulse(); MockPulse(); Poll("axis-done");
            assert(runtime.motion.serial_motion_busy==0 && serial_motion_active==0 && pwm_running==0);
            assert(runtime.motion.serial_motion_state==SERIAL_MOTION_STATE_DONE);
            assert(runtime.motion.serial_motion_target_axis==axes[i] && runtime.motion.serial_motion_mscnt_ok==1);
        }
        for(i=1;i<3;i++) {
            Command(2,0,axes[i],0,3000,100); Poll("axis-profile-start");
            for(unsigned pulse=0;pulse<100;pulse++) MockPulse();
            Poll("axis-profile-done");
            assert(runtime.motion.serial_motion_state==SERIAL_MOTION_STATE_DONE);
            assert(serial_motion_peak_speed_hz>1000 && serial_motion_peak_speed_hz<3000);
            assert(serial_motion_profile_accel_steps>0 && serial_motion_profile_decel_steps>0);
            assert(runtime.motion.serial_motion_mscnt_ok==1);
        }
        Command(2,0xf0,SERIAL_MOTION_AXIS_Y,0,1000,0); Poll("y-continuous");
        input_levels[0]|=Z_LIM_L_Pin; HAL_GPIO_EXTI_Callback(Z_LIM_L_Pin); Poll("z-limit-ignored-by-y");
        assert(runtime.motion.serial_motion_busy==1 && serial_motion_active==1);
        input_levels[0]&=~Z_LIM_L_Pin; HAL_GPIO_EXTI_Callback(Z_LIM_L_Pin);
        input_levels[2]|=Y_LIM_L_Pin; HAL_GPIO_EXTI_Callback(Y_LIM_L_Pin); Poll("y-limit-stop");
        assert(runtime.motion.serial_motion_busy==0 && runtime.motion.serial_motion_state==SERIAL_MOTION_STATE_LIMIT_STOPPED);
        assert(runtime.motion.serial_motion_last_limit_mask==(1U<<BSP_GPIO_Y_LIMIT_L_BIT));
        input_levels[2]&=~Y_LIM_L_Pin; HAL_GPIO_EXTI_Callback(Y_LIM_L_Pin);
        Command(2,0xf0,SERIAL_MOTION_AXIS_Z,0,1000,0); Poll("z-continuous");
        Command(2,2,SERIAL_MOTION_AXIS_Y,0,0,0); Poll("wrong-axis-stop");
        assert(runtime.motion.serial_motion_busy==1 && serial_motion_active==1);
        Command(2,2,SERIAL_MOTION_AXIS_Z,0,0,0); Poll("z-stop");
        assert(runtime.motion.serial_motion_busy==0 && runtime.motion.serial_motion_state==SERIAL_MOTION_STATE_STOPPED);
        input_levels[0]&=~Z_LIM_L_Pin; HAL_GPIO_EXTI_Callback(Z_LIM_L_Pin);
        Command(2,0xf0,SERIAL_MOTION_AXIS_Z,0,1000,0); Poll("z-continuous-limit-test");
        assert(runtime.motion.serial_motion_busy==1 && serial_motion_active==1);
        input_levels[2]|=X_LIM_L_Pin; HAL_GPIO_EXTI_Callback(X_LIM_L_Pin); Poll("x-limit-ignored-by-z");
        assert(runtime.motion.serial_motion_busy==1 && serial_motion_active==1);
        input_levels[2]&=~X_LIM_L_Pin; HAL_GPIO_EXTI_Callback(X_LIM_L_Pin);
        input_levels[0]|=Z_LIM_L_Pin; HAL_GPIO_EXTI_Callback(Z_LIM_L_Pin); Poll("z-limit-stop");
        assert(runtime.motion.serial_motion_busy==0 && runtime.motion.serial_motion_state==SERIAL_MOTION_STATE_LIMIT_STOPPED);
        assert(runtime.motion.serial_motion_last_limit_mask==(1U<<BSP_GPIO_Z_LIMIT_L_BIT));
        input_levels[0]&=~Z_LIM_L_Pin; HAL_GPIO_EXTI_Callback(Z_LIM_L_Pin);
#endif
    }
    Snapshot("final"); return 0;
}
