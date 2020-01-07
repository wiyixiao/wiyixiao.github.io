// camera controls

void init_camera()
{
        pinMode(CAM_SHUTTER_PIN1, OUTPUT);
        pinMode(CAM_SHUTTER_PIN2, OUTPUT);
        pinMode(CAM_AUX_PIN1, OUTPUT); // analog 0
        pinMode(CAM_AUX_PIN2, OUTPUT); // analog 1
        pinMode(CAM_AUX_PIN3, OUTPUT); // analog 2
        pinMode(CAM_AUX_PIN4, OUTPUT); // analog 3
}

void camera_shutter1()
{
	// fire the camera shutter via relay...1/4 sec hold time
	digitalWrite(CAM_SHUTTER_PIN1, HIGH);
	delay(250);
	digitalWrite(CAM_SHUTTER_PIN1, LOW);

}

void camera_shutter2()
{
	// fire the camera shutter via relay...1/4 sec hold time
	digitalWrite(CAM_SHUTTER_PIN2, HIGH);
	delay(250);
	digitalWrite(CAM_SHUTTER_PIN2, LOW);

}

void camera_aux1_on()
{
	// turn aux relay 1 on
	digitalWrite(CAM_AUX_PIN1, HIGH);
}

void camera_aux1_off()
{
	// turn aux relay 1 off
	digitalWrite(CAM_AUX_PIN1, LOW);
}

void camera_aux2_on()
{
	// turn aux relay 2 on
	digitalWrite(CAM_AUX_PIN2, HIGH);
}

void camera_aux2_off()
{
	// turn aux relay 2 off
	digitalWrite(CAM_AUX_PIN2, LOW);
}

void camera_aux3_on()
{
	// turn aux relay 3 on
	digitalWrite(CAM_AUX_PIN3, HIGH);
}

void camera_aux3_off()
{
	// turn aux relay 3 off
	digitalWrite(CAM_AUX_PIN3, LOW);
}

void camera_aux4_on()
{
	// turn aux relay 4 on
	digitalWrite(CAM_AUX_PIN4, HIGH);
}

void camera_aux4_off()
{
	// turn aux relay 4 off
	digitalWrite(CAM_AUX_PIN4, LOW);
}


