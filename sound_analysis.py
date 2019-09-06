# Baby cry audio analysis, to see the patern of Highs
# and Lows in a baby cry audio, and set a pattern to 
# recognize ig using an arduino with digital inputs

from scipy.io.wavfile import read
import matplotlib.pyplot as plt 
import numpy as np 
from scipy.signal import butter, lfilter, freqz



def butter_lowpass(cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    return b, a

def butter_lowpass_filter(data, cutoff, fs, order=5):
    b, a = butter_lowpass(cutoff, fs, order=order)
    y = lfilter(b, a, data)
    return y


fs, data = read("baby_crying_1.wav")
raw_data_size = len(data)
nData = np.array(data)
data_audio = nData[:,1]

data_time = 18 # second
data_size =  int(data_time * fs)
data_audio_time = np.linspace(0, data_time, data_size, endpoint=False)

data_audio_filtered = np.ndarray(shape = data_audio.shape)
data_audio_filtered = data_audio_filtered[:data_size]

peak_amplitude     = 5000 # amplitude value

order = 6
cutoff = 50  # desired cutoff frequency of the filter, Hz

# Get the filter coefficients so we can check its frequency response.
b, a = butter_lowpass(cutoff, fs, order)


idx = 0

_LOW_   = 0
_HIGH_  = 1

signal_state = _LOW_
signal_state_OLD = _LOW_
rising_edge_detected = False
faling_edge_detected = False

timer_peak_duration       = 0
timer_peak_durationArray  = []
timer_peak_distances      = 0
timer_peak_distancesArray = []

while idx < data_size:
  if(data_audio[idx] >= peak_amplitude):
    data_audio_filtered[idx] = data_audio[idx]
  elif(data_audio[idx] < peak_amplitude):
    data_audio_filtered[idx] = 0
  idx = idx + 1

idx = 0

y = butter_lowpass_filter(data_audio_filtered, cutoff, fs, order)

while idx < data_size:
  if(y[idx] > 0):
    signal_state = _HIGH_
    if(signal_state_OLD == (_LOW_)):
      #we hav a rising edge start counting the seconds.
      rising_edge_detected = True
      signal_state_OLD = _HIGH_
  elif(y[idx] < 0.02):
    signal_state = _LOW_
    if(signal_state_OLD == _HIGH_):
      faling_edge_detected = True
      signal_state_OLD = _LOW_
  
  if(rising_edge_detected == True):
    rising_edge_detected = False
    #reset and stop timer peak distances and save value
    timer_peak_distances = idx - timer_peak_distances
    timer_peak_distancesArray.append(timer_peak_distances)
    timer_peak_distances = 0
    #start timer peak duration
    timer_peak_duration = idx
  elif(faling_edge_detected == True):
    faling_edge_detected = False
    #reset and stop timer peak duration and save value
    timer_peak_duration = idx - timer_peak_duration
    timer_peak_durationArray.append(timer_peak_duration)
    timer_peak_duration = 0
    #start time peak distances
    timer_peak_distances = idx

  idx = idx + 1


print("timer durations : ", timer_peak_durationArray)
print("timer distances :" , timer_peak_distancesArray)

plt.plot(data_audio_time,data_audio_filtered, 'b-', label='data')
plt.plot(data_audio_time,y, 'g-', linewidth = 2, label='filtered data')
plt.ylabel("Amplitude")
plt.xlabel("Time")
plt.title("WaveForm")
plt.show()
