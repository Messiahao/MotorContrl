"""Build both production code versions against identical host HAL mocks.

No board access. Requires Python 3 and an installed MSVC C compiler.
The fixture is the uncommitted working tree saved immediately before extraction.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
import zipfile

ROOT = Path(__file__).resolve().parents[1]
FW = ROOT / 'MotorContrl'
TESTS = ROOT / 'tests'
REMOVED_PROTOCOL_DIAGNOSTICS = {
    'serial_test_build_marker',
    'serial_test_command_count',
    'serial_test_frame_error_count',
    'serial_test_last_frame_ok',
    'serial_test_last_response_ok',
    'serial_test_last_rx_byte',
    'serial_test_rx_byte_count',
    'serial_test_uart_error_count',
}

def mask(text):
    return re.sub(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"',
                  lambda m: re.sub(r'[^\n]', ' ', m[0]), text, flags=re.S)

def function(text, name):
    masked = mask(text)
    found = re.search(r'^[^\n;{}]*\b' + name + r'\([^;{}]*\)\s*\{', masked, re.M)
    assert found, name
    p = masked.index('{', found.start()) + 1
    depth = 1
    while depth:
        depth += (masked[p] == '{') - (masked[p] == '}')
        p += 1
    return text[found.start():p].lstrip('\n')

def body(text, name):
    f = function(text, name)
    return f[f.index('{')+1:f.rfind('}')]

def no_includes(text):
    return re.sub(r'^\s*#include[^\n]*\n', '', text, flags=re.M)

def text(relative):
    return (FW / relative).read_text(encoding='utf-8-sig')

def tokens(source):
    return re.findall(r'\w+|\S', mask(source))

def run_host_executable(executable, case):
    command = [str(executable), str(case)]
    if sys.platform == 'win32':
        command = ['cmd.exe', '/d', '/c'] + command
    return subprocess.run(command, capture_output=True)

def check_structure(old, manifest):
    new = text('Src/main.c')
    for name in ['Serial_Motion_IntegerSqrt', 'Serial_Motion_ProfileSpeed']:
        assert function(old('Src/main.c'), name) == function(new, name), name
    for rel in ['Src/stm32f1xx_hal_msp.c', 'Src/system_stm32f1xx.c', 'Inc/main.h']:
        assert old(rel) == text(rel), rel
    irq_source = text('Src/stm32f1xx_it.c')
    assert all(function(irq_source, name) for name in
               ['TIM2_IRQHandler', 'TIM3_IRQHandler', 'TIM4_IRQHandler'])
    ioc = text('MotorContrl.ioc')
    assert all(value in ioc for value in ['TIM2', 'TIM3', 'TIM4'])
    for rel, digest in manifest['vendor_sha256'].items():
        assert hashlib.sha256((FW / rel).read_bytes()).hexdigest() == digest, rel
    for name in ['SystemClock_Config', 'Error_Handler', 'assert_failed']:
        assert function(old('Src/main.c'), name) == function(text('Src/system_clock.c'), name), name

    definitions = {}
    for h in (FW / 'Inc').glob('*.h'):
        definitions.update(re.findall(r'^#define\s+(\w+)\s+((?:0x[0-9a-fA-F]+|\d+)[UuLl]*)\s*$', h.read_text(encoding='utf-8-sig'), re.M))
    def expand(source):
        return re.sub(r'\b\w+\b', lambda m: definitions.get(m[0], m[0]), source)
    for file, names in {'i2c':['MX_I2C1_Init'], 'spi':['MX_SPI1_Init'],
                        'tim':['MX_TIM2_Init','MX_TIM3_Init','MX_TIM4_Init','HAL_TIM_PWM_MspInit','HAL_TIM_MspPostInit','HAL_TIM_PWM_MspDeInit'],
                        'usart':['MX_USART2_UART_Init','MX_USART3_UART_Init','HAL_UART_MspInit','HAL_UART_MspDeInit']}.items():
        for n in names:
            assert tokens(expand(body(old('Src/'+file+'.c'),n))) == tokens(expand(body(text('Drivers/Board/'+file+'.c'),n))), n
    gpio_init = body(text('Drivers/Board/gpio.c'), 'MX_GPIO_Init')
    assert gpio_init.count('GPIO_MODE_IT_RISING_FALLING') == 3, 'three-axis limit EXTI setup'
    for pin in ['X_LIM_L_Pin', 'X_LIM_H_Pin', 'X_LIM_R_Pin',
                'Y_LIM_L_Pin', 'Y_LIM_H_Pin', 'Y_LIM_R_Pin',
                'Z_LIM_L_Pin', 'Z_LIM_H_Pin', 'Z_LIM_R_Pin']:
        assert pin in gpio_init, pin
    for file, names in {'i2c':['HAL_I2C_MspInit','HAL_I2C_MspDeInit'],
                        'spi':['HAL_SPI_MspInit','HAL_SPI_MspDeInit']}.items():
        for n in names:
            assert tokens(body(old('Src/'+file+'.c'),n)) == tokens(body(text('Drivers/Board/'+file+'.c'),n)), n

    replacements = {
        'TMC5160_WriteRegister':'BspTmc5160_WriteRegister',
        'TMC5160_ReadRegister':'BspTmc5160_ReadRegister',
        'TMC5160_CS_LOW(dev)':'Tmc5160_WriteChipSelect(dev, GPIO_PIN_RESET)',
        'TMC5160_CS_HIGH(dev)':'Tmc5160_WriteChipSelect(dev, GPIO_PIN_SET)',
        'TMC5160_DISABLE(dev)':'BspTmc5160_WriteEnable(dev, 0U)',
        'TMC5160_ENABLE(dev)':'BspTmc5160_WriteEnable(dev, 1U)',
        'HAL_SPI_Transmit(&hspi1, ':'BspSpi_Write(',
        'HAL_SPI_TransmitReceive(&hspi1, ':'BspSpi_ReadWrite(',
    }
    for suffix in ['ReadRegister','WriteRegister','Init']:
        before=body(old('Src/tmc5160.c'),'TMC5160_'+suffix)
        for a,b in replacements.items(): before=before.replace(a,b)
        after=body(text('Drivers/Board/tmc5160.c'),'BspTmc5160_'+suffix)
        assert tokens(expand(before)) == tokens(expand(after)), 'TMC5160 '+suffix
    timer_driver = text('Drivers/Board/tim.c')
    for value in ['TIM_CHANNEL_2', 'TIM_CHANNEL_4', 'TIM_CHANNEL_1',
                  'TIM2_IRQn', 'TIM3_IRQn', 'TIM4_IRQn']:
        assert value in function(timer_driver, 'BspTim_GetAxis'), value

    project = ET.fromstring(text('MDK-ARM/MotorContrl.uvprojx'))
    original = ET.fromstring(old('MDK-ARM/MotorContrl.uvprojx'))
    # The IDE may rewrite whitespace but compiler/linker options must remain identical.
    for path in ['.//TargetOption/TargetArmAds', './/TargetOption/TargetCommonOption/Cpu', './/pCCUsed']:
        assert tokens(ET.tostring(project.find(path),encoding='unicode')) == tokens(ET.tostring(original.find(path),encoding='unicode')), path
    listed = []
    for node in project.findall('.//Group/Files/File'):
        path = (FW / 'MDK-ARM' / node.findtext('FilePath').replace('\\','/')).resolve()
        assert path.exists(), path
        if path.suffix == '.c': listed.append(path)
    assert len(listed) == len(set(listed)), 'duplicate compile unit'
    for directory in ['APPs','Src','Drivers/Board']:
        for source in (FW / directory).glob('*.c'):
            assert source.resolve() in listed, source
    assert 'APPs' in [p.name for p in FW.iterdir()], 'folder case must be APPs'
    for source in (FW/'APPs').glob('*.c'):
        code = mask(source.read_text(encoding='utf-8-sig'))
        assert not re.search(r'\bextern\b',code), source
        assert not re.search(r'\b(?:HAL_GPIO_|HAL_UART_|HAL_TIM_|__HAL_|HAL_NVIC_)\w*\s*\(',code), source
        if source.stem != 'app_scheduler':
            own_prefix = {'app_protocol':'AppProtocol_', 'app_motion':'AppMotion_', 'app_aux_output':'AppAuxOutput_',
                          'app_limit':'AppLimit_', 'app_led':'AppLed_', 'app_light':'AppLight_'}[source.stem]
            assert all(n.startswith(own_prefix) for n in re.findall(r'\b(App\w+)\s*\(',code)), source
    for h in (FW/'Inc').glob('*.h'):
        if h.name == 'stm32f1xx_hal_conf.h': continue
        externs = re.findall(r'^extern[ \t]+\w+[ \t]+\w+;', mask(h.read_text(encoding='utf-8-sig')), re.M)
        expected = (['extern TIM_HandleTypeDef htim2;',
                     'extern TIM_HandleTypeDef htim3;',
                     'extern TIM_HandleTypeDef htim4;'] if h.name=='tim.h' else [])
        assert externs == expected, (h,externs)
    plan=(ROOT/'doc/refactor_test_plan.md').read_text(encoding='utf-8')
    frames=re.findall(r'`((?:[0-9A-F]{2} ){5,}[0-9A-F]{2})`',plan)
    assert frames and all(len(f.split())==14 for f in frames), 'manual test frame length'
    print('PASS: frozen ISR sources, startup bodies, clock/NVIC/IOC/vendor files, project options and module boundaries')

def generate(old, refactored, mode):
    source = old('Src/main.c')
    pv = source.split('/* USER CODE BEGIN PV */')[1].split('/* USER CODE END PV */')[0]
    declarations = re.findall(r'^(?:static )?(?:volatile )?uint(?:8|16|32)_t (\w+)(\[[^]]+\])?;', mask(pv), re.M)
    declarations = [item for item in declarations if item[0] not in REMOVED_PROTOCOL_DIAGNOSTICS]
    declarations = [item for item in declarations if not item[0].startswith('x_tmc5160_')]
    names = [n for n,_ in declarations]
    irq_source = '\n'.join(function(source,n) for n in ['HAL_TIM_PeriodElapsedCallback','Serial_Motion_ProfileSpeed','Serial_Motion_ApplySpeed'])
    irq = {n for n in names if re.search(r'\b'+n+r'\b',irq_source)}
    allcode = (TESTS/'refactor_mock.c').read_text(encoding='utf-8') + '\n'
    pin_header = old('Inc/main.h')
    allcode += '\n'.join(re.findall(r'^#define (?:\w+_Pin|\w+_GPIO_Port)\s+[^\n]+', pin_header, re.M)) + '\n'
    if refactored:
        for h in ['config.h','app_types.h','tmc5160.h','gpio.h','tim.h','usart.h','led.h','mcp4728.h',
                  'app_protocol.h','app_motion.h','app_aux_output.h','app_limit.h','app_led.h','app_light.h','app_scheduler.h']:
            allcode += no_includes(text('Inc/'+h)) + '\n'
        for drv,names_ in {'gpio':['BspGpio_EnableLimitInterrupts','BspGpio_Read','BspGpio_Write','BspGpio_WriteStepMode',
                                   'BspGpio_ReadLimitPin','BspGpio_LimitBitFromPin','BspGpio_ReadLimitActiveMask',
                                   'BspGpio_LimitMaskForAxis','BspGpio_ReadAxisLimitMask'],
                           'tim':['BspTim_GetAxis','BspTim_IsAxisTimer','BspTim_WriteAxisPeriod',
                                   'BspTim_WriteAxisSetupInterrupt','BspTim_WriteAxisStart','BspTim_WriteAxisStop',
                                   'BspTim_WriteAxisDisableUpdate','BspTim_WriteAxisEmergencyStop'],
                           'usart':['BspUsart_ReadOverrun','BspUsart_WriteClearOverrun','BspUsart_ReadAvailable','BspUsart_ReadByte','BspUsart_Write'],
                           'led':['BspLed_Init','BspLed_Write'],
                           'tmc5160':['BspTmc5160_WriteEnable']}.items():
            for n in names_: allcode += function(text('Drivers/Board/'+drv+'.c'),n) + '\n'
        for i,n in enumerate(['BspGpio_Init','BspUsart2_Init','BspI2c_Init','BspSpi_Init','BspTim2_Init','BspTim3_Init','BspTim4_Init','BspUsart3_Init']):
            allcode += f'void {n}(void) {{ MockInit({i}); }}\n'
        allcode += '''static unsigned MockTmcAxisIndex(const TMC5160_HandleTypeDef *d) {
          if (d->CS_GPIO_Port==Y_CS_GPIO_Port && d->CS_Pin==Y_CS_Pin) return 1;
          if (d->CS_GPIO_Port==Z_CS_GPIO_Port && d->CS_Pin==Z_CS_Pin) return 2;
          return 0;
        }
        uint32_t BspTmc5160_ReadRegister(const TMC5160_HandleTypeDef *d,uint8_t a)
          {return MockTmcReadAxis(MockTmcAxisIndex(d),a);}
        void BspTmc5160_WriteRegister(const TMC5160_HandleTypeDef *d,uint8_t a,uint32_t v)
          {MockTmcWriteAxis(MockTmcAxisIndex(d),a,v);}
        '''
        main = text('Src/main.c')
        allcode += no_includes(main.replace(function(main,'main'),'')) + '\n'
        for app in ['protocol','motion','aux_output','limit','led','light','scheduler']:
            allcode += no_includes(text('APPs/app_'+app+'.c')) + '\n'
        allcode += no_includes(text('Drivers/Board/mcp4728.c')) + '\n'
        allcode += '''static void TestInit(void) {
          input_levels[0]=input_levels[1]=input_levels[2]=
            LIMIT_GPIO_ACTIVE_LEVEL ? 0U : 65535U;
          HAL_Init(); SystemClock_Config(); AppScheduler_Init(&motion_irq);
        }
        '''
        allcode += 'static void TestPoll(void) { AppScheduler_Process(); }\n'
        allcode += '''static void TestLightDriver(void) {
          AppProtocolState protocol={0};
          unsigned before=i2c_write_count;
          uint8_t frame[SERIAL_TEST_FRAME_SIZE]={0x55,0x55,SERIAL_COMMAND_LIGHT,0,
            SERIAL_AUX_ACTION_ON,0,APP_LIGHT_CHANNEL_4,0,0,0,0,0,0xaa,0xaa};
          AppLight_Init(); BspMcp4728_Init();
          AppLight_Process(&protocol,frame);
          assert(i2c_write_count==before+1 && i2c_address==0xc0 && i2c_length==3);
          assert(i2c_data[0]==0x46 && i2c_data[1]==0x0f && i2c_data[2]==0xff);
          frame[SERIAL_FRAME_DATA0_INDEX]=SERIAL_AUX_ACTION_OFF;
          frame[SERIAL_FRAME_DATA2_INDEX]=APP_LIGHT_CHANNEL_1;
          AppLight_Process(&protocol,frame);
          assert(i2c_write_count==before+2 && i2c_data[0]==0x40 &&
                 i2c_data[1]==0x00 && i2c_data[2]==0x00);
          frame[SERIAL_FRAME_DATA0_INDEX]=0x03;
          AppLight_Process(&protocol,frame);
          assert(i2c_write_count==before+2);
        }\n'''
    else:
        allcode += source.split('/* USER CODE BEGIN PD */')[1].split('/* USER CODE END PD */')[0]
        allcode += no_includes(old('Inc/tmc5160.h')) + '\n'
        for i,n in enumerate(['MX_GPIO_Init','MX_USART2_UART_Init','MX_I2C1_Init','MX_SPI1_Init','MX_TIM2_Init','MX_TIM3_Init','MX_TIM4_Init','MX_USART3_UART_Init']):
            allcode += f'void {n}(void) {{ MockInit({i}); }}\n'
        allcode += 'uint32_t TMC5160_ReadRegister(const TMC5160_HandleTypeDef *d,uint8_t a) {(void)d;return MockTmcRead(a);}\n'
        allcode += 'void TMC5160_WriteRegister(const TMC5160_HandleTypeDef *d,uint8_t a,uint32_t v) {(void)d;MockTmcWrite(a,v);}\n'
        allcode += pv + '\n'
        allcode += source.split('/* USER CODE BEGIN PFP */')[1].split('/* USER CODE END PFP */')[0] + '\n'
        allcode += source.split('/* USER CODE BEGIN 0 */')[1].split('/* USER CODE END 0 */')[0] + '\n'
        allcode += no_includes(old('Src/led.c')) + '\n'
        mb = body(source,'main')
        init = mb[:mb.index('  /* Infinite loop */')]
        loop = mb[mb.index('  while (1)'):]
        loop = loop[loop.index('{')+1:loop.rfind('}')]
        allcode += 'static void TestInit(void) {\n' + init + '\n}\n'
        allcode += 'static void TestPoll(void) {\n' + loop + '\n}\n'
        allcode += 'static void TestLightDriver(void) {}\n'
    allcode += 'static void Snapshot(const char *name) {\n'
    allcode += 'printf("%s trace=%016llx events=%u tick=%u timer=%u/%u/%u/%u/%u gpio=%u/%u/%u\\n",name,trace_hash,event_count,tick,period,compare_value,counter,interrupt_enable,pwm_running,output_levels[0],output_levels[1],output_levels[2]);\n'
    for n, array in declarations:
        value=n
        if refactored and n not in irq:
            group = ('protocol' if n.startswith('serial_test_') else 'motion' if n.startswith('serial_motion_') else
                     'aux' if n.startswith(('serial_brake_','serial_relay_')) else 'limits' if n.startswith('limit_') else 'tmc')
            value='runtime.'+group+'.'+n
            if n == 'limit_gpio_poll_tick':
                value='0U'
        if array:
            allcode += 'for(unsigned i=0;i<SERIAL_TEST_FRAME_SIZE;i++) printf("rx[%u]=%u\\n",i,(unsigned)'+value+'[i]);\n'
        else:
            allcode += 'printf("'+n+'=%u\\n",(unsigned)'+value+');\n'
    allcode += 'printf("led=%u/%u\\n",led_state,led_last_tick);\n}\n'
    allcode += (TESTS/'refactor_cases.c').read_text(encoding='utf-8')
    if mode == 'continuous_disabled':
        allcode = allcode.replace('#define SERIAL_MOTION_VALIDATION_COMMAND_ENABLED 1U','#define SERIAL_MOTION_VALIDATION_COMMAND_ENABLED 0U')
    return allcode

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--vcvars', type=Path, default=Path(r'D:\APP\Visual Studio 2026\install\VC\Auxiliary\Build\vcvars64.bat'))
    parser.add_argument('--structure-only', action='store_true')
    parser.add_argument('--three-axis-only', action='store_true')
    parser.add_argument('--modes', nargs='+', choices=['default','continuous_disabled'],
                        default=['default','continuous_disabled'])
    args=parser.parse_args()
    with zipfile.ZipFile(TESTS/'fixtures/pre_refactor.zip') as archive:
        def old(rel): return archive.read(rel).decode('utf-8-sig').replace('\r\n','\n')
        manifest=json.loads(archive.read('manifest.json'))
        check_structure(old,manifest)
        if args.structure_only: return
        assert args.vcvars.is_file(), 'MSVC environment missing; pass --vcvars <vcvars64.bat>'
        out=ROOT/'tmp/refactor_host_tests'
        out.mkdir(parents=True,exist_ok=True)
        report=[]
        modes = ['default'] if args.three_axis_only else args.modes
        for mode in modes:
            variants = [('refactored',True)] if args.three_axis_only else [('baseline',False),('refactored',True)]
            for variant, is_new in variants:
                source=out/f'{mode}_{variant}.c'
                source.write_text(generate(old,is_new,mode),encoding='utf-8')
                exe=source.with_suffix('.exe')
                command=f'call "{args.vcvars}" >nul && cl /nologo /std:c11 /Od /utf-8 /W3 /wd4101 /wd4102 /wd4996 "{source}" /Fe:"{exe}" /Fo:"{source.with_suffix(".obj")}"'
                batch=source.with_suffix('.cmd')
                batch.write_text('@chcp 65001 >nul\n@echo off\n'+command+'\n',encoding='utf-8')
                result=subprocess.run(['cmd.exe','/d','/c',str(batch)],capture_output=True,text=True,encoding='utf-8',errors='replace')
                (out/f'{mode}_{variant}_build.log').write_text(result.stdout+result.stderr,encoding='utf-8')
                if result.returncode: raise AssertionError(result.stdout+result.stderr)
            if args.three_axis_only:
                executable=out/f'{mode}_refactored.exe'
                result=run_host_executable(executable,49)
                assert result.returncode==0, result.stderr.decode(errors='replace')
                print('PASS: X/Y/Z axis selection, timer routing, completion, stop and limit isolation')
                return
            cases=range(49) if mode=='default' else [2,5]
            for case in cases:
                results=[]
                for variant in ['baseline','refactored']:
                    executable=out/f'{mode}_{variant}.exe'
                    try:
                        r=run_host_executable(executable,case)
                    except OSError as error:
                        (out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
                        raise RuntimeError(f'Host execution blocked or executable unavailable: {executable}. '
                                           'Completed results saved; no security settings were changed.') from error
                    assert r.returncode==0, (mode,variant,case,r.stderr)
                    results.append(r.stdout)
                if results[0]!=results[1]:
                    for variant,data in zip(['baseline','refactored'],results):
                        (out/f'mismatch_{mode}_{case}_{variant}.txt').write_bytes(data)
                    a,b=[r.decode().splitlines() for r in results]
                    first=next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
                    raise AssertionError(f'{mode} case {case}, line {first+1}:\nold: {a[first:first+1]}\nnew: {b[first:first+1]}')
                report.append({'mode':mode,'case':case,'sha256':hashlib.sha256(results[0]).hexdigest(),'bytes':len(results[0])})
                (out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
            print(f'PASS: {mode}: {len(cases)} cases, ordered HAL traces and every legacy state value match')
        (out/'report.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
        print(f'PASS: {len(report)} differential cases. Hardware timing and electrical behavior remain untested.')

if __name__=='__main__':
    main()
