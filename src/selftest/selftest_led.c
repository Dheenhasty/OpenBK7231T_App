#ifdef WINDOWS

#include "selftest_local.h".

void Test_LEDDriver_CW() {
	int i;
	// reset whole device
	SIM_ClearOBK(0);

	// CW bulb requires two PWMs - first on channel 1, second on channel 2
	// NOTE: we also support case where first PWM is on channel 0, and second on 1
	PIN_SetPinRoleForPinIndex(24, IOR_PWM);
	PIN_SetPinChannelForPinIndex(24, 1);

	PIN_SetPinRoleForPinIndex(26, IOR_PWM);
	PIN_SetPinChannelForPinIndex(26, 2);

	CMD_ExecuteCommand("led_enableAll 1", 0);


	// check expressions (not really LED related but ok)
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 1.0f);
	// Set 100% Warm
	CMD_ExecuteCommand("led_temperature 500", 0);

	printf("Channel C is %i, channel W is %i\n", CHANNEL_Get(1), CHANNEL_Get(2));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 100);

	// check read access
	SELFTEST_ASSERT_EXPRESSION("$led_temperature", 500.0f);

	// check expressions (not really LED related but ok)
	SELFTEST_ASSERT_EXPRESSION("$led_temperature*2",1000.0f);
	SELFTEST_ASSERT_EXPRESSION("2*$led_temperature", 1000.0f);

	// Set 100% Cold
	CMD_ExecuteCommand("led_temperature 154", 0);
	// check read access
	SELFTEST_ASSERT_EXPRESSION("$led_temperature", 154.0f);
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 1.0f);
	SELFTEST_ASSERT_CHANNEL(1, 100);
	SELFTEST_ASSERT_CHANNEL(2, 0);

	// Tasmota style command should disable LED
	Test_FakeHTTPClientPacket_GET("cm?cmnd=POWER%200");
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 0.0f);
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	// Tasmota style command should enable LED
	Test_FakeHTTPClientPacket_GET("cm?cmnd=POWER%201");
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 1.0f);
	SELFTEST_ASSERT_CHANNEL(1, 100);
	SELFTEST_ASSERT_CHANNEL(2, 0);


	SIM_SendFakeMQTTAndRunSimFrame_CMND("POWER", "ON");
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 1.0f);
	SELFTEST_ASSERT_CHANNEL(1, 100);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SIM_SendFakeMQTTAndRunSimFrame_CMND("POWER", "OFF");
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 0.0f);
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SIM_SendFakeMQTTAndRunSimFrame_CMND("POWER", "ON");
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 1.0f);
	SELFTEST_ASSERT_CHANNEL(1, 100);
	SELFTEST_ASSERT_CHANNEL(2, 0);

	for (i = 0; i <= 100; i++) {
		char buffer[64];
		sprintf(buffer, "Dimmer %i", i);
		CMD_ExecuteCommand(buffer, 0);
		//printf("Dimmer loop test: %i\n",i);
		SELFTEST_ASSERT_EXPRESSION("$led_dimmer", i);
	}
	for (i = 154; i <= 500; i++) {
		char buffer[64];
		sprintf(buffer, "CT %i", i);
		CMD_ExecuteCommand(buffer, 0);
		//printf("Dimmer loop test: %i\n",i);
		SELFTEST_ASSERT_EXPRESSION("$led_temperature", i);
	}
	CMD_ExecuteCommand("Dimmer 0", 0);
	for (i = 0; i < 100; i++) {
		SELFTEST_ASSERT_EXPRESSION("$led_dimmer", i);
		CMD_ExecuteCommand("add_dimmer 1", 0);
		SELFTEST_ASSERT_EXPRESSION("$led_dimmer", i+1);
	}
}


void Test_LEDDriver_RGBCW() {
	// reset whole device
	SIM_ClearOBK(0);

	PIN_SetPinRoleForPinIndex(24, IOR_PWM);
	PIN_SetPinChannelForPinIndex(24, 1);

	PIN_SetPinRoleForPinIndex(26, IOR_PWM);
	PIN_SetPinChannelForPinIndex(26, 2);

	PIN_SetPinRoleForPinIndex(9, IOR_PWM);
	PIN_SetPinChannelForPinIndex(9, 3);

	PIN_SetPinRoleForPinIndex(6, IOR_PWM);
	PIN_SetPinChannelForPinIndex(6, 4);

	PIN_SetPinRoleForPinIndex(7, IOR_PWM);
	PIN_SetPinChannelForPinIndex(7, 5);

	CMD_ExecuteCommand("led_enableAll 1", 0);
	CMD_ExecuteCommand("led_dimmer 100", 0);
	// Set 100% Warm
	CMD_ExecuteCommand("led_temperature 500", 0);

	printf("Channel R is %i, channel G is %i, channel B is %i, channel C is %i, channel W is %i\n",
		CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3), CHANNEL_Get(4), CHANNEL_Get(5));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 0);
	SELFTEST_ASSERT_CHANNEL(4, 0);
	SELFTEST_ASSERT_CHANNEL(5, 100);

	// Set 100% Cool
	CMD_ExecuteCommand("led_temperature 154", 0);

	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 0);
	SELFTEST_ASSERT_CHANNEL(4, 100);
	SELFTEST_ASSERT_CHANNEL(5, 0);

	// Set 100% Red
	CMD_ExecuteCommand("led_basecolor_rgb FF0000", 0);

	SELFTEST_ASSERT_CHANNEL(1, 100);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 0);
	SELFTEST_ASSERT_CHANNEL(4, 0);
	SELFTEST_ASSERT_CHANNEL(5, 0);


	// Set 100% RedGreen
	CMD_ExecuteCommand("led_basecolor_rgb FFFF00", 0);

	SELFTEST_ASSERT_CHANNEL(1, 100);
	SELFTEST_ASSERT_CHANNEL(2, 100);
	SELFTEST_ASSERT_CHANNEL(3, 0);
	SELFTEST_ASSERT_CHANNEL(4, 0);
	SELFTEST_ASSERT_CHANNEL(5, 0);

	// set 50%
	CMD_ExecuteCommand("led_dimmer 50", 0);

	printf("Channel R is %i, channel G is %i, channel B is %i, channel C is %i, channel W is %i\n",
		CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3), CHANNEL_Get(4), CHANNEL_Get(5));
	SELFTEST_ASSERT_CHANNEL(1, 21);
	SELFTEST_ASSERT_CHANNEL(2, 21);
	SELFTEST_ASSERT_CHANNEL(3, 0);
	SELFTEST_ASSERT_CHANNEL(4, 0);
	SELFTEST_ASSERT_CHANNEL(5, 0);
}
void Test_LEDDriver_RGB() {
	// reset whole device
	SIM_ClearOBK(0);

	// RGB bulb requires three PWMs - first on channel 1, second on channel 2, third on channel 3
	// NOTE: we also support case where first PWM is on channel 0, and second on 1
	PIN_SetPinRoleForPinIndex(24, IOR_PWM);
	PIN_SetPinChannelForPinIndex(24, 1);

	PIN_SetPinRoleForPinIndex(26, IOR_PWM);
	PIN_SetPinChannelForPinIndex(26, 2);

	PIN_SetPinRoleForPinIndex(9, IOR_PWM);
	PIN_SetPinChannelForPinIndex(9, 3);

	CMD_ExecuteCommand("led_enableAll 1", 0);
	// check expressions (not really LED related but ok)
	SELFTEST_ASSERT_EXPRESSION("$led_enableAll", 1.0f);
	// Set red
	CMD_ExecuteCommand("led_baseColor_rgb FF0000", 0);
	// full brightness
	CMD_ExecuteCommand("led_dimmer 100", 0);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer", 100.0f);
	SELFTEST_ASSERT_EXPRESSION("123*$led_dimmer", 12300.0f);
	SELFTEST_ASSERT_EXPRESSION("123-$led_dimmer", 23.0f);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer-123", -23.0f);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer*0.01", 1.0f);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer*0.001", 0.1f);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer/100", 1.0f);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer/1000", 0.1f);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer+$led_dimmer", 200.0f);
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer+$led_dimmer+$led_dimmer", 300.0f);

	printf("Channel R is %i, channel G is %i, channel B is %i\n", CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3));
	SELFTEST_ASSERT_CHANNEL(1, 100);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 0);

	// Set green
	CMD_ExecuteCommand("led_baseColor_rgb 00FF00", 0);
	printf("Channel R is %i, channel G is %i, channel B is %i\n", CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 100);
	SELFTEST_ASSERT_CHANNEL(3, 0);

	// Set blue - also support #
	CMD_ExecuteCommand("led_baseColor_rgb #0000FF", 0);
	printf("Channel R is %i, channel G is %i, channel B is %i\n", CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 100);

	// set 50% brightness
	CMD_ExecuteCommand("led_dimmer 50", 0);
	// check expressions (not really LED related but ok)
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer", 50.0f);
	printf("Channel R is %i, channel G is %i, channel B is %i\n", CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 21);

	// set 90% brightness
	CMD_ExecuteCommand("led_dimmer 90", 0);
	// check expressions (not really LED related but ok)
	SELFTEST_ASSERT_EXPRESSION("$led_dimmer", 90.0f);
	printf("Channel R is %i, channel G is %i, channel B is %i\n", CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 79);
	// disable
	CMD_ExecuteCommand("led_enableAll 0", 0);
	printf("Channel R is %i, channel G is %i, channel B is %i\n", CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 0);
	// reenable, it should remember last state
	CMD_ExecuteCommand("led_enableAll 1", 0);
	printf("Channel R is %i, channel G is %i, channel B is %i\n", CHANNEL_Get(1), CHANNEL_Get(2), CHANNEL_Get(3));
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 79);

	// Tasmota style command should disable LED
	Test_FakeHTTPClientPacket_GET("cm?cmnd=POWER%200");
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 0);
	// Tasmota style command should enable LED
	Test_FakeHTTPClientPacket_GET("cm?cmnd=POWER%201");
	SELFTEST_ASSERT_CHANNEL(1, 0);
	SELFTEST_ASSERT_CHANNEL(2, 0);
	SELFTEST_ASSERT_CHANNEL(3, 79);

	// make error
	//SELFTEST_ASSERT_CHANNEL(3, 666);
}
void Test_LEDDriver() {

	Test_LEDDriver_CW();
	Test_LEDDriver_RGB();
	Test_LEDDriver_RGBCW();
}

#endif
