// Arduino G-code Interpreter for Rep Rap
// v1.0 by Mike Ellery - initial software (mellery@gmail.com)
// v1.1 by Zach Hoeken - cleaned up and did lots of tweaks (hoeken@gmail.com)
// v1.2 by Chris Meighan - cleanup / G2&G3 support (cmeighan@gmail.com)
// v1.3 by Zach Hoeken - added thermocouple support and multi-sample temp readings. (hoeken@gmail.com)

// Arduino G-code Interpreter for Macro / Micro photography
// v1.4 by Gene Cooper - modified and setup controls for macro / micro photography (gene@fourchambers.org)

#include <HardwareSerial.h>

//our command string
#define COMMAND_SIZE 128
char word_old[COMMAND_SIZE];
byte serial_count;
int no_data = 0;

void setup()
{
	//Do startup stuff here
	Serial.begin(19200);
	Serial.println("start");
	
	//other initialization.
	init_process_string();
	init_steppers();
        init_camera();
       
}

void loop()
{
	char c;
       
	//read in characters if we got them.
	if (Serial.available() > 0)
	{
		c = Serial.read();
		no_data = 0;

                // give us some feedback on what we are sending
                //Serial.print(c);
		
		//newlines are ends of commands.
		if (c != '\n')
		{
			word_old[serial_count] = c;
			serial_count++;

                        
		}
	}
	//mark no data.
	else
	{
		no_data++;
		delayMicroseconds(100);
	}

	//if theres a pause or we got a real command, do it
	if (serial_count && (c == '\n' || no_data > 100))
	{
		//process our command!
		process_string(word_old, serial_count);

		//clear command.
		init_process_string();
	}

	//no data?  turn off steppers
	if (no_data > 1000)
	disable_steppers();
      


}

