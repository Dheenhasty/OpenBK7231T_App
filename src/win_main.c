#ifdef WINDOWS

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "new_common.h"
#include "driver\drv_public.h"
#include "cmnds\cmd_public.h"
#include "httpserver\new_http.h"
#include "hal\hal_flashVars.h"
#include "new_pins.h"
#include <timeapi.h>

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "Winmm.lib")

int accum_time = 0;
int win_frameNum = 0;
// this time counter is simulated, I need this for unit tests to work
int g_simulatedTimeNow = 0;
extern int g_port;
#define DEFAULT_FRAME_TIME 5


void strcat_safe_test(){
	char tmpA[16];
	char tmpB[16];
	char buff[128];
	char timeStrA[128];
	char timeStrB[128];
	char timeStrC[128];
	char timeStrD[128];
	char timeStrE[128];
	char timeStrF[128];
	char timeStrG[128];
	char timeStrH[128];
	int res0, res1, res2, res3, res4, res5;
	tmpA[0] = 0;
	res0 = strcat_safe(tmpA,"Test1",sizeof(tmpA));
	res1 = strcat_safe(tmpA," ",sizeof(tmpA));
	res2 = strcat_safe(tmpA,"is now processing",sizeof(tmpA));
	res3 = strcat_safe(tmpA," very long string",sizeof(tmpA));
	res4 = strcat_safe(tmpA," and it",sizeof(tmpA));
	res5 = strcat_safe(tmpA," and it",sizeof(tmpA));
	tmpB[0] = 0;
	res0 = strcat_safe(tmpB,"Test1",sizeof(tmpB));
	res1 = strcat_safe(tmpB," ",sizeof(tmpB));
	res2 = strcat_safe(tmpB,"is now processing",sizeof(tmpB));
	res3 = strcat_safe(tmpB," very long string",sizeof(tmpB));
	res4 = strcat_safe(tmpB," and it",sizeof(tmpB));
	res5 = strcat_safe(tmpB," and it",sizeof(tmpB));

	urldecode2_safe(buff,"qqqqqq%40qqqq",sizeof(buff));
	urldecode2_safe(buff,"qqqqqq%40qqqq",sizeof(buff));


}

void Sim_RunFrame(int frameTime) {
	//printf("Sim_RunFrame: frametime %i\n", frameTime);
	win_frameNum++;
	// this time counter is simulated, I need this for unit tests to work
	g_simulatedTimeNow += frameTime;
	accum_time += frameTime;
	QuickTick(0);
	WIN_RunMQTTFrame();
	HTTPServer_RunQuickTick();
	if (accum_time > 1000) {
		accum_time -= 1000;
		Main_OnEverySecond();
	}
}
void Sim_RunMiliseconds(int ms, bool bApplyRealtimeWait) {
	while (ms > 0) {
		if (bApplyRealtimeWait) {
			Sleep(DEFAULT_FRAME_TIME);
		}
		Sim_RunFrame(DEFAULT_FRAME_TIME);
		ms -= DEFAULT_FRAME_TIME;
	}
}
void Sim_RunSeconds(float f, bool bApplyRealtimeWait) {
	int ms = f * 1000;
	Sim_RunMiliseconds(ms, bApplyRealtimeWait);
}
void Sim_RunFrames(int n, bool bApplyRealtimeWait) {
	int i;

	for (i = 0; i < n; i++) {
		if (bApplyRealtimeWait) {
			Sleep(DEFAULT_FRAME_TIME);
		}
		Sim_RunFrame(DEFAULT_FRAME_TIME);
	}
}

bool bObkStarted = false;
void SIM_Hack_ClearSimulatedPinRoles();


void SIM_ClearOBK(const char *flashPath) {
	if (bObkStarted) {
		DRV_ShutdownAllDrivers();
		release_lfs();
		SIM_Hack_ClearSimulatedPinRoles();
		WIN_ResetMQTT();
		UART_ResetForSimulator();
		CMD_ExecuteCommand("clearAll", 0);
		CMD_ExecuteCommand("led_expoMode", 0);
		// LOG deinit after main init so commands will be re-added
		LOG_DeInit();
	}
	if (flashPath) {
		SIM_SetupFlashFileReading(flashPath);
	}
	bObkStarted = true;
	Main_Init();
}
void Win_DoUnitTests() {
	Test_TwoPWMsOneChannel();
	Test_ClockEvents();
	Test_HassDiscovery();
	Test_Role_ToggleAll_2();
	Test_Demo_ButtonToggleGroup();
	Test_Demo_ButtonScrollingChannelValues();
	Test_CFG_Via_HTTP();
	Test_Commands_Calendar();
	Test_Commands_Generic();
	Test_Demo_SimpleShuttersScript();
	Test_Role_ToggleAll();
	Test_Demo_FanCyclingRelays();
	Test_Demo_MapFanSpeedToRelays();
	Test_MapRanges();
	Test_Demo_ExclusiveRelays();
	Test_MultiplePinsOnChannel();
	Test_Flags();
	Test_DHT();
	Test_EnergyMeter();
	Test_Tasmota();
	Test_NTP();
	Test_MQTT();
	Test_HTTP_Client();
	Test_ExpandConstant();
	Test_ChangeHandlers_MQTT();
	Test_ChangeHandlers();
	Test_RepeatingEvents();
	Test_ButtonEvents();
	Test_Commands_Alias();
	Test_Expressions_RunTests_Basic();
	Test_LEDDriver();
	Test_LFS();
	Test_Scripting();
	Test_Commands_Channels();
	Test_Command_If();
	Test_Command_If_Else(); 
	Test_Tokenizer();
	Test_Http();
	Test_DeviceGroups();

	// this is slowest
	Test_TuyaMCU_Basic();




	// Just to be sure
	// Must be last step
	// reset whole device
	SIM_ClearOBK(0);
}
long g_delta;
float SIM_GetDeltaTimeSeconds() {
	return g_delta * 0.001f;
}
long start_time = 0;
bool bStartTimeSet = false;
long SIM_GetTime() {
	long cur = timeGetTime();
	if (bStartTimeSet == false) {
		start_time = cur;
		bStartTimeSet = true;
	}
	return cur - start_time;
}
// this time counter is simulated, I need this for unit tests to work
int rtos_get_time() {
	return g_simulatedTimeNow;
}
int g_bDoingUnitTestsNow = 0;

#include "sim/sim_public.h"
int __cdecl main(int argc, char **argv)
{
	bool bWantsUnitTests = 1;

	// clear debug data
	if (1) {
		FILE *f = fopen("sim_lastPublishes.txt", "wb");
		if (f != 0) {
			fprintf(f, "\n\n");
			fclose(f);
		}
	}
	if (argc > 1) {
		int value;

		for (int i = 1; i < argc; i++) {
			if (argv[i][0] == '-') {
				if (wal_strnicmp(argv[i] + 1, "port", 4) == 0) {
					i++;

					if (i < argc && sscanf(argv[i], "%d", &value) == 1) {
						g_port = value;
					}
				} else if (wal_strnicmp(argv[i] + 1, "runUnitTests", 12) == 0) {
					i++;

					if (i < argc && sscanf(argv[i], "%d", &value) == 1) {
						bWantsUnitTests = value != 0;
					}
				}
			}
		}
	}

    WSADATA wsaData;
    int iResult;

#if 0
	int maxTest = 100;
	for (int i = 0; i <= maxTest; i++) {
		float frac = (float)i / (float)(maxTest);
		float in = frac;
		float res = LED_BrightnessMapping(255, in);
		printf("Brightness %f with color %f gives %f\n", in, 255.0f, res);
	}
#endif
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
	printf("sizeof(short) = %d\n", (int)sizeof(short));
	printf("sizeof(int) = %d\n", (int)sizeof(int));
	printf("sizeof(long) = %d\n", (int)sizeof(long));
	printf("sizeof(unsigned long) = %d\n", (int)sizeof(unsigned long));
	printf("sizeof(float) = %d\n", (int)sizeof(float));
	printf("sizeof(double) = %d\n", (int)sizeof(double));
	printf("sizeof(long double) = %d\n", (int)sizeof(long double));
	printf("sizeof(led_corr_t) = %d\n", (int)sizeof(led_corr_t));
	
	if (sizeof(FLASH_VARS_STRUCTURE) != MAGIC_FLASHVARS_SIZE) {
		printf("sizeof(FLASH_VARS_STRUCTURE) != MAGIC_FLASHVARS_SIZE!: %i\n", sizeof(FLASH_VARS_STRUCTURE));
		system("pause");
	}
	if (sizeof(ledRemap_t) != MAGIC_LED_REMAP_SIZE) {
		printf("sizeof(ledRemap_t) != MAGIC_LED_REMAP_SIZE!: %i\n", sizeof(ledRemap_t));
		system("pause");
	}
	if (sizeof(led_corr_t) != MAGIC_LED_CORR_SIZE) {
		printf("sizeof(led_corr_t) != MAGIC_LED_CORR_SIZE!: %i\n", sizeof(led_corr_t));
		system("pause");
	}
	//printf("Offset MQTT Group: %i", OFFSETOF(mainConfig_t, mqtt_group));
	if (sizeof(mainConfig_t) != MAGIC_CONFIG_SIZE) {
		printf("sizeof(mainConfig_t) != MAGIC_CONFIG_SIZE!: %i\n", sizeof(mainConfig_t));
		system("pause");
	}
	if (OFFSETOF(mainConfig_t, mqtt_group) != 0x00000554) {
		printf("OFFSETOF(mainConfig_t, mqtt_group) != 0x00000554: %i\n", OFFSETOF(mainConfig_t, mqtt_group));
		system("pause");
	}
	if (OFFSETOF(mainConfig_t, LFS_Size) != 0x000004BC) {
		printf("OFFSETOF(mainConfig_t, LFS_Size) != 0x000004BC: %i\n", OFFSETOF(mainConfig_t, LFS_Size));
		system("pause");
	}
	if (OFFSETOF(mainConfig_t, ping_host) != 0x000005A0) {
		printf("OFFSETOF(mainConfig_t, ping_host) != 0x000005A0: %i\n", OFFSETOF(mainConfig_t, ping_host));
		system("pause");
	}
	if (OFFSETOF(mainConfig_t, buttonShortPress) != 0x000004B8) {
		printf("OFFSETOF(mainConfig_t, buttonShortPress) != 0x000004B8: %i\n", OFFSETOF(mainConfig_t, buttonShortPress));
		system("pause");
	}
	if (OFFSETOF(mainConfig_t, pins) != 0x0000033E) {
		printf("OFFSETOF(mainConfig_t, pins) != 0x0000033E: %i\n", OFFSETOF(mainConfig_t, pins));
		system("pause");
	}
	if (OFFSETOF(mainConfig_t, version) != 0x00000004) {
		printf("OFFSETOF(mainConfig_t, version) != 0x00000004: %i\n", OFFSETOF(mainConfig_t, version));
		system("pause");
	}
	// Test expansion
	//CMD_UART_Send_Hex(0,0,"FFAA$CH1$BB",0);

	if (bWantsUnitTests) {
		g_bDoingUnitTestsNow = 1;
		SIM_ClearOBK(0);
		// let things warm up a little
		Sim_RunFrames(50, false);
		// run tests
		Win_DoUnitTests();
		Sim_RunFrames(50, false);
		g_bDoingUnitTestsNow = 0;
	}


	SIM_CreateWindow(argc, argv);
#if 1
	CMD_ExecuteCommand("MQTTHost 192.168.0.113", 0);
	CMD_ExecuteCommand("MqttPassword ma1oovoo0pooTie7koa8Eiwae9vohth1vool8ekaej8Voohi7beif5uMuph9Diex", 0);
	CMD_ExecuteCommand("MqttClient WindowsOBK", 0);
	CMD_ExecuteCommand("MqttUser homeassistant", 0);
#else
	CMD_ExecuteCommand("MQTTHost 192.168.0.118", 0);
	CMD_ExecuteCommand("MqttPassword Test1", 0);
	CMD_ExecuteCommand("MqttClient WindowsOBK", 0);
	CMD_ExecuteCommand("MqttUser homeassistant", 0);
#endif
	CMD_ExecuteCommand("reboot", 0);
	//CMD_ExecuteCommand("addRepeatingEvent 1 -1 backlog addChannel 1 1; publishInt myTestTopic $CH1", 0);
	
	if (false) {
		while (1) {
			Sleep(DEFAULT_FRAME_TIME);
			Sim_RunFrame(DEFAULT_FRAME_TIME);
			SIM_RunWindow();
		}
	}
	else {
		long prev_time = SIM_GetTime();
		while (1) {
			long cur_time = SIM_GetTime();
			g_delta = cur_time - prev_time;
			if (g_delta <= 0)
				continue;
			Sim_RunFrame(g_delta);
			SIM_RunWindow();
			prev_time = cur_time;
		}
	}
	return 0;
}

// initialise OTA flash starting at startaddr
int init_ota(unsigned int startaddr) {
	return 0;
}

// add any length of data to OTA
void add_otadata(unsigned char *data, int len) {
	return;
}

// finalise OTA flash (write last sector if incomplete)
void close_ota() {
	return;
}

void otarequest(const char *urlin) {
	return;
}

int ota_progress() {
	return 0;
}
int ota_total_bytes() {
	return 0;
}

#endif

