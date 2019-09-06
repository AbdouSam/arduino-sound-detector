/*
 * TODOs to improve code :
 * - Sanity check for variables.
 * - the millis(), is reset every 50days of operating, add a chek for millis() reset.
 * - improve the Crying decting process, by adding more conditions.
 * - improve the starting conditions.
*/
#define _OFF_  0x00
#define _ON_   0x01
#define _LOW_  0x00
#define _HIGH_ 0x01
#define TRUE   0x01
#define FALSE  0x00

/* Defines for Simulating the Baby cray, should not be in the finale release*/
#define Peak_Duration 400
#define Peak_Distance 4000
#define ON_Time 20000
#define OFF_Time 30000

/* Defines for duration of scream distance during baby Cry*/
#define distace_duration_MAX  8000 //ms
#define distace_duration_MIN  900  //ms
#define peak_duration_MAX     1000 //ms
#define peak_duration_MIN     170  //ms

/* The time during baby silence */
#define silence_duration_MAX  10000 //ms


/*Low pass filter parameters*/
float mean_value_high   = 0.0;
float mean_value_low    = 0.0;
float a_filter_high     = 0.04;
float a_filter_low      = 0.98;

/* Pin assignement*/
int low_sound_pin   = 8;
int high_sound_pin  = 7;

/* Variables for Simulating Baby Cry, should not be included in finale release*/
#ifdef TEST
  int pwm_output_pin               = 3;
  int pwm_output                   = 99;
  char peak_state                  = _ON_ ;
  char distance_state              = _ON_ ;
  unsigned long time_now_peaks     = 0;
  unsigned long time_now_distances = 0;
#endif

/* signal state processing states*/
char signal_state           = _LOW_;
char signal_state_OLD       = _LOW_;
char rising_edge_detected   = FALSE;
char falling_edge_detected  = FALSE;
char peak_detected          = FALSE;
char distance_detected      = FALSE;
char starting_event         = FALSE;
char silence_detected       = FALSE;

/* signal duration record variables*/
unsigned long timer_peak_duration        = 0;
unsigned long timer_peak_distance        = 0;
unsigned long timer_silence_duration     = 0;
unsigned long timer_silence_duration_str = 0;

//Threshol of detection after filtering
float threshold_peak_high = 0.03; 
float threshold_peak_low  = 0.03;

void setup()
{
  Serial.begin(9600);
  pinMode(low_sound_pin, INPUT);
  pinMode(high_sound_pin, INPUT);
  mean_value_high = digitalRead(high_sound_pin);
  mean_value_low = digitalRead(low_sound_pin);
  
  #ifdef TEST
    Simulate the Cry as a PWM output
    pinMode(pwm_output_pin, OUTPUT);
    analogWrite(pwm_output_pin, pwm_output);
    peak_state = _ON_;
  #endif
}

void loop()
{
  
  mean_value_high = filtered_input_high();
  mean_value_low = filtered_input_low();

  #ifdef TEST
    simulate_crying_sound();
  #endif

  process_signal();
  
}

void do_something_when_silence(void)
{
  // write here, the code you want to be excuted when there is 
  // too long silence.
  Serial.println("No sound for too long, check in the baby.");
}

void do_something_when_crying(void)
{
  // write here, the code you want to be excuted whene the 
  // baby crying is detected.
  Serial.println("Baby is Crying ...");
}

/*NOTE: This routine needs Serial.begin() to be called before*/
void serial_plot(float y_value)
{
  Serial.println(y_value);
  Serial.print(" ");
}

/*Use the second LM393, to detects the duration of the silence*/
void detect_silence(void)
{
  if((mean_value_low <= threshold_peak_low) && (silence_detected == FALSE))
  {
    silence_detected = TRUE;
    timer_silence_duration_str = millis();
  }
  else
  if(mean_value_low > threshold_peak_low)
  {
    silence_detected = FALSE;
  }

  if(silence_detected == TRUE)
  {
    timer_silence_duration = millis() - timer_silence_duration_str;
    if(timer_silence_duration > silence_duration_MAX)
    {
      silence_detected = FALSE;
      do_something_when_silence();
    }
  }
  
}
/* Detect rising Edge and Falling Edge of the filtered signal
* to know when baby is screaming and when he is breathing in*/
void detect_signal_edge(void)
{
  if(mean_value_high >= threshold_peak_high)
  {
   signal_state = _HIGH_;
    if(signal_state_OLD == (_LOW_))
    {
      rising_edge_detected = TRUE;
      signal_state_OLD = _HIGH_;
    }

  }
  else if(mean_value_high < threshold_peak_high)
  {
    signal_state = _LOW_;
    if(signal_state_OLD == _HIGH_)
    {
      falling_edge_detected = TRUE;
      signal_state_OLD = _LOW_;
    }
  }
  else
  {
    //in between
  }
}

/*Detect the peaks in the signal and count the duration of 
* peaks and distance between peaks*/
void detect_peak(void)
{  
  detect_signal_edge();

  if(rising_edge_detected == TRUE)
  {
    rising_edge_detected = FALSE;
    //start timer peak duration
    timer_peak_duration = millis();

    //reset and stop timer peak distances and save value
    timer_peak_distance = millis() - timer_peak_distance;
    //save the data on an array
    //We ve got a Distance.
    distance_detected = TRUE;
  }
  else if(falling_edge_detected == TRUE)
  {
    falling_edge_detected = FALSE;
    //start time peak distances
    timer_peak_distance = millis();
    //reset and stop timer peak duration and save value
    timer_peak_duration = millis() - timer_peak_duration;
    //save the data on array
    //We ve got a Peak
    peak_detected = TRUE;
  }
}

/* Process signals comming from the tow LM393, detects the peak and decide based
* on the duration of peaks and distance between peaks */
void process_signal(void)
{
  detect_silence();
  detect_peak();

  if(peak_detected == TRUE)
  {
    peak_detected = FALSE;
    if(timer_peak_duration < peak_duration_MAX &&
      timer_peak_duration > peak_duration_MIN)
    {
      //we have a starting Cry
      starting_event = TRUE;
    }
    else
    {
      //signal too small or too big
    }
  }

  if(distance_detected == TRUE)
  {
    distance_detected = FALSE;
    if(timer_peak_distance < distace_duration_MAX &&
       timer_peak_distance > distace_duration_MIN)
    {
      //breath in
      if(starting_event == TRUE)
      {
        //Crying Baby
        starting_event = FALSE;
        do_something_when_crying();
      }
    }
    else if(timer_peak_distance > silence_duration_MAX)
    {
      // silence too long, this section is processed by the 
      // silence routine.
    }
  }
}

/*Filter the inputs by a low pass filter to avoid indesirable peaks, and control
* the signal more effeciently*/
float filtered_input_high(void)
{
  return ((a_filter_high * (HIGH - digitalRead(high_sound_pin))) + (1 - a_filter_high) * mean_value_high);
}

float filtered_input_low(void)
{
  return ((a_filter_low * (HIGH - digitalRead(low_sound_pin))) + (1 - a_filter_low) * mean_value_low);
}

#ifdef TEST
  /* Test function, to sumilate a baby cry */
  void simulate_crying_sound(void)
  {
    if((millis() > time_now_peaks + Peak_Duration) && (peak_state == _ON_) &&
                                           (distance_state == _ON_))
    {
      time_now_peaks = millis();
      analogWrite(pwm_output_pin, 0);
      peak_state = _OFF_;
    }

    if((millis() > time_now_peaks + Peak_Distance) && (peak_state == _OFF_) &&
                                             (distance_state == _ON_))
    {
      time_now_peaks = millis();    
      analogWrite(pwm_output_pin, pwm_output);
      peak_state = _ON_;
    }
    if((millis() > time_now_distances + ON_Time )&& (distance_state == _ON_))
    {
      time_now_distances = millis();
      analogWrite(pwm_output_pin, 0);
      distance_state = _OFF_;
    }

    if((millis() > time_now_distances + OFF_Time) && (distance_state ==_OFF_))
    {
      time_now_distances = millis();
      distance_state = _ON_;
    }
  }
#endif

