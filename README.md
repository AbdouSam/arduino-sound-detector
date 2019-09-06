# arduino-sound-detector
Detects a typical baby cry using only digital inputs


 This Arduino Code serves as a simplified detector for Baby Crying using digital 
 inputs with only two modules LM393.

 - First LM393 Module (Crying Detection), the module sound threshold is set 
     to high, in a way to detect only the peaks during the Baby Crying process.
     Connection pins:
     OUTPUT -> Pin 7 Arduino
     VCC -> VCC
     GND -> GND

 - Second LM393 Module (Silence Detection), the module sound threshold is set low,
     in a way to detects even a smallest sound made by the baby, and outpus zero, 
     only when there is absolute silence.
      Connectin pins :
     OUTPUT -> Pin 8 Arduino
     VCC -> VCC
     GND -> GND

 - Code description :
     The code detects peaks during baby cry, record the time of peak, and detect the
     distance between the two peaks to decide, if it is similair to a baby cry.

 - do_something_when_crying() : this method is called when the baby is crying,
     put your code inside this routine to be excuted every time a baby cry,
     is detected

 - do_something_when_silence() : same as the first one, this method is called
     whenever a silence is detected

 - #define silence_duration_MAX  determines the max silence duration for baby silence

Note: the LM393 is an Active Low Chip, when the output of LM393 is 5V (HIGH), it means
        no signal is detected When the output is 0V (LOW), it means the threshold 
        sound is detected.

The python script : audio analysis for a crying baby sound. detects the highs and lows
                    and the periods between each high, and duration of highs


